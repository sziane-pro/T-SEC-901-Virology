#include "persistence.h"
#include "config.h"
#include "error.h"
#include "log.h"
#include <windows.h>
#include <string>

namespace {

constexpr DWORD   RecoveryDelayMs  = 5000;
constexpr DWORD   RecoveryResetDay = 86400;   // 1 jour, en secondes

std::wstring current_exe_path() {
    wchar_t buffer[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, buffer, MAX_PATH) == 0) {
        return {};
    }
    return buffer;
}

// Configure les recovery actions : redémarrage après 5 s, 3 fois, reset 1 jour.
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
    // Non bloquant : une description manquante n'empêche pas le service de tourner.
    return ChangeServiceConfig2W(service, SERVICE_CONFIG_DESCRIPTION, &desc) != 0;
}

}  // namespace

bool install_persistence() {
    const std::wstring exe = current_exe_path();
    if (exe.empty()) {
        log_line(format_last_error("GetModuleFileNameW"));
        return false;
    }

    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (!scm) {
        log_line(format_last_error("OpenSCManager"));
        return false;
    }

    SC_HANDLE service = CreateServiceW(
        scm,
        ServiceNameW,
        ServiceDisplayNameW,                // display name
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        exe.c_str(),
        nullptr, nullptr, nullptr,
        nullptr,                            // LocalSystem
        nullptr);

    bool created_now = true;
    if (!service) {
        const DWORD err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS) {
            // Idempotence : on rafraîchit la config d'un service existant.
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

    // Sur un service existant, on aligne aussi le binaire et le type de démarrage
    // (utile si l'exe a été déplacé).
    if (!created_now) {
        if (!ChangeServiceConfigW(service,
                SERVICE_NO_CHANGE,
                SERVICE_AUTO_START,
                SERVICE_NO_CHANGE,
                exe.c_str(),
                nullptr, nullptr, nullptr, nullptr, nullptr,
                ServiceNameW)) {
            log_line(format_error(GetLastError(), "ChangeServiceConfig(binPath)"));
        }
    }

    const bool ok = apply_failure_actions(service);
    apply_description(service);   // best-effort

    // Démarrage immédiat en Release (en Debug, on évite le doublon avec la console interactive).
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
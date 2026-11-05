#define _CRT_SECURE_NO_WARNINGS

#include "commands_internal.h"
#include "config.h"
#include "error.h"
#include <cstdio>
#include <string>
#include <windows.h>

namespace {

// "C:\Windows" -> "C:\Windows\*" ; "C:\" -> "C:\*" ; "C:" -> "C:\*"
std::string search_pattern(const std::string& base) {
	if (base.empty()) {
		return "*";
	}
	const char last = base.back();
	if (last == '\\' || last == '/' || last == ':') {
		return base + "*";
	}
	return base + "\\*";
}

std::string absolute_path(const std::string& path) {
	char buffer[MAX_PATH];
	const DWORD size = GetFullPathNameA(path.c_str(), MAX_PATH, buffer, nullptr);
	if (size == 0 || size >= MAX_PATH) {
		return path;
	}
	return std::string(buffer);
}

std::string human_size(unsigned long long bytes) {
	char buffer[32];
	if (bytes < 1024ull) {
		snprintf(buffer, sizeof(buffer), "%llu o", bytes);
	}
	else if (bytes < 1024ull * 1024ull) {
		snprintf(buffer, sizeof(buffer), "%.1f Ko", bytes / 1024.0);
	}
	else if (bytes < 1024ull * 1024ull * 1024ull) {
		snprintf(buffer, sizeof(buffer), "%.1f Mo", bytes / (1024.0 * 1024.0));
	}
	else {
		snprintf(buffer, sizeof(buffer), "%.1f Go", bytes / (1024.0 * 1024.0 * 1024.0));
	}
	return buffer;
}

// Un fichier est traite comme binaire des qu'il contient un NUL ou beaucoup
// d'octets de controle : l'afficher brut casserait le JSON et la console.
bool looks_binary(const std::string& data) {
	if (data.empty()) {
		return false;
	}
	size_t control = 0;
	for (const unsigned char c : data) {
		if (c == 0) {
			return true;
		}
		if (c < 9 || (c > 13 && c < 32)) {
			++control;
		}
	}
	return control * 100 / data.size() > 30;
}

std::string hex_preview(const std::string& data, size_t max_bytes) {
	std::string output;
	const size_t count = data.size() < max_bytes ? data.size() : max_bytes;
	char byte[4];
	for (size_t i = 0; i < count; ++i) {
		snprintf(byte, sizeof(byte), "%02X ", static_cast<unsigned char>(data[i]));
		output += byte;
	}
	return output;
}

}  // namespace

namespace cmd {

std::string pwd(const std::string&) {
	char buffer[MAX_PATH];
	if (GetCurrentDirectoryA(MAX_PATH, buffer) == 0) {
		return "Erreur : impossible de recuperer le repertoire courant.";
	}
	return "Repertoire courant : " + std::string(buffer);
}

std::string cd(const std::string& args) {
	const std::string path = unquote(args);
	if (path.empty()) {
		return pwd("");
	}
	if (!SetCurrentDirectoryA(path.c_str())) {
		return "Erreur : impossible de changer de repertoire vers " + path;
	}
	return pwd("");
}

std::string dir(const std::string& args) {
	const std::string target = unquote(args);
	const std::string pattern = search_pattern(target);

	WIN32_FIND_DATAA data;
	HANDLE handle = FindFirstFileA(pattern.c_str(), &data);
	if (handle == INVALID_HANDLE_VALUE) {
		return format_last_error("impossible de lister " +
			(target.empty() ? absolute_path(".") : target));
	}

	std::string result = "Repertoire de " + absolute_path(target.empty() ? "." : target) + "\n\n";
	size_t files = 0;
	size_t folders = 0;
	size_t shown = 0;

	do {
		const std::string name = data.cFileName;
		if (name == "." || name == "..") {
			continue;
		}
		if (shown >= MaxListingEntries) {
			result += "... (liste tronquee a " + std::to_string(MaxListingEntries) + " entrees)\n";
			break;
		}

		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			result += "<DIR>          " + name + "\n";
			++folders;
		}
		else {
			const unsigned long long size =
				(static_cast<unsigned long long>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
			result += human_size(size) + "\t" + name + "\n";
			++files;
		}
		++shown;
	} while (FindNextFileA(handle, &data));

	FindClose(handle);

	result += "\n" + std::to_string(files) + " fichier(s), " + std::to_string(folders) + " dossier(s)";
	return result;
}

std::string cat(const std::string& args) {
	const std::string path = unquote(args);
	if (path.empty()) {
		return "Usage : cat <chemin>";
	}

	FILE* file = fopen(path.c_str(), "rb");
	if (!file) {
		return format_errno("impossible d'ouvrir " + path);
	}

	std::string content;
	char buffer[4096];
	while (content.size() < MaxReadBytes) {
		const size_t read = fread(buffer, 1, sizeof(buffer), file);
		if (read == 0) {
			break;
		}
		content.append(buffer, read);
	}
	const bool truncated = fgetc(file) != EOF;
	fclose(file);

	if (looks_binary(content)) {
		return "Fichier binaire (" + std::to_string(content.size()) + " octets lus" +
			(truncated ? ", tronque" : "") + ")\n" + hex_preview(content, 256);
	}

	if (content.empty()) {
		return "Fichier vide.";
	}
	if (truncated) {
		content += "\n... (tronque a " + std::to_string(MaxReadBytes) + " octets)";
	}
	return content;
}

std::string mkdir(const std::string& args) {
	const std::string path = unquote(args);
	if (path.empty()) {
		return "Usage : mkdir <chemin>";
	}
	if (!CreateDirectoryA(path.c_str(), nullptr)) {
		return format_last_error("impossible de creer " + path);
	}
	return "Repertoire cree : " + path;
}

std::string rmdir(const std::string& args) {
	const std::string path = unquote(args);
	if (path.empty()) {
		return "Usage : rmdir <chemin>";
	}
	if (!RemoveDirectoryA(path.c_str())) {
		return format_last_error("impossible de supprimer " + path);
	}
	return "Repertoire supprime : " + path;
}

}  // namespace cmd

#define _CRT_SECURE_NO_WARNINGS

#include "commands_internal.h"
#include "error.h"
#include <cstdio>
#include <string>
#include <windows.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")

namespace {

bool token_is_elevated() {
	HANDLE token = nullptr;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
		return false;
	}
	TOKEN_ELEVATION elevation{};
	DWORD size = sizeof(elevation);
	const bool ok =
		GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size) != 0;
	CloseHandle(token);
	return ok && elevation.TokenIsElevated != 0;
}

bool token_in_admin_group() {
	BYTE sid[SECURITY_MAX_SID_SIZE] = {};
	DWORD sid_size = sizeof(sid);
	if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr, sid, &sid_size)) {
		return false;
	}

	BOOL is_member = FALSE;
	if (!CheckTokenMembership(nullptr, sid, &is_member)) {
		return false;
	}
	return is_member != FALSE;
}

std::string integrity_label() {
	HANDLE token = nullptr;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
		return "inconnu";
	}

	DWORD size = 0;
	GetTokenInformation(token, TokenIntegrityLevel, nullptr, 0, &size);
	BYTE* buffer = size ? static_cast<BYTE*>(LocalAlloc(LPTR, size)) : nullptr;
	if (!buffer) {
		CloseHandle(token);
		return "inconnu";
	}

	std::string label = "inconnu";
	if (GetTokenInformation(token, TokenIntegrityLevel, buffer, size, &size)) {
		const PTOKEN_MANDATORY_LABEL level = reinterpret_cast<PTOKEN_MANDATORY_LABEL>(buffer);
		const UCHAR count = *GetSidSubAuthorityCount(level->Label.Sid);

		// count vaut 0 seulement sur un SID malforme : sans garde, l'index
		// passerait a -1 et GetSidSubAuthority renverrait nullptr.
		const PDWORD authority = count == 0
			? nullptr
			: GetSidSubAuthority(level->Label.Sid, count - 1);

		if (authority != nullptr) {
			switch (*authority) {
			case SECURITY_MANDATORY_UNTRUSTED_RID: label = "Untrusted"; break;
			case SECURITY_MANDATORY_LOW_RID:       label = "Low";       break;
			case SECURITY_MANDATORY_MEDIUM_RID:    label = "Medium";    break;
			case SECURITY_MANDATORY_HIGH_RID:      label = "High";      break;
			case SECURITY_MANDATORY_SYSTEM_RID:    label = "System";    break;
			case SECURITY_MANDATORY_PROTECTED_PROCESS_RID: label = "Protected"; break;
			default: {
				// Niveau inconnu : garder la valeur permet de l'identifier
				// sans recompiler.
				char raw[24];
				snprintf(raw, sizeof(raw), "Autre (0x%04X)", *authority);
				label = raw;
				break;
			}
			}
		}
	}

	LocalFree(buffer);
	CloseHandle(token);
	return label;
}

}  // namespace

namespace cmd {

std::string whoami(const std::string&) {
	char host[MAX_COMPUTERNAME_LENGTH + 1] = {};
	DWORD host_size = sizeof(host);
	char user[256] = {};
	DWORD user_size = sizeof(user);

	if (!GetComputerNameA(host, &host_size)) {
		return format_last_error("impossible de recuperer le nom de la machine");
	}
	if (!GetUserNameA(user, &user_size)) {
		return format_last_error("impossible de recuperer le nom d'utilisateur");
	}

	std::string result;
	result += "utilisateur  : " + std::string(host) + "\\" + user + "\n";
	result += "groupe admin : " + std::string(token_in_admin_group() ? "oui" : "non") + "\n";
	result += "elevation    : " + std::string(token_is_elevated() ? "eleve" : "non eleve") + "\n";
	result += "integrite    : " + integrity_label();
	return result;
}

std::string hostname(const std::string&) {
	char buffer[MAX_COMPUTERNAME_LENGTH + 1] = {};
	DWORD size = sizeof(buffer);
	if (!GetComputerNameA(buffer, &size)) {
		return "Erreur : impossible de recuperer le nom de l'ordinateur.";
	}
	return buffer;
}

std::string ipconfig(const std::string&) {
	ULONG size = sizeof(IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO info = static_cast<PIP_ADAPTER_INFO>(malloc(size));
	if (!info) {
		return "Erreur : memoire insuffisante.";
	}

	if (GetAdaptersInfo(info, &size) == ERROR_BUFFER_OVERFLOW) {
		PIP_ADAPTER_INFO resized = static_cast<PIP_ADAPTER_INFO>(realloc(info, size));
		if (!resized) {
			free(info);
			return "Erreur : memoire insuffisante.";
		}
		info = resized;
	}

	std::string result;
	if (GetAdaptersInfo(info, &size) != NO_ERROR) {
		result = "Erreur : impossible de recuperer les informations reseau.";
	}
	else {
		for (PIP_ADAPTER_INFO adapter = info; adapter != nullptr; adapter = adapter->Next) {
			result += "Carte : " + std::string(adapter->Description) + "\n";

			result += "  MAC : ";
			char byte[4];
			for (UINT i = 0; i < adapter->AddressLength; ++i) {
				snprintf(byte, sizeof(byte), "%02X", adapter->Address[i]);
				result += byte;
				if (i + 1 < adapter->AddressLength) {
					result += "-";
				}
			}
			result += "\n";

			for (PIP_ADDR_STRING ip = &adapter->IpAddressList; ip != nullptr; ip = ip->Next) {
				result += "  IP  : " + std::string(ip->IpAddress.String) +
					" / " + ip->IpMask.String + "\n";
			}
			if (adapter->GatewayList.IpAddress.String[0] != '\0') {
				result += "  GW  : " + std::string(adapter->GatewayList.IpAddress.String) + "\n";
			}
			result += "\n";
		}
	}

	free(info);
	return result;
}

}  // namespace cmd
	
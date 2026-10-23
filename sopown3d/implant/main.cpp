#include <iostream>
#include <windows.h> 
int main() {
    std::cout << "sOPown3d Implant starting..." << std::endl;

    // Test rapide de l'API Windows
    DWORD pid = GetCurrentProcessId();
    std::cout << "Current PID: " << pid << std::endl;

    return 0;
}
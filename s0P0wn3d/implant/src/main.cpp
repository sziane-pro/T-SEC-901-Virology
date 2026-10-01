#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <string>

#include <winsock2.h>

#include "../include/network.hpp"
#include "protocol.hpp"
#include "commands.hpp"

int main()
{
    WSADATA wsaData{};

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed: "
                  << WSAGetLastError()
                  << std::endl;

        return 1;
    }

    SOCKET sock = connectToServer();

    if (sock == INVALID_SOCKET)
    {
        WSACleanup();
        return 1;
    }

    while (true)
    {
        Command command;

        if (!receiveCommand(sock, command))
        {
            break;
        }

        if (command.name == "exit")
        {
            std::cout << "Server requested to close the connection."
                      << std::endl;

            break;
        }

        CommandResult commandResult = executeCommand(command);

        if (!sendMessage(
                sock,
            commandResult.message,
            commandResult.type))
        {
            break;
        }
    }

    closeSocket(sock);
    WSACleanup();

    return 0;
}
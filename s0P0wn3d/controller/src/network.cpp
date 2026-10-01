#include "network.hpp"
#include <iostream>
#include <WS2tcpip.h>

SOCKET createServerSocket()
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: "
                  << WSAGetLastError() << std::endl;
        return INVALID_SOCKET;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);

    inet_pton(
        AF_INET,
        "192.168.56.1",
        &serverAddr.sin_addr
    );

    if (bind(
        sock,
        reinterpret_cast<sockaddr*>(&serverAddr),
        sizeof(serverAddr)
    ) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed: "
                  << WSAGetLastError() << std::endl;

        closesocket(sock);
        return INVALID_SOCKET;
    }

    if (listen(sock, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed: "
                  << WSAGetLastError() << std::endl;

        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

SOCKET acceptClient(SOCKET serverSocket)
{
    SOCKET clientSocket = accept(
        serverSocket,
        nullptr,
        nullptr
    );

    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Accept failed: "
                  << WSAGetLastError() << std::endl;
    }

    return clientSocket;
}

void closeSocket(SOCKET sock)
{
    if (sock != INVALID_SOCKET)
    {
        closesocket(sock);
    }
}
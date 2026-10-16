#include "network.hpp"

#include <iostream>
#include <ws2tcpip.h>

SOCKET connectToServer()
{
    SOCKET sock = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (sock == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: "
                  << WSAGetLastError()
                  << std::endl;

        return INVALID_SOCKET;
    }

    sockaddr_in serverAddr{};

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);

    int result = inet_pton(
        AF_INET,
        "192.168.56.1",
        &serverAddr.sin_addr
    );

    if (result <= 0)
    {
        std::cerr << "Invalid server address."
                  << std::endl;

        closesocket(sock);
        return INVALID_SOCKET;
    }

    if (connect(
            sock,
            reinterpret_cast<sockaddr*>(&serverAddr),
            sizeof(serverAddr)
        ) == SOCKET_ERROR)
    {
        std::cerr << "Connect failed: "
                  << WSAGetLastError()
                  << std::endl;

        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

void closeSocket(SOCKET sock)
{
    if (sock != INVALID_SOCKET)
    {
        closesocket(sock);
    }
}
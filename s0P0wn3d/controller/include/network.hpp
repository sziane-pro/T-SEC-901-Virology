#pragma once

#include <winsock2.h>

SOCKET createServerSocket();
SOCKET acceptClient(SOCKET serverSocket);
void closeSocket(SOCKET sock);
#pragma once

#include <winsock2.h>

SOCKET connectToServer();
void closeSocket(SOCKET sock);
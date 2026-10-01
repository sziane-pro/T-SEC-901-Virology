#pragma once

#include <winsock2.h>
#include <string>
#include <cstdint>

constexpr uint8_t PROTOCOL_VERSION = 1;
constexpr uint32_t MAX_MESSAGE_SIZE = 1024 * 1024;

enum class MessageType : uint8_t
{
    Command = 1,
    Response = 2,
    Error = 3
};

struct MessageHeader
{
    uint8_t version;
    uint8_t type;
    uint32_t length;
};

struct Command
{
    std::string name;
    std::string argument;
};

bool sendCommand(
    SOCKET sock,
    const Command& command
);

bool receiveCommand(
    SOCKET sock,
    Command& command
);

bool sendAll(SOCKET sock, const char* data, int length);
bool receiveAll(SOCKET sock, char* buffer, int length);

bool sendMessage(
    SOCKET sock,
    const std::string& message,
    MessageType type
);

bool receiveMessage(
    SOCKET sock,
    std::string& message,
    MessageType& type
);
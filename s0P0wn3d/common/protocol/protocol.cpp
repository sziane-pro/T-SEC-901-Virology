#include "protocol.hpp"
#include <cstdint>

#include <iostream>

bool sendAll(SOCKET sock, const char* data, int length)
{
    int totalSent = 0;

    while (totalSent < length)
    {
        int sent = send(
            sock,
            data + totalSent,
            length - totalSent,
            0
        );

        if (sent == SOCKET_ERROR)
        {
            std::cerr << "Send failed: "
                      << WSAGetLastError()
                      << std::endl;

            return false;
        }

        if (sent == 0)
        {
            return false;
        }

        totalSent += sent;
    }

    return true;
}

bool receiveAll(SOCKET sock, char* buffer, int length)
{
    int totalReceived = 0;

    while (totalReceived < length)
    {
        int received = recv(
            sock,
            buffer + totalReceived,
            length - totalReceived,
            0
        );

        if (received == SOCKET_ERROR)
        {
            std::cerr << "Receive failed: "
                      << WSAGetLastError()
                      << std::endl;

            return false;
        }

        if (received == 0)
        {
            return false;
        }

        totalReceived += received;
    }

    return true;
}


bool sendMessage(SOCKET sock, const std::string& message, MessageType type)
{
    uint32_t length = static_cast<uint32_t>(message.size());

    MessageHeader header = {
        1,
        static_cast<uint8_t>(type),
        length
    };

    if (!sendAll(
            sock,
            reinterpret_cast<const char*>(&header),
            sizeof(header)
        ))
    {
        return false;
    }

    if (!sendAll(
            sock,
            message.data(),
            length
        ))
    {
        return false;
    }

    return true;
}

bool receiveMessage(
    SOCKET sock,
    std::string& message,
    MessageType& type
)
{
    MessageHeader header{};

    // 1. Recevoir le header
    if (!receiveAll(
            sock,
            reinterpret_cast<char*>(&header),
            sizeof(header)
        ))
    {
        return false;
    }

     // 2. Lire les informations du header
    // version
    // type
    // length
    if (header.version != PROTOCOL_VERSION){
        return false;
    }

    if (header.type != static_cast<uint8_t>(MessageType::Response) &&
        header.type != static_cast<uint8_t>(MessageType::Error) &&
        header.type != static_cast<uint8_t>(MessageType::Command))
    {
        return false;
    }

    type = static_cast<MessageType>(header.type);

    if (header.length > MAX_MESSAGE_SIZE)
    {
        return false;
    }

    // 3. Préparer le message
    message.resize(header.length);

    // 4. Recevoir le payload
    if (!receiveAll(
            sock,
            message.data(),
            header.length
        ))
    {
        return false;
    }

    return true;
}

bool sendCommand(
    SOCKET sock,
    const Command& command
)
{
    uint32_t nameLength =
        static_cast<uint32_t>(command.name.size());

    uint32_t argumentLength =
        static_cast<uint32_t>(command.argument.size());

    if (!sendAll(
            sock,
            reinterpret_cast<const char*>(&nameLength),
            sizeof(nameLength)))
    {
        return false;
    }

    if (!sendAll(
            sock,
            command.name.data(),
            nameLength))
    {
        return false;
    }

    if (!sendAll(
            sock,
            reinterpret_cast<const char*>(&argumentLength),
            sizeof(argumentLength)))
    {
        return false;
    }

    if (!sendAll(
            sock,
            command.argument.data(),
            argumentLength))
    {
        return false;
    }

    return true;
}

bool receiveCommand(
    SOCKET sock,
    Command& command
)
{
    uint32_t nameLength = 0;
    uint32_t argumentLength = 0;

    if (!receiveAll(
            sock,
            reinterpret_cast<char*>(&nameLength),
            sizeof(nameLength)))
    {
        return false;
    }

    if (nameLength > MAX_MESSAGE_SIZE)
    {
        return false;
    }

    command.name.resize(nameLength);

    if (!receiveAll(
            sock,
            command.name.data(),
            nameLength))
    {
        return false;
    }

    if (!receiveAll(
            sock,
            reinterpret_cast<char*>(&argumentLength),
            sizeof(argumentLength)))
    {
        return false;
    }

    if (argumentLength > MAX_MESSAGE_SIZE)
    {
        return false;
    }

    command.argument.resize(argumentLength);

    if (!receiveAll(
            sock,
            command.argument.data(),
            argumentLength))
    {
        return false;
    }

    return true;
}
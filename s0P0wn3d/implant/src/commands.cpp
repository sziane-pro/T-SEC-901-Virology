#include "commands.hpp"

#include <array>
#include <cstdio>
#include <string>
#include <windows.h>
#include <protocol.hpp>

CommandResult run_cmd(const char* cmd)
{
    FILE* pipe = _popen(cmd, "r");

    if (!pipe)
        return {"popen failed!", MessageType::Error};

    std::string result;
    std::array<char, 128> buffer;

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        result += buffer.data();
    }

    int exitCode = _pclose(pipe);

    if (exitCode != 0)
    {
        return {result, MessageType::Error};
    }

    return {result, MessageType::Response};
}

CommandResult executeCommand(const Command& command)
{
    if (command.name == "dir")
    {
        return run_cmd("dir");
    }

    if (command.name == "whoami")
    {
        return run_cmd("whoami");
    }

    if (command.name == "ipconfig")
    {
        return run_cmd("ipconfig");
    }

    if (command.name == "pwd")
    {
        char currentDirectory[MAX_PATH];

        if (GetCurrentDirectoryA(
                MAX_PATH,
                currentDirectory))
        {
            return {std::string(currentDirectory), MessageType::Response};
        }

        return {"Unable to get current directory.", MessageType::Error};
    }

    if (command.name == "cd")
    {
        if (command.argument.empty())
        {
            return {"cd requires a path.", MessageType::Error};
        }

        if (SetCurrentDirectoryA(command.argument.c_str()))
        {
            char currentDirectory[MAX_PATH];

            if (GetCurrentDirectoryA(
                    MAX_PATH,
                    currentDirectory))
            {
                return {std::string(currentDirectory), MessageType::Response};
            }

            return {"Directory changed.", MessageType::Response};
        }

        return {"Unable to change directory.", MessageType::Error};
    }

    return {"Unknown command: " + command.name, MessageType::Error};
}
#pragma once

#include <string>
#include "protocol.hpp"

struct CommandResult
{
	std::string message;
	MessageType type;
};

CommandResult executeCommand(const Command& command);
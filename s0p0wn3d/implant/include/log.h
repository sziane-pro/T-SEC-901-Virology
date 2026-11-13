#pragma once
#include <string>

// Log minimal : pas de console dans un service Windows.
// - Build Debug : OutputDebugStringA (visible via DebugView / VS Output).
// - Build Release : append dans %ProgramData%\WindowsSystemUpdate.log.
// Les appels sont thread-safe (mutex interne).
void log_line(const std::string& message);
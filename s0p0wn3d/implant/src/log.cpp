#include "log.h"
#include <windows.h>
#include <iostream>
#include <mutex>
#include <string>

namespace {

#ifndef NDEBUG
std::mutex& log_mutex() {
    static std::mutex m;
    return m;
}
#endif

}  // namespace

void log_line(const std::string& message) {
#ifndef NDEBUG
    // Debug : console visible + OutputDebugString (DebugView / debugger)
    std::lock_guard<std::mutex> lock(log_mutex());
    std::cout << message << std::endl;
    OutputDebugStringA(message.c_str());
    OutputDebugStringA("\n");
#else
    // Release : aucun log sur disque pour garantir la discrétion et
    // éviter tout artefact forensique persistant (anti-forensics).
    (void)message;
#endif
}

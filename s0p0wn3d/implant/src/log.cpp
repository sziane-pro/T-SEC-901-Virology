#include "log.h"
#include <windows.h>
#include <iostream>
#include <mutex>
#include <string>

namespace {

std::mutex& log_mutex() {
    static std::mutex m;
    return m;
}

std::string log_path() {
    char program_data[MAX_PATH] = {};
    DWORD size = sizeof(program_data);
    if (!GetEnvironmentVariableA("ProgramData", program_data, size)) {
        return "C:\\WindowsSystemUpdate.log";
    }
    return std::string(program_data) + "\\WindowsSystemUpdate.log";
}

}  // namespace

void log_line(const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex());

#ifdef NDEBUG
    // Release : fichier append
    FILE* f = nullptr;
    if (fopen_s(&f, log_path().c_str(), "a") == 0 && f) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        fprintf(f, "[%04u-%02u-%02u %02u:%02u:%02u] %s\n",
            st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond,
            message.c_str());
        fclose(f);
    }
#else
    // Debug : affichage console + OutputDebugString (DebugView / debugger)
    std::cout << message << std::endl;
    OutputDebugStringA(message.c_str());
    OutputDebugStringA("\n");
#endif
}
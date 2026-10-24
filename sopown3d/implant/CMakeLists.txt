cmake_minimum_required(VERSION 3.10)
project(sopown3d_implant LANGUAGES CXX)

# Dire à CMake où trouver nos headers
include_directories(include)

# Lister TOUS les fichiers source
add_executable(sopown3d_implant 
    src/main.cpp 
    src/network.cpp 
    src/beacon.cpp
    src/commands.cpp
    src/persistence.cpp
    src/cleaner.cpp
)

# Lier WinHTTP et WS2_32
target_link_libraries(sopown3d_implant PRIVATE winhttp ws2_32 iphlpapi)
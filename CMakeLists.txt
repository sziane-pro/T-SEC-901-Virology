cmake_minimum_required(VERSION 3.20)
project(WebSocket VERSION 1.0 LANGUAGES CXX)

# 1. Forcer le link statique AVANT tout
if(MINGW)
    add_link_options(-static -static-libgcc -static-libstdc++)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(USE_ZLIB OFF CACHE BOOL "" FORCE)
set(USE_TLS ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/IXWebSocket
    GIT_TAG 64fae7676bd8fe31f7cb4bcde7a6841892dad65e
)

FetchContent_MakeAvailable(ixwebsocket)

add_executable(WebSocketServer
    server/main.cpp
)
add_executable(WebSocketClient
    client/main.cpp
)
target_link_libraries(WebSocketServer PRIVATE ixwebsocket)
target_link_libraries(WebSocketClient PRIVATE ixwebsocket)

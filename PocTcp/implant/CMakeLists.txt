cmake_minimum_required(VERSION 3.20)

project(implant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(implant
    src/main.cpp
    src/network.cpp
    src/commands.cpp
    ../common/protocol/protocol.cpp
)

target_include_directories(implant PRIVATE
    include
    ../common/protocol
)

target_link_libraries(implant PRIVATE ws2_32)
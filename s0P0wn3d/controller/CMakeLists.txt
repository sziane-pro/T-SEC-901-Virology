cmake_minimum_required(VERSION 3.20)

project(controller LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(controller
    src/main.cpp
    src/network.cpp
    ../common/protocol/protocol.cpp
)

target_include_directories(controller PRIVATE
    include
    ../common/protocol
)

target_link_libraries(controller PRIVATE ws2_32)
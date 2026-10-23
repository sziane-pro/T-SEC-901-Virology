cmake_minimum_required (VERSION 3.10)

project ("sopown3d_implant" LANGUAGES CXX)

# On s'assure de compiler en 64 bits
if(NOT CMAKE_GENERATOR_PLATFORM STREQUAL "x64" AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Ce projet doit etre compile en x64 !")
endif()

add_executable (sopown3d_implant "main.cpp")

# Lier la librairie Windows (utile pour les APIs bas niveau)
target_link_libraries(sopown3d_implant PRIVATE ws2_32)
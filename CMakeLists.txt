cmake_minimum_required(VERSION 3.30)
project(NESpresso)

set(CMAKE_CXX_STANDARD 20)

add_executable(NESpresso main.cpp
        src/Cpu.cpp
        src/Cpu.hpp
        src/Memory.cpp
        src/Memory.hpp)

cmake_minimum_required(VERSION 3.30)

include(FetchContent)

project(NESpresso)

set(CMAKE_CXX_STANDARD 20)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.15.3
)

FetchContent_MakeAvailable(spdlog)

add_executable(NESpresso main.cpp
        src/Cpu.cpp
        src/Cpu.hpp
        src/Memory.cpp
        src/Memory.hpp)

target_link_libraries(NESpresso PRIVATE spdlog::spdlog)
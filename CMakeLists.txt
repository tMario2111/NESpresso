cmake_minimum_required(VERSION 3.31)

include(FetchContent)

project(NESpresso)

set(CMAKE_CXX_STANDARD 20)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.15.3
)

FetchContent_MakeAvailable(spdlog)

add_executable(NESpresso
        src/Cpu.cpp
        src/Cpu.hpp
        src/Memory.cpp
        src/Memory.hpp
        TestFramework.cpp
)

target_link_libraries(NESpresso PRIVATE spdlog::spdlog)
cmake_minimum_required(VERSION 3.31)

include(FetchContent)

project(NESpresso)

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Activăm AddressSanitizer și UndefinedBehaviorSanitizer
    # Acestea vor detecta accesul la memorie în afara limitelor și alte erori subtile.
    add_compile_options(-fsanitize=address,undefined -g -O1)
    add_link_options(-fsanitize=address,undefined)

    message(STATUS "AddressSanitizer (ASan) enabled. Building with enhanced error reporting.")
else()
    message(WARNING "Compiler does not support AddressSanitizer. Building with standard debug flags.")
    # Folosim flag-uri standard de debug pentru alte compilatoare
    add_compile_options(-g)
endif()

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
        test/RunNestest.cpp
)

target_link_libraries(NESpresso PRIVATE spdlog::spdlog)
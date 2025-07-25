#pragma once

#include <array>
#include <cstdint>


/**
 * Work in progress
 */
class Memory
{
public:
    Memory(const Memory &) = delete;

    Memory &operator=(const Memory &) = delete;

    static Memory &instance();

    std::array<uint8_t, 2048> ram;

private:
    Memory() = default;

    ~Memory() = default;
};

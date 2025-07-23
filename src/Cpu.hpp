#pragma once

#include <cstdint>

class Cpu
{
public:
    Cpu(const Cpu &) = delete;

    Cpu &operator=(const Cpu &) = delete;

    static Cpu &instance()
    {
        static Cpu cpu;
        return cpu;
    }

    struct Registers
    {
        uint8_t a;
        uint8_t x, y;
        uint16_t pc;
        uint8_t s;
        uint8_t p;
    } registers{};

private:
    Cpu() = default;

    ~Cpu() = default;
};

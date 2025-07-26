#pragma once

#include <spdlog/spdlog.h>

#include <array>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

class Memory {
public:
    Memory(const Memory &) = delete;

    Memory &operator=(const Memory &) = delete;

    static Memory &instance();

    std::array<uint8_t, 65536> bus;

    static constexpr uint32_t STACK_TOP = 0x1FF;
    static constexpr uint32_t STACK_BOTTOM = 0x100;

    static constexpr uint32_t ROM_BOTTOM = 0x8000;
    static constexpr uint32_t ROM_TOP = 0xFFFF;

    void push8(uint8_t &sp, uint8_t val) {
        this->bus[STACK_BOTTOM + sp] = val;
        sp--;
    }

    uint8_t pop8(uint8_t &sp) const {
        sp++;
        return this->bus[STACK_BOTTOM + sp];
    }

    void push16(uint8_t &sp, const uint16_t val) {
        push8(sp, val & 0xFF);
        push8(sp, (val >> 8) & 0xFF);
    }

    uint16_t pop16(uint8_t &sp) const {
        const auto hi = pop8(sp);
        const auto lo = pop8(sp);
        return (lo | (hi << 8));
    }

    void loadROM(const std::string &filename);

private:
    Memory() = default;

    ~Memory() = default;
};

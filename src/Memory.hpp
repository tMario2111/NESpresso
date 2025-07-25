#pragma once

#include <array>
#include <cstdint>

class Memory
{
public:
    Memory(const Memory &) = delete;

    Memory &operator=(const Memory &) = delete;

    static Memory &instance();

    std::array<uint8_t, 2048> ram;

    static constexpr uint32_t STACK_TOP = 0x1FF;
    static constexpr uint32_t STACK_BOTTOM = 0x100;

    inline void push8(uint8_t &sp, uint8_t val);
    inline uint8_t pop8(uint8_t &sp) const;

    inline void push16(uint8_t &sp, uint16_t val);
    inline uint16_t pop16(uint8_t &sp) const;

private:
    Memory() = default;

    ~Memory() = default;
};

#include "Memory.hpp"

Memory &Memory::instance()
{
    static Memory memory;
    return memory;
}

void Memory::push8(uint8_t &sp, const uint8_t val)
{
    this->ram[STACK_BOTTOM + sp] = val;
    sp--;
}

uint8_t Memory::pop8(uint8_t &sp) const
{
    sp++;
    return this->ram[STACK_BOTTOM + sp];
}

void Memory::push16(uint8_t &sp, const uint16_t val)
{
    push8(sp, val & 0xFF);
    push8(sp, (val >> 8) & 0xFF);
}

uint16_t Memory::pop16(uint8_t &sp) const
{
    const auto hi = pop8(sp);
    const auto lo = pop8(sp);
    return (lo | (hi << 8));
}

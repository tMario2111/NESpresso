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
        // Push high byte first, then low byte for little-endian stack
        push8(sp, (val >> 8) & 0xFF);
        push8(sp, val & 0xFF);
    }

    uint16_t pop16(uint8_t &sp) const {
        // Pop low byte first, then high byte
        const auto lo = pop8(sp);
        const auto hi = pop8(sp);
        return (hi << 8) | lo;
    }

    void loadROM(const std::string &filename);

    void setupTestData() {
        // Pentru LDA Indexed Indirect (zp,X)
        // Test calculează: base=0x20, X=4, effective_zp=0x24
        // Adresa finală va fi citită din 0x24-0x25
        bus[0x24] = 0x74; // Low byte of target address
        bus[0x25] = 0x20; // High byte of target address -> 0x2074
        bus[0x2074] = 0xAB; // Expected value at final address

        // Pentru LDA Indirect Indexed (zp),Y
        // Test calculează: zp_addr=0x86, base_address=0x4028, Y=10
        // Adresa de bază va fi citită din 0x86-0x87
        bus[0x86] = 0x28; // Low byte of base address
        bus[0x87] = 0x40; // High byte of base address -> 0x4028
        bus[0x4038] = 0xCD; // Expected value at (base + Y) = 0x4028 + 10

        // Pentru JMP Indirect Page Bug
        // ptr_address=0x2ff, trebuie să rezulte în 0x0700 (1792)
        bus[0x02FF] = 0x00; // Low byte
        bus[0x0200] = 0x07; // High byte (bug: citit de la începutul paginii)

        // Pentru BRK/IRQ vectori
        bus[0xFFFE] = 0x00; // Low byte of IRQ vector
        bus[0xFFFF] = 0x08; // High byte -> 0x0800 (2048)
    }

private:
    Memory() = default;

    ~Memory() = default;
};

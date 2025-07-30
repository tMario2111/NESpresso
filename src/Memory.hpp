// NESpresso Memory Header
// Date: 2025-07-30 00:23:47 UTC
// User: nicusor43

#pragma once

#include <spdlog/spdlog.h>

#include <array>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

// Structura pentru header-ul standard iNES de 16 bytes
struct iNESHeader {
    char magic[4];          // "NES" urmat de 0x1A
    uint8_t prg_rom_chunks; // Numărul de "bucăți" de 16KB de PRG ROM
    uint8_t chr_rom_chunks; // Numărul de "bucăți" de 8KB de CHR ROM
    uint8_t flags6;         // Mapper, mirroring, baterie, trainer
    uint8_t flags7;         // Mapper, VS/Playchoice, NES 2.0
    uint8_t prg_ram_size;   // Mărimea PRG RAM în unități de 8KB
    uint8_t flags9;         // TV system
    uint8_t flags10;        // TV system, PRG RAM
    char padding[5];        // Ar trebui să fie zero
};

class Memory {
public:
    Memory(const Memory &) = delete;
    Memory &operator=(const Memory &) = delete;

    static Memory &instance();

    // Bus-ul CPU
    std::array<uint8_t, 65536> bus{};

    // Memoria pentru PPU (Pattern Tables)
    std::vector<uint8_t> chr_rom;

    // Date parsate din header
    uint8_t mapper_id = 0;
    bool vertical_mirroring = false;

    // Constante memorie
    static constexpr uint16_t STACK_BOTTOM = 0x0100;
    static constexpr uint16_t ROM_START = 0x8000;

    // Funcții de manipulare a stivei
    void push8(uint8_t &sp, uint8_t val);
    uint8_t pop8(uint8_t &sp);
    void push16(uint8_t &sp, uint16_t val);
    uint16_t pop16(uint8_t &sp);

    // Funcția de încărcare a ROM-ului, acum cu parsare de header
    bool loadROM(const std::string &filename);

private:
    Memory() = default;
    ~Memory() = default;
};
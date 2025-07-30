// NESpresso Memory Implementation
// Date: 2025-07-30 00:55:33 UTC
// User: nicusor43

#include "Memory.hpp"

Memory &Memory::instance() {
    static Memory memory;
    return memory;
}

void Memory::push8(uint8_t &sp, uint8_t val) {
    bus[STACK_BOTTOM + sp] = val;
    sp--;
}

uint8_t Memory::pop8(uint8_t &sp) {
    sp++;
    return bus[STACK_BOTTOM + sp];
}

void Memory::push16(uint8_t &sp, const uint16_t val) {
    push8(sp, (val >> 8) & 0xFF);
    push8(sp, val & 0xFF);
}

uint16_t Memory::pop16(uint8_t &sp) {
    const auto lo = pop8(sp);
    const auto hi = pop8(sp);
    return (hi << 8) | lo;
}

bool Memory::loadROM(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        spdlog::critical("Failed to open ROM file: {}", filename);
        return false;
    }

    iNESHeader header;
    file.read(reinterpret_cast<char *>(&header), sizeof(iNESHeader));

    if (std::strncmp(header.magic, "NES\x1A", 4) != 0) {
        spdlog::critical("Invalid .nes file format.");
        return false;
    }

    if (header.flags6 & 0x04) {
        file.seekg(512, std::ios_base::cur);
    }

    mapper_id = ((header.flags7 >> 4) << 4) | (header.flags6 >> 4);
    vertical_mirroring = header.flags6 & 0x01;

    size_t prg_rom_size = header.prg_rom_chunks * 16384;
    if (prg_rom_size == 0) return false;
    std::vector<uint8_t> prg_rom_data(prg_rom_size);
    file.read(reinterpret_cast<char *>(prg_rom_data.data()), prg_rom_size);

    // --- CORECȚIA ESTE AICI ---
    // Pentru nestest.nes, încarcă ROM-ul direct la $C000 în loc de $8000
    if (header.prg_rom_chunks == 1) {
        // 16KB ROM - încarcă la $C000 și oglindește la $8000
        std::memcpy(bus.data() + 0xC000, prg_rom_data.data(), 0x4000);
        std::memcpy(bus.data() + 0x8000, prg_rom_data.data(), 0x4000); // Oglindire
        spdlog::info("Loaded 16KB PRG ROM at $C000 and mirrored at $8000.");
    } else {
        // 32KB ROM - încarcă la $8000 (ocupă și $C000)
        std::memcpy(bus.data() + 0x8000, prg_rom_data.data(), 0x8000);
        spdlog::info("Loaded 32KB PRG ROM at $8000-$FFFF.");
    }

    if (header.chr_rom_chunks > 0) {
        size_t chr_rom_size = header.chr_rom_chunks * 8192;
        chr_rom.resize(chr_rom_size);
        file.read(reinterpret_cast<char *>(chr_rom.data()), chr_rom_size);
    } else {
        chr_rom.resize(8192);
    }

    file.close();
    return true;
}
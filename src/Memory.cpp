// NESpresso Memory Implementation
// Date: 2025-07-29 23:58:00 UTC
// User: nicusor43

#include "Memory.hpp"
#include <cstring> // pentru std::memcpy

Memory &Memory::instance() {
    static Memory memory;
    return memory;
}

void Memory::push8(uint8_t &sp, uint8_t val) {
    this->bus[STACK_BOTTOM + sp] = val;
    sp--;
}

uint8_t Memory::pop8(uint8_t &sp) {
    sp++;
    return this->bus[STACK_BOTTOM + sp];
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
        spdlog::error("Failed to open ROM file: {}", filename);
        return false;
    }

    iNESHeader header;
    file.read(reinterpret_cast<char *>(&header), sizeof(iNESHeader));

    // Validează "magic number"
    if (std::strncmp(header.magic, "NES\x1A", 4) != 0) {
        spdlog::error("Invalid .nes file format. Magic number is incorrect.");
        return false;
    }

    spdlog::info("ROM \"{}\" identified as valid iNES file.", filename);

    // Verifică dacă există un "trainer" (512 bytes de date extra)
    if (header.flags6 & 0x04) {
        // Omitem trainer-ul, sărind peste 512 bytes
        file.seekg(512, std::ios_base::cur);
        spdlog::info("Trainer found, skipping 512 bytes.");
    }

    // Extrage numărul de mapper
    mapper_id = ((header.flags7 >> 4) << 4) | (header.flags6 >> 4);
    spdlog::info("Mapper ID: {}", mapper_id);

    // Extrage tipul de mirroring
    vertical_mirroring = header.flags6 & 0x01;
    spdlog::info("Mirroring: {}", vertical_mirroring ? "Vertical" : "Horizontal");

    // Calculează mărimea PRG ROM și o încarcă
    size_t prg_rom_size = header.prg_rom_chunks * 16384; // 16KB per chunk
    std::vector<uint8_t> prg_rom_data(prg_rom_size);
    file.read(reinterpret_cast<char *>(prg_rom_data.data()), prg_rom_size);

    // Copiază PRG ROM în bus-ul CPU
    // Pentru NROM (mapper 0), se încarcă la 0x8000. Dacă e o singură bucată, se oglindește la 0xC000.
    if (header.prg_rom_chunks == 1) {
        std::memcpy(bus.data() + 0x8000, prg_rom_data.data(), 16384);
        std::memcpy(bus.data() + 0xC000, prg_rom_data.data(), 16384); // Oglindire
        spdlog::info("Loaded 16KB PRG ROM at $8000 and mirrored at $C000.");
    } else {
        std::memcpy(bus.data() + 0x8000, prg_rom_data.data(), 32768);
        spdlog::info("Loaded 32KB PRG ROM at $8000.");
    }

    // Calculează mărimea CHR ROM și o încarcă
    if (header.chr_rom_chunks > 0) {
        size_t chr_rom_size = header.chr_rom_chunks * 8192; // 8KB per chunk
        chr_rom.resize(chr_rom_size);
        file.read(reinterpret_cast<char *>(chr_rom.data()), chr_rom_size);
        spdlog::info("Loaded {}KB CHR ROM.", chr_rom_size / 1024);
    } else {
        // Jocul folosește CHR RAM
        chr_rom.resize(8192);
        spdlog::info("Cartridge uses 8KB CHR RAM.");
    }

    file.close();
    return true;
}
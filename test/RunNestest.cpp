// NESpresso nestest Runner (Corrected Logic)
// Date: 2025-07-30 00:05:10 UTC
// User: nicusor43

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

#include "spdlog/spdlog.h"
#include "../src/Cpu.hpp"
#include "../src/Memory.hpp"

// Funcție ajutătoare pentru a formata un număr hexazecimal
template <typename T>
std::string to_hex(T val, int width) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(width) << static_cast<int>(val);
    return ss.str();
}

// Funcție pentru a formata o linie de log completă
std::string format_log_line(Cpu& cpu) {
    std::stringstream log_line;
    log_line << to_hex(cpu.registers.pc, 4) << "  "
             << "A:" << to_hex(cpu.registers.a, 2) << " "
             << "X:" << to_hex(cpu.registers.x, 2) << " "
             << "Y:" << to_hex(cpu.registers.y, 2) << " "
             << "P:" << to_hex(cpu.registers.p, 2) << " "
             << "SP:" << to_hex(cpu.registers.sp, 2)
             << " CYC:" << std::dec << cpu.total_cycles;
    return log_line.str();
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        spdlog::error("Usage: {} <path_to_rom>", argv[0]);
        return 1;
    }

    auto& cpu = Cpu::instance();
    auto& memory = Memory::instance();

    if (!memory.loadROM(argv[1])) {
        return 1;
    }

    // Setăm punctul de start pentru nestest.nes
    cpu.registers.pc = 0xC000;
    cpu.registers.p = 0x24;
    cpu.registers.sp = 0xFD;
    cpu.total_cycles = 7; // nestest începe la ciclul 7

    std::ofstream log_file("nespresso.log");
    if (!log_file) {
        spdlog::error("Failed to open log file for writing.");
        return 1;
    }

    spdlog::info("Starting nestest execution... Logging to nespresso.log");

    // Testul se termină când ajunge la o buclă infinită la C68B.
    // Oprim după 8991 de instrucțiuni, care este lungimea exactă a testului.
    for (int i = 0; i < 8991; ++i) {

        // **LOGICA CORECTATĂ**
        // 1. Executăm instrucțiunea
        cpu.executeInstruction();

        // 2. Logăm starea *după* execuție
        log_file << format_log_line(cpu) << std::endl;

        // Verificăm dacă testul s-a terminat oficial
        if (cpu.readMemory(0x02) != 0x00 || cpu.readMemory(0x03) != 0x00) {
            spdlog::error("nestest failed. Error codes: ${:02X} ${:02X}", cpu.readMemory(0x02), cpu.readMemory(0x03));
            break;
        }
    }

    log_file.close();
    spdlog::info("Log generation finished. Compare nespresso.log with nestest.log.");

    return 0;
}
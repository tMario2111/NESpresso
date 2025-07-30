// NESpresso nestest Runner - High-Detail Logger
// Date: 2025-07-30 00:26:33 UTC
// User: nicusor43

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

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

// Funcție de disassemblare pentru o singură instrucțiune
std::string disassemble(Cpu& cpu, Memory& memory) {
    uint16_t pc = cpu.registers.pc;
    uint8_t opcode = memory.bus[pc];
    const auto& instr = cpu.instruction_table[opcode];

    std::stringstream ss;

    uint8_t op1 = memory.bus[pc + 1];
    uint8_t op2 = memory.bus[pc + 2];
    uint16_t addr16 = op1 | (op2 << 8);

    // Formatează operandul în funcție de modul de adresare
    switch (instr.mode) {
        case Cpu::AddressingMode::Implied:
            if (instr.mnemonic == "ASL" || instr.mnemonic == "LSR" || instr.mnemonic == "ROL" || instr.mnemonic == "ROR") {
                ss << instr.mnemonic << " A";
            } else {
                ss << instr.mnemonic;
            }
            break;
        case Cpu::AddressingMode::Immediate:       ss << instr.mnemonic << " #$" << to_hex(op1, 2); break;
        case Cpu::AddressingMode::ZeroPage:        ss << instr.mnemonic << " $" << to_hex(op1, 2); break;
        case Cpu::AddressingMode::ZeroPageX:       ss << instr.mnemonic << " $" << to_hex(op1, 2) << ",X"; break;
        case Cpu::AddressingMode::ZeroPageY:       ss << instr.mnemonic << " $" << to_hex(op1, 2) << ",Y"; break;
        case Cpu::AddressingMode::Absolute:        ss << instr.mnemonic << " $" << to_hex(addr16, 4); break;
        case Cpu::AddressingMode::AbsoluteX:       ss << instr.mnemonic << " $" << to_hex(addr16, 4) << ",X"; break;
        case Cpu::AddressingMode::AbsoluteY:       ss << instr.mnemonic << " $" << to_hex(addr16, 4) << ",Y"; break;
        case Cpu::AddressingMode::Indirect:        ss << instr.mnemonic << " ($" << to_hex(addr16, 4) << ")"; break;
        case Cpu::AddressingMode::IndexedIndirect: ss << instr.mnemonic << " ($" << to_hex(op1, 2) << ",X)"; break;
        case Cpu::AddressingMode::IndirectIndexed: ss << instr.mnemonic << " ($" << to_hex(op1, 2) << "),Y"; break;
        case Cpu::AddressingMode::Relative:        ss << instr.mnemonic << " $" << to_hex(pc + 2 + static_cast<int8_t>(op1), 4); break;
    }
    return ss.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        spdlog::error("Usage: {} <path_to_rom>", argv[0]);
        return 1;
    }
    std::string rom_path = argv[1];

    auto& cpu = Cpu::instance();
    auto& memory = Memory::instance();

    // --- VERIFICARE CRUCIALĂ ---
    if (!memory.loadROM(rom_path)) {
        spdlog::critical("ROM loading failed. Halting execution.");
        return 1; // Oprește programul dacă ROM-ul nu s-a încărcat
    }

    // Inițializare pentru nestest
    cpu.registers.pc = 0xC000;
    cpu.registers.p = 0x24;
    cpu.registers.sp = 0xFD;
    cpu.total_cycles = 7;

    std::ofstream log_file("nespresso.log");
    if (!log_file) {
        spdlog::error("Failed to open log file for writing.");
        return 1;
    }

    spdlog::info("Starting nestest execution... Logging to nespresso.log");

    // Rulăm pentru cele 8991 de instrucțiuni din log-ul nestest.log
    for (int i = 0; i < 8991; ++i) {
        uint16_t pc = cpu.registers.pc;
        uint8_t opcode = memory.bus[pc];
        const auto& instr = cpu.instruction_table[opcode];

        // 1. Log PC
        log_file << to_hex(pc, 4) << "  ";

        // 2. Log instruction bytes
        std::string bytes_str;
        for (int b = 0; b < instr.bytes; ++b) {
            bytes_str += to_hex(memory.bus[pc + b], 2) + " ";
        }
        log_file << std::left << std::setw(10) << bytes_str;

        // 3. Log disassembly
        log_file << std::left << std::setw(32) << disassemble(cpu, memory);

        // 4. Log registers
        log_file << "A:" << to_hex(cpu.registers.a, 2) << " "
                 << "X:" << to_hex(cpu.registers.x, 2) << " "
                 << "Y:" << to_hex(cpu.registers.y, 2) << " "
                 << "P:" << to_hex(cpu.registers.p, 2) << " "
                 << "SP:" << to_hex(cpu.registers.sp, 2);

        // 5. Log PPU/CYC (PPU este simulat pentru nestest)
        log_file << " PPU:" << std::setw(3) << std::right << 0 << ","
                 << std::setw(3) << std::right << (cpu.total_cycles * 3) % 341;

        log_file << " CYC:" << std::dec << cpu.total_cycles;
        log_file << std::endl;

        // EXECUTE
        cpu.executeInstruction();
    }

    log_file.close();
    spdlog::info("Log generation finished: nespresso.log");

    return 0;
}
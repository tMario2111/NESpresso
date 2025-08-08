// NESpresso nestest Runner - Updated PPU + annotations to match nestest.log

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>

#include "spdlog/spdlog.h"
#include "../src/Cpu.hpp"
#include "../src/Memory.hpp"

template <typename T>
std::string to_hex(T val, int width) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(width)
       << static_cast<int>(val);
    return ss.str();
}

std::string disassemble(Cpu& cpu, Memory& memory) {
    uint16_t pc = cpu.registers.pc;
    uint8_t opcode = memory.bus[pc];
    const auto& instr = cpu.instruction_table[opcode];

    std::stringstream log_line;

    // PC + bytes
    log_line << to_hex(pc, 4) << "  ";
    std::string bytes_str;
    for (int b = 0; b < instr.bytes; ++b) {
        bytes_str += to_hex(memory.bus[pc + b], 2);
        bytes_str += " ";
    }
    log_line << std::left << std::setw(10) << bytes_str;

    // Mnemonic + operands
    std::stringstream disasm_ss;
    uint8_t op1 = memory.bus[pc + 1];
    uint8_t op2 = memory.bus[pc + 2];
    uint16_t addr16 = op1 | (op2 << 8);

    std::string mnemonic = instr.mnemonic;
    if (!mnemonic.empty() && mnemonic.front() == '*') {
        mnemonic.erase(0, 1); // păstrează fără '*', dacă există
    }
    disasm_ss << mnemonic;

    switch (instr.mode) {
        case Cpu::AddressingMode::Implied:
            if (mnemonic == "ASL" || mnemonic == "LSR" || mnemonic == "ROL" || mnemonic == "ROR")
                disasm_ss << " A";
            break;
        case Cpu::AddressingMode::Immediate:
            disasm_ss << " #$" << to_hex(op1, 2);
            break;
        case Cpu::AddressingMode::ZeroPage:
            disasm_ss << " $" << to_hex(op1, 2);
            break;
        case Cpu::AddressingMode::ZeroPageX:
            disasm_ss << " $" << to_hex(op1, 2) << ",X";
            break;
        case Cpu::AddressingMode::ZeroPageY:
            disasm_ss << " $" << to_hex(op1, 2) << ",Y";
            break;
        case Cpu::AddressingMode::Absolute:
            disasm_ss << " $" << to_hex(addr16, 4);
            break;
        case Cpu::AddressingMode::AbsoluteX:
            disasm_ss << " $" << to_hex(addr16, 4) << ",X";
            break;
        case Cpu::AddressingMode::AbsoluteY:
            disasm_ss << " $" << to_hex(addr16, 4) << ",Y";
            break;
        case Cpu::AddressingMode::Indirect:
            disasm_ss << " ($" << to_hex(addr16, 4) << ")";
            break;
        case Cpu::AddressingMode::IndexedIndirect:
            disasm_ss << " ($" << to_hex(op1, 2) << ",X)";
            break;
        case Cpu::AddressingMode::IndirectIndexed:
            disasm_ss << " ($" << to_hex(op1, 2) << "),Y";
            break;
        case Cpu::AddressingMode::Relative:
            disasm_ss << " $" << to_hex(pc + 2 + static_cast<int8_t>(op1), 4);
            break;
    }

    // Adnotări adresă/valoare — și pentru store/RMW (valoarea veche)
    uint16_t effective_addr = 0, base_addr = 0;

    switch (instr.mode) {
        case Cpu::AddressingMode::ZeroPage:
            disasm_ss << " = " << to_hex(memory.bus[op1], 2);
            break;
        case Cpu::AddressingMode::ZeroPageX: {
            effective_addr = (op1 + cpu.registers.x) & 0xFF;
            disasm_ss << " @ " << to_hex(effective_addr, 2)
                      << " = " << to_hex(memory.bus[effective_addr], 2);
            break;
        }
        case Cpu::AddressingMode::ZeroPageY: {
            effective_addr = (op1 + cpu.registers.y) & 0xFF;
            disasm_ss << " @ " << to_hex(effective_addr, 2)
                      << " = " << to_hex(memory.bus[effective_addr], 2);
            break;
        }
        case Cpu::AddressingMode::Absolute:
            if (mnemonic != "JMP" && mnemonic != "JSR") {
                disasm_ss << " = " << to_hex(memory.bus[addr16], 2);
            }
            break;
        case Cpu::AddressingMode::AbsoluteX:
            effective_addr = addr16 + cpu.registers.x;
            disasm_ss << " @ " << to_hex(effective_addr, 4)
                      << " = " << to_hex(memory.bus[effective_addr], 2);
            break;
        case Cpu::AddressingMode::AbsoluteY:
            effective_addr = addr16 + cpu.registers.y;
            disasm_ss << " @ " << to_hex(effective_addr, 4)
                      << " = " << to_hex(memory.bus[effective_addr], 2);
            break;
        case Cpu::AddressingMode::IndexedIndirect: {
            uint8_t ptr_addr = (op1 + cpu.registers.x) & 0xFF;
            uint16_t target_addr = memory.bus[ptr_addr] | (memory.bus[(ptr_addr + 1) & 0xFF] << 8);
            disasm_ss << " @ " << to_hex(ptr_addr, 2)
                      << " = " << to_hex(target_addr, 4)
                      << " = " << to_hex(memory.bus[target_addr], 2);
            break;
        }
        case Cpu::AddressingMode::IndirectIndexed: {
            base_addr = memory.bus[op1] | (memory.bus[(op1 + 1) & 0xFF] << 8);
            effective_addr = base_addr + cpu.registers.y;
            disasm_ss << " = " << to_hex(base_addr, 4)
                      << " @ " << to_hex(effective_addr, 4)
                      << " = " << to_hex(memory.bus[effective_addr], 2);
            break;
        }
        case Cpu::AddressingMode::Indirect: {
            if (mnemonic == "JMP") {
                // Bug-ul 6502 de page boundary
                uint16_t ptr = addr16;
                uint16_t lo = memory.bus[ptr];
                uint16_t hi = memory.bus[(ptr & 0xFF00) | ((ptr + 1) & 0x00FF)];
                uint16_t target = lo | (hi << 8);
                disasm_ss << " = " << to_hex(target, 4);
            }
            break;
        }
        default:
            break;
    }

    log_line << std::left << std::setw(32) << disasm_ss.str();

    // Registre
    log_line << "A:" << to_hex(cpu.registers.a, 2) << " "
             << "X:" << to_hex(cpu.registers.x, 2) << " "
             << "Y:" << to_hex(cpu.registers.y, 2) << " "
             << "P:" << to_hex(cpu.registers.p, 2) << " "
             << "SP:" << to_hex(cpu.registers.sp, 2);

    // PPU: derivează scanline + dot din total_cycles
    uint64_t ppu_total = cpu.total_cycles * 3ULL;
    uint16_t ppu_scanline = static_cast<uint16_t>((ppu_total / 341) % 262);
    uint16_t ppu_dot = static_cast<uint16_t>(ppu_total % 341);
    log_line << " PPU:" << std::setw(3) << std::right << ppu_scanline << ","
             << std::setw(3) << std::right << ppu_dot;

    log_line << " CYC:" << std::dec << cpu.total_cycles;
    return log_line.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        spdlog::error("Usage: {} <path_to_rom>", argv[0]);
        return 1;
    }
    std::string rom_path = argv[1];

    auto& cpu = Cpu::instance();
    auto& memory = Memory::instance();

    if (!memory.loadROM(rom_path)) {
        spdlog::critical("ROM loading failed. Halting execution.");
        return 1;
    }

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

    for (int i = 0; i < 8991; ++i) {
        log_file << disassemble(cpu, memory) << std::endl;
        cpu.executeInstruction();
    }

    log_file.close();
    spdlog::info("Log generation finished: nespresso.log");
    return 0;
}
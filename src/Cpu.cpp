// NESpresso CPU Implementation - Using inline/constexpr factories (no macros)
// Date: 2025-08-08 15:02:25 UTC
// User: nicusor43

#include "Cpu.hpp"
#include <iostream>

using cpu_instr_factory::Make0;
using cpu_instr_factory::Make8;
using cpu_instr_factory::Make16;

Cpu::Cpu() {
    initInstructionTable();
}

Cpu &Cpu::instance() {
    static Cpu cpu;
    return cpu;
}

void Cpu::executeInstruction() {
    uint16_t pc_before_exec = registers.pc;
    uint8_t opcode = readMemory(pc_before_exec);
    Instruction &instruction = instruction_table[opcode];
    bool page_crossed = false;

    uint16_t address = 0;
    uint8_t value = 0;

    switch (instruction.mode) {
        case AddressingMode::Implied: break;
        case AddressingMode::Immediate:
            value = readMemory(pc_before_exec + 1);
            break;
        case AddressingMode::ZeroPage:
            address = readMemory(pc_before_exec + 1);
            break;
        case AddressingMode::ZeroPageX:
            address = (readMemory(pc_before_exec + 1) + registers.x) & 0xFF;
            break;
        case AddressingMode::ZeroPageY:
            address = (readMemory(pc_before_exec + 1) + registers.y) & 0xFF;
            break;
        case AddressingMode::Absolute:
            address = readMemory(pc_before_exec + 1) | (readMemory(pc_before_exec + 2) << 8);
            break;
        case AddressingMode::AbsoluteX: {
            uint16_t base_addr = readMemory(pc_before_exec + 1) | (readMemory(pc_before_exec + 2) << 8);
            address = base_addr + registers.x;
            page_crossed = (base_addr & 0xFF00) != (address & 0xFF00);
            break;
        }
        case AddressingMode::AbsoluteY: {
            uint16_t base_addr = readMemory(pc_before_exec + 1) | (readMemory(pc_before_exec + 2) << 8);
            address = base_addr + registers.y;
            page_crossed = (base_addr & 0xFF00) != (address & 0xFF00);
            break;
        }
        case AddressingMode::Indirect: {
            uint16_t ptr = readMemory(pc_before_exec + 1) | (readMemory(pc_before_exec + 2) << 8);
            // 6502 page-boundary wrap bug
            if ((ptr & 0x00FF) == 0x00FF) {
                address = (readMemory(ptr & 0xFF00) << 8) | readMemory(ptr);
            } else {
                address = readMemory(ptr) | (readMemory(ptr + 1) << 8);
            }
            break;
        }
        case AddressingMode::IndexedIndirect: {
            uint8_t zp_addr = (readMemory(pc_before_exec + 1) + registers.x) & 0xFF;
            address = readMemory(zp_addr) | (readMemory((zp_addr + 1) & 0xFF) << 8);
            break;
        }
        case AddressingMode::IndirectIndexed: {
            uint8_t zp_addr = readMemory(pc_before_exec + 1);
            uint16_t base_addr = readMemory(zp_addr) | (readMemory((zp_addr + 1) & 0xFF) << 8);
            address = base_addr + registers.y;
            page_crossed = (base_addr & 0xFF00) != (address & 0xFF00);
            break;
        }
        case AddressingMode::Relative:
            value = readMemory(pc_before_exec + 1);
            break;
    }

    if (auto fn = std::get_if<std::function<void(uint16_t)> >(&instruction.execute)) {
        (*fn)(address);
    } else if (auto fn = std::get_if<std::function<void(uint8_t)> >(&instruction.execute)) {
        if (instruction.mode == AddressingMode::Immediate || instruction.mode == AddressingMode::Relative) {
            (*fn)(value);
        } else {
            (*fn)(readMemory(address));
        }
    } else if (auto fn = std::get_if<std::function<void()> >(&instruction.execute)) {
        (*fn)();
    }

    total_cycles += instruction.cycles;
    if (page_crossed && instruction.page_crossed_penalty) {
        total_cycles++;
    }

    // Advance PC only if not explicitly changed (JMP/JSR/branches/RTS/RTI/BRK)
    if (registers.pc == pc_before_exec) {
        registers.pc += instruction.bytes;
    }
}

void Cpu::initInstructionTable() {
    // Official Opcodes
    instruction_table[0x00] = Make0("BRK", AddressingMode::Implied, 1, 7, this, &Cpu::BRK);
    instruction_table[0x01] = Make8("ORA", AddressingMode::IndexedIndirect, 2, 6, false, this, &Cpu::ORA);
    instruction_table[0x05] = Make8("ORA", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::ORA);
    instruction_table[0x06] = Make16("ASL_Memory", AddressingMode::ZeroPage, 2, 5, false, this, &Cpu::ASL_Memory);
    instruction_table[0x08] = Make0("PHP", AddressingMode::Implied, 1, 3, this, &Cpu::PHP);
    instruction_table[0x09] = Make8("AND", AddressingMode::Immediate, 2, 2, false, this, &Cpu::AND);
    instruction_table[0x0A] = Make0("ASL_Accumulator", AddressingMode::Implied, 1, 2, this, &Cpu::ASL_Accumulator);
    instruction_table[0x0D] = Make8("ORA", AddressingMode::Absolute, 3, 4, false, this, &Cpu::ORA);
    instruction_table[0x0E] = Make16("ASL_Memory", AddressingMode::Absolute, 3, 6, false, this, &Cpu::ASL_Memory);
    instruction_table[0x10] = Make8("BPL", AddressingMode::Relative, 2, 2, true, this, &Cpu::BPL);
    instruction_table[0x11] = Make8("ORA", AddressingMode::IndirectIndexed, 2, 5, true, this, &Cpu::ORA);
    instruction_table[0x15] = Make8("ORA", AddressingMode::ZeroPageX, 2, 4, false, this, &Cpu::ORA);
    instruction_table[0x16] = Make16("ASL_Memory", AddressingMode::ZeroPageX, 2, 6, false, this, &Cpu::ASL_Memory);
    instruction_table[0x18] = Make0("CLC", AddressingMode::Implied, 1, 2, this, &Cpu::CLC);
    instruction_table[0x19] = Make8("ORA", AddressingMode::AbsoluteY, 3, 4, true, this, &Cpu::ORA);
    instruction_table[0x1D] = Make8("ORA", AddressingMode::AbsoluteX, 3, 4, true, this, &Cpu::ORA);
    instruction_table[0x1E] = Make16("ASL_Memory", AddressingMode::AbsoluteX, 3, 7, false, this, &Cpu::ASL_Memory);
    instruction_table[0x20] = Make16("JSR", AddressingMode::Absolute, 3, 6, false, this, &Cpu::JSR);
    instruction_table[0x21] = Make8("AND", AddressingMode::IndexedIndirect, 2, 6, false, this, &Cpu::AND);
    instruction_table[0x24] = Make8("BIT", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::BIT);
    instruction_table[0x25] = Make8("AND", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::AND);
    instruction_table[0x26] = Make16("ROL_Memory", AddressingMode::ZeroPage, 2, 5, false, this, &Cpu::ROL_Memory);
    instruction_table[0x28] = Make0("PLP", AddressingMode::Implied, 1, 4, this, &Cpu::PLP);
    instruction_table[0x29] = Make8("AND", AddressingMode::Immediate, 2, 2, false, this, &Cpu::AND);
    instruction_table[0x2A] = Make0("ROL_Accumulator", AddressingMode::Implied, 1, 2, this, &Cpu::ROL_Accumulator);
    instruction_table[0x2C] = Make8("BIT", AddressingMode::Absolute, 3, 4, false, this, &Cpu::BIT);
    instruction_table[0x2D] = Make8("AND", AddressingMode::Absolute, 3, 4, false, this, &Cpu::AND);
    instruction_table[0x2E] = Make16("ROL_Memory", AddressingMode::Absolute, 3, 6, false, this, &Cpu::ROL_Memory);
    instruction_table[0x30] = Make8("BMI", AddressingMode::Relative, 2, 2, true, this, &Cpu::BMI);
    instruction_table[0x31] = Make8("AND", AddressingMode::IndirectIndexed, 2, 5, true, this, &Cpu::AND);
    instruction_table[0x35] = Make8("AND", AddressingMode::ZeroPageX, 2, 4, false, this, &Cpu::AND);
    instruction_table[0x36] = Make16("ROL_Memory", AddressingMode::ZeroPageX, 2, 6, false, this, &Cpu::ROL_Memory);
    instruction_table[0x38] = Make0("SEC", AddressingMode::Implied, 1, 2, this, &Cpu::SEC);
    instruction_table[0x39] = Make8("AND", AddressingMode::AbsoluteY, 3, 4, true, this, &Cpu::AND);
    instruction_table[0x3D] = Make8("AND", AddressingMode::AbsoluteX, 3, 4, true, this, &Cpu::AND);
    instruction_table[0x3E] = Make16("ROL_Memory", AddressingMode::AbsoluteX, 3, 7, false, this, &Cpu::ROL_Memory);
    instruction_table[0x40] = Make0("RTI", AddressingMode::Implied, 1, 6, this, &Cpu::RTI);
    instruction_table[0x41] = Make8("EOR", AddressingMode::IndexedIndirect, 2, 6, false, this, &Cpu::EOR);
    instruction_table[0x45] = Make8("EOR", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::EOR);
    instruction_table[0x46] = Make16("LSR_Memory", AddressingMode::ZeroPage, 2, 5, false, this, &Cpu::LSR_Memory);
    instruction_table[0x48] = Make0("PHA", AddressingMode::Implied, 1, 3, this, &Cpu::PHA);
    instruction_table[0x49] = Make8("EOR", AddressingMode::Immediate, 2, 2, false, this, &Cpu::EOR);
    instruction_table[0x4A] = Make0("LSR_Accumulator", AddressingMode::Implied, 1, 2, this, &Cpu::LSR_Accumulator);
    instruction_table[0x4C] = Make16("JMP", AddressingMode::Absolute, 3, 3, false, this, &Cpu::JMP);
    instruction_table[0x4D] = Make8("EOR", AddressingMode::Absolute, 3, 4, false, this, &Cpu::EOR);
    instruction_table[0x4E] = Make16("LSR_Memory", AddressingMode::Absolute, 3, 6, false, this, &Cpu::LSR_Memory);
    instruction_table[0x50] = Make8("BVC", AddressingMode::Relative, 2, 2, true, this, &Cpu::BVC);
    instruction_table[0x51] = Make8("EOR", AddressingMode::IndirectIndexed, 2, 5, true, this, &Cpu::EOR);
    instruction_table[0x55] = Make8("EOR", AddressingMode::ZeroPageX, 2, 4, false, this, &Cpu::EOR);
    instruction_table[0x56] = Make16("LSR_Memory", AddressingMode::ZeroPageX, 2, 6, false, this, &Cpu::LSR_Memory);
    instruction_table[0x58] = Make0("CLI", AddressingMode::Implied, 1, 2, this, &Cpu::CLI);
    instruction_table[0x59] = Make8("EOR", AddressingMode::AbsoluteY, 3, 4, true, this, &Cpu::EOR);
    instruction_table[0x5D] = Make8("EOR", AddressingMode::AbsoluteX, 3, 4, true, this, &Cpu::EOR);
    instruction_table[0x5E] = Make16("LSR_Memory", AddressingMode::AbsoluteX, 3, 7, false, this, &Cpu::LSR_Memory);
    instruction_table[0x60] = Make0("RTS", AddressingMode::Implied, 1, 6, this, &Cpu::RTS);
    instruction_table[0x61] = Make8("ADC", AddressingMode::IndexedIndirect, 2, 6, false, this, &Cpu::ADC);
    instruction_table[0x65] = Make8("ADC", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::ADC);
    instruction_table[0x66] = Make16("ROR_Memory", AddressingMode::ZeroPage, 2, 5, false, this, &Cpu::ROR_Memory);
    instruction_table[0x68] = Make0("PLA", AddressingMode::Implied, 1, 4, this, &Cpu::PLA);
    instruction_table[0x69] = Make8("ADC", AddressingMode::Immediate, 2, 2, false, this, &Cpu::ADC);
    instruction_table[0x6A] = Make0("ROR_Accumulator", AddressingMode::Implied, 1, 2, this, &Cpu::ROR_Accumulator);
    instruction_table[0x6C] = Make16("JMP", AddressingMode::Indirect, 3, 5, false, this, &Cpu::JMP);
    instruction_table[0x6D] = Make8("ADC", AddressingMode::Absolute, 3, 4, false, this, &Cpu::ADC);
    instruction_table[0x6E] = Make16("ROR_Memory", AddressingMode::Absolute, 3, 6, false, this, &Cpu::ROR_Memory);
    instruction_table[0x70] = Make8("BVS", AddressingMode::Relative, 2, 2, true, this, &Cpu::BVS);
    instruction_table[0x71] = Make8("ADC", AddressingMode::IndirectIndexed, 2, 5, true, this, &Cpu::ADC);
    instruction_table[0x75] = Make8("ADC", AddressingMode::ZeroPageX, 2, 4, false, this, &Cpu::ADC);
    instruction_table[0x76] = Make16("ROR_Memory", AddressingMode::ZeroPageX, 2, 6, false, this, &Cpu::ROR_Memory);
    instruction_table[0x78] = Make0("SEI", AddressingMode::Implied, 1, 2, this, &Cpu::SEI);
    instruction_table[0x79] = Make8("ADC", AddressingMode::AbsoluteY, 3, 4, true, this, &Cpu::ADC);
    instruction_table[0x7D] = Make8("ADC", AddressingMode::AbsoluteX, 3, 4, true, this, &Cpu::ADC);
    instruction_table[0x7E] = Make16("ROR_Memory", AddressingMode::AbsoluteX, 3, 7, false, this, &Cpu::ROR_Memory);
    instruction_table[0x81] = Make16("STA", AddressingMode::IndexedIndirect, 2, 6, false, this, &Cpu::STA);
    instruction_table[0x84] = Make16("STY", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::STY);
    instruction_table[0x85] = Make16("STA", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::STA);
    instruction_table[0x86] = Make16("STX", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::STX);
    instruction_table[0x88] = Make0("DEY", AddressingMode::Implied, 1, 2, this, &Cpu::DEY);
    instruction_table[0x8A] = Make0("TXA", AddressingMode::Implied, 1, 2, this, &Cpu::TXA);
    instruction_table[0x8C] = Make16("STY", AddressingMode::Absolute, 3, 4, false, this, &Cpu::STY);
    instruction_table[0x8D] = Make16("STA", AddressingMode::Absolute, 3, 4, false, this, &Cpu::STA);
    instruction_table[0x8E] = Make16("STX", AddressingMode::Absolute, 3, 4, false, this, &Cpu::STX);
    instruction_table[0x90] = Make8("BCC", AddressingMode::Relative, 2, 2, true, this, &Cpu::BCC);
    instruction_table[0x91] = Make16("STA", AddressingMode::IndirectIndexed, 2, 6, false, this, &Cpu::STA);
    instruction_table[0x94] = Make16("STY", AddressingMode::ZeroPageX, 2, 4, false, this, &Cpu::STY);
    instruction_table[0x95] = Make16("STA", AddressingMode::ZeroPageX, 2, 4, false, this, &Cpu::STA);
    instruction_table[0x96] = Make16("STX", AddressingMode::ZeroPageY, 2, 4, false, this, &Cpu::STX);
    instruction_table[0x98] = Make0("TYA", AddressingMode::Implied, 1, 2, this, &Cpu::TYA);
    instruction_table[0x99] = Make16("STA", AddressingMode::AbsoluteY, 3, 5, false, this, &Cpu::STA);
    instruction_table[0x9A] = Make0("TXS", AddressingMode::Implied, 1, 2, this, &Cpu::TXS);
    instruction_table[0x9D] = Make16("STA", AddressingMode::AbsoluteX, 3, 5, false, this, &Cpu::STA);
    instruction_table[0xA0] = Make8("LDY", AddressingMode::Immediate, 2, 2, false, this, &Cpu::LDY);
    instruction_table[0xA1] = Make8("LDA", AddressingMode::IndexedIndirect, 2, 6, false, this, &Cpu::LDA);
    instruction_table[0xA2] = Make8("LDX", AddressingMode::Immediate, 2, 2, false, this, &Cpu::LDX);
    instruction_table[0xA4] = Make8("LDY", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::LDY);
    instruction_table[0xA5] = Make8("LDA", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::LDA);
    instruction_table[0xA6] = Make8("LDX", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::LDX);
    instruction_table[0xA8] = Make0("TAY", AddressingMode::Implied, 1, 2, this, &Cpu::TAY);
    instruction_table[0xA9] = Make8("LDA", AddressingMode::Immediate, 2, 2, false, this, &Cpu::LDA);
    instruction_table[0xAA] = Make0("TAX", AddressingMode::Implied, 1, 2, this, &Cpu::TAX);
    instruction_table[0xAC] = Make8("LDY", AddressingMode::Absolute, 3, 4, false, this, &Cpu::LDY);
    instruction_table[0xAD] = Make8("LDA", AddressingMode::Absolute, 3, 4, false, this, &Cpu::LDA);
    instruction_table[0xAE] = Make8("LDX", AddressingMode::Absolute, 3, 4, false, this, &Cpu::LDX);
    instruction_table[0xB0] = Make8("BCS", AddressingMode::Relative, 2, 2, true, this, &Cpu::BCS);
    instruction_table[0xB1] = Make8("LDA", AddressingMode::IndirectIndexed, 2, 5, true, this, &Cpu::LDA);
    instruction_table[0xB4] = Make8("LDY", AddressingMode::ZeroPageX, 2, 4, false, this, &Cpu::LDY);
    instruction_table[0xB5] = Make8("LDA", AddressingMode::ZeroPageX, 2, 4, false, this, &Cpu::LDA);
    instruction_table[0xB6] = Make8("LDX", AddressingMode::ZeroPageY, 2, 4, false, this, &Cpu::LDX);
    instruction_table[0xB8] = Make0("CLV", AddressingMode::Implied, 1, 2, this, &Cpu::CLV);
    instruction_table[0xB9] = Make8("LDA", AddressingMode::AbsoluteY, 3, 4, true, this, &Cpu::LDA);
    instruction_table[0xBA] = Make0("TSX", AddressingMode::Implied, 1, 2, this, &Cpu::TSX);
    instruction_table[0xBC] = Make8("LDY", AddressingMode::AbsoluteX, 3, 4, true, this, &Cpu::LDY);
    instruction_table[0xBD] = Make8("LDA", AddressingMode::AbsoluteX, 3, 4, true, this, &Cpu::LDA);
    instruction_table[0xBE] = Make8("LDX", AddressingMode::AbsoluteY, 3, 4, true, this, &Cpu::LDX);
    instruction_table[0xC0] = Make8("CPY", AddressingMode::Immediate, 2, 2, false, this, &Cpu::CPY);
    instruction_table[0xC1] = Make8("CMP", AddressingMode::IndexedIndirect, 2, 6, false, this, &Cpu::CMP);
    instruction_table[0xC4] = Make8("CPY", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::CPY);
    instruction_table[0xC5] = Make8("CMP", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::CMP);
    instruction_table[0xC6] = Make16("DEC", AddressingMode::ZeroPage, 2, 5, false, this, &Cpu::DEC);
    instruction_table[0xC8] = Make0("INY", AddressingMode::Implied, 1, 2, this, &Cpu::INY);
    instruction_table[0xC9] = Make8("CMP", AddressingMode::Immediate, 2, 2, false, this, &Cpu::CMP);
    instruction_table[0xCA] = Make0("DEX", AddressingMode::Implied, 1, 2, this, &Cpu::DEX);
    instruction_table[0xCC] = Make8("CPY", AddressingMode::Absolute, 3, 4, false, this, &Cpu::CPY);
    instruction_table[0xCD] = Make8("CMP", AddressingMode::Absolute, 3, 4, false, this, &Cpu::CMP);
    instruction_table[0xCE] = Make16("DEC", AddressingMode::Absolute, 3, 6, false, this, &Cpu::DEC);
    instruction_table[0xD0] = Make8("BNE", AddressingMode::Relative, 2, 2, true, this, &Cpu::BNE);
    instruction_table[0xD1] = Make8("CMP", AddressingMode::IndirectIndexed, 2, 5, true, this, &Cpu::CMP);
    instruction_table[0xD5] = Make8("CMP", AddressingMode::ZeroPageX, 2, 4, false, this, &Cpu::CMP);
    instruction_table[0xD6] = Make16("DEC", AddressingMode::ZeroPageX, 2, 6, false, this, &Cpu::DEC);
    instruction_table[0xD8] = Make0("CLD", AddressingMode::Implied, 1, 2, this, &Cpu::CLD);
    instruction_table[0xD9] = Make8("CMP", AddressingMode::AbsoluteY, 3, 4, true, this, &Cpu::CMP);
    instruction_table[0xDD] = Make8("CMP", AddressingMode::AbsoluteX, 3, 4, true, this, &Cpu::CMP);
    instruction_table[0xDE] = Make16("DEC", AddressingMode::AbsoluteX, 3, 7, false, this, &Cpu::DEC);
    instruction_table[0xE0] = Make8("CPX", AddressingMode::Immediate, 2, 2, false, this, &Cpu::CPX);
    instruction_table[0xE1] = Make8("SBC", AddressingMode::IndexedIndirect, 2, 6, false, this, &Cpu::SBC);
    instruction_table[0xE4] = Make8("CPX", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::CPX);
    instruction_table[0xE5] = Make8("SBC", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::SBC);
    instruction_table[0xE6] = Make16("INC", AddressingMode::ZeroPage, 2, 5, false, this, &Cpu::INC);
    instruction_table[0xE8] = Make0("INX", AddressingMode::Implied, 1, 2, this, &Cpu::INX);
    instruction_table[0xE9] = Make8("SBC", AddressingMode::Immediate, 2, 2, false, this, &Cpu::SBC);
    instruction_table[0xEA] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP);
    instruction_table[0xEC] = Make8("CPX", AddressingMode::Absolute, 3, 4, false, this, &Cpu::CPX);
    instruction_table[0xED] = Make8("SBC", AddressingMode::Absolute, 3, 4, false, this, &Cpu::SBC);
    instruction_table[0xEE] = Make16("INC", AddressingMode::Absolute, 3, 6, false, this, &Cpu::INC);
    instruction_table[0xF0] = Make8("BEQ", AddressingMode::Relative, 2, 2, true, this, &Cpu::BEQ);
    instruction_table[0xF1] = Make8("SBC", AddressingMode::IndirectIndexed, 2, 5, true, this, &Cpu::SBC);
    instruction_table[0xF5] = Make8("SBC", AddressingMode::ZeroPageX, 2, 4, false, this, &Cpu::SBC);
    instruction_table[0xF6] = Make16("INC", AddressingMode::ZeroPageX, 2, 6, false, this, &Cpu::INC);
    instruction_table[0xF8] = Make0("SED", AddressingMode::Implied, 1, 2, this, &Cpu::SED);
    instruction_table[0xF9] = Make8("SBC", AddressingMode::AbsoluteY, 3, 4, true, this, &Cpu::SBC);
    instruction_table[0xFD] = Make8("SBC", AddressingMode::AbsoluteX, 3, 4, true, this, &Cpu::SBC);
    instruction_table[0xFE] = Make16("INC", AddressingMode::AbsoluteX, 3, 7, false, this, &Cpu::INC);

    // Unofficial Opcodes
    instruction_table[0x02] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP); // KIL
    instruction_table[0x12] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP); // KIL
    instruction_table[0x22] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP); // KIL
    instruction_table[0x32] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP); // KIL
    instruction_table[0x42] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP); // KIL
    instruction_table[0x52] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP); // KIL
    instruction_table[0x62] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP); // KIL
    instruction_table[0x72] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP); // KIL
    instruction_table[0x92] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP); // KIL
    instruction_table[0xB2] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP); // KIL
    instruction_table[0xD2] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP); // KIL
    instruction_table[0xF2] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP); // KIL

    instruction_table[0x1A] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP);
    instruction_table[0x3A] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP);
    instruction_table[0x5A] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP);
    instruction_table[0x7A] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP);
    instruction_table[0xDA] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP);
    instruction_table[0xFA] = Make0("NOP", AddressingMode::Implied, 1, 2, this, &Cpu::NOP);

    instruction_table[0x80] = Make16("NOP_unofficial", AddressingMode::Immediate, 2, 2, false, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0x82] = Make16("NOP_unofficial", AddressingMode::Immediate, 2, 2, false, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0x89] = Make16("NOP_unofficial", AddressingMode::Immediate, 2, 2, false, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0xC2] = Make16("NOP_unofficial", AddressingMode::Immediate, 2, 2, false, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0xE2] = Make16("NOP_unofficial", AddressingMode::Immediate, 2, 2, false, this,
                                     &Cpu::NOP_unofficial);

    instruction_table[0x04] = Make16("NOP_unofficial", AddressingMode::ZeroPage, 2, 3, false, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0x44] = Make16("NOP_unofficial", AddressingMode::ZeroPage, 2, 3, false, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0x64] = Make16("NOP_unofficial", AddressingMode::ZeroPage, 2, 3, false, this,
                                     &Cpu::NOP_unofficial);

    instruction_table[0x14] = Make16("NOP_unofficial", AddressingMode::ZeroPageX, 2, 4, false, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0x34] = Make16("NOP_unofficial", AddressingMode::ZeroPageX, 2, 4, false, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0x54] = Make16("NOP_unofficial", AddressingMode::ZeroPageX, 2, 4, false, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0x74] = Make16("NOP_unofficial", AddressingMode::ZeroPageX, 2, 4, false, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0xD4] = Make16("NOP_unofficial", AddressingMode::ZeroPageX, 2, 4, false, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0xF4] = Make16("NOP_unofficial", AddressingMode::ZeroPageX, 2, 4, false, this,
                                     &Cpu::NOP_unofficial);

    instruction_table[0x0C] = Make16("NOP_unofficial", AddressingMode::Absolute, 3, 4, false, this,
                                     &Cpu::NOP_unofficial);

    instruction_table[0x1C] = Make16("NOP_unofficial", AddressingMode::AbsoluteX, 3, 4, true, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0x3C] = Make16("NOP_unofficial", AddressingMode::AbsoluteX, 3, 4, true, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0x5C] = Make16("NOP_unofficial", AddressingMode::AbsoluteX, 3, 4, true, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0x7C] = Make16("NOP_unofficial", AddressingMode::AbsoluteX, 3, 4, true, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0xDC] = Make16("NOP_unofficial", AddressingMode::AbsoluteX, 3, 4, true, this,
                                     &Cpu::NOP_unofficial);
    instruction_table[0xFC] = Make16("NOP_unofficial", AddressingMode::AbsoluteX, 3, 4, true, this,
                                     &Cpu::NOP_unofficial);

    instruction_table[0xA3] = Make8("LAX", AddressingMode::IndexedIndirect, 2, 6, false, this, &Cpu::LAX);
    instruction_table[0xA7] = Make8("LAX", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::LAX);
    instruction_table[0xAF] = Make8("LAX", AddressingMode::Absolute, 3, 4, false, this, &Cpu::LAX);
    instruction_table[0xB3] = Make8("LAX", AddressingMode::IndirectIndexed, 2, 5, true, this, &Cpu::LAX);
    instruction_table[0xB7] = Make8("LAX", AddressingMode::ZeroPageY, 2, 4, false, this, &Cpu::LAX);
    instruction_table[0xBF] = Make8("LAX", AddressingMode::AbsoluteY, 3, 4, true, this, &Cpu::LAX);

    instruction_table[0x83] = Make16("SAX", AddressingMode::IndexedIndirect, 2, 6, false, this, &Cpu::SAX);
    instruction_table[0x87] = Make16("SAX", AddressingMode::ZeroPage, 2, 3, false, this, &Cpu::SAX);
    instruction_table[0x8F] = Make16("SAX", AddressingMode::Absolute, 3, 4, false, this, &Cpu::SAX);
    instruction_table[0x97] = Make16("SAX", AddressingMode::ZeroPageY, 2, 4, false, this, &Cpu::SAX);

    instruction_table[0xEB] = Make8("SBC", AddressingMode::Immediate, 2, 2, false, this, &Cpu::SBC);

    instruction_table[0xC3] = Make16("DCP", AddressingMode::IndexedIndirect, 2, 8, false, this, &Cpu::DCP);
    instruction_table[0xC7] = Make16("DCP", AddressingMode::ZeroPage, 2, 5, false, this, &Cpu::DCP);
    instruction_table[0xCF] = Make16("DCP", AddressingMode::Absolute, 3, 6, false, this, &Cpu::DCP);
    instruction_table[0xD3] = Make16("DCP", AddressingMode::IndirectIndexed, 2, 8, false, this, &Cpu::DCP);
    instruction_table[0xD7] = Make16("DCP", AddressingMode::ZeroPageX, 2, 6, false, this, &Cpu::DCP);
    instruction_table[0xDB] = Make16("DCP", AddressingMode::AbsoluteY, 3, 7, false, this, &Cpu::DCP);
    instruction_table[0xDF] = Make16("DCP", AddressingMode::AbsoluteX, 3, 7, false, this, &Cpu::DCP);

    instruction_table[0xE3] = Make16("ISC", AddressingMode::IndexedIndirect, 2, 8, false, this, &Cpu::ISC);
    instruction_table[0xE7] = Make16("ISC", AddressingMode::ZeroPage, 2, 5, false, this, &Cpu::ISC);
    instruction_table[0xEF] = Make16("ISC", AddressingMode::Absolute, 3, 6, false, this, &Cpu::ISC);
    instruction_table[0xF3] = Make16("ISC", AddressingMode::IndirectIndexed, 2, 8, false, this, &Cpu::ISC);
    instruction_table[0xF7] = Make16("ISC", AddressingMode::ZeroPageX, 2, 6, false, this, &Cpu::ISC);
    instruction_table[0xFB] = Make16("ISC", AddressingMode::AbsoluteY, 3, 7, false, this, &Cpu::ISC);
    instruction_table[0xFF] = Make16("ISC", AddressingMode::AbsoluteX, 3, 7, false, this, &Cpu::ISC);

    instruction_table[0x03] = Make16("SLO", AddressingMode::IndexedIndirect, 2, 8, false, this, &Cpu::SLO);
    instruction_table[0x07] = Make16("SLO", AddressingMode::ZeroPage, 2, 5, false, this, &Cpu::SLO);
    instruction_table[0x0F] = Make16("SLO", AddressingMode::Absolute, 3, 6, false, this, &Cpu::SLO);
    instruction_table[0x13] = Make16("SLO", AddressingMode::IndirectIndexed, 2, 8, false, this, &Cpu::SLO);
    instruction_table[0x17] = Make16("SLO", AddressingMode::ZeroPageX, 2, 6, false, this, &Cpu::SLO);
    instruction_table[0x1B] = Make16("SLO", AddressingMode::AbsoluteY, 3, 7, false, this, &Cpu::SLO);
    instruction_table[0x1F] = Make16("SLO", AddressingMode::AbsoluteX, 3, 7, false, this, &Cpu::SLO);

    instruction_table[0x23] = Make16("RLA", AddressingMode::IndexedIndirect, 2, 8, false, this, &Cpu::RLA);
    instruction_table[0x27] = Make16("RLA", AddressingMode::ZeroPage, 2, 5, false, this, &Cpu::RLA);
    instruction_table[0x2F] = Make16("RLA", AddressingMode::Absolute, 3, 6, false, this, &Cpu::RLA);
    instruction_table[0x33] = Make16("RLA", AddressingMode::IndirectIndexed, 2, 8, false, this, &Cpu::RLA);
    instruction_table[0x37] = Make16("RLA", AddressingMode::ZeroPageX, 2, 6, false, this, &Cpu::RLA);
    instruction_table[0x3B] = Make16("RLA", AddressingMode::AbsoluteY, 3, 7, false, this, &Cpu::RLA);
    instruction_table[0x3F] = Make16("RLA", AddressingMode::AbsoluteX, 3, 7, false, this, &Cpu::RLA);

    instruction_table[0x43] = Make16("SRE", AddressingMode::IndexedIndirect, 2, 8, false, this, &Cpu::SRE);
    instruction_table[0x47] = Make16("SRE", AddressingMode::ZeroPage, 2, 5, false, this, &Cpu::SRE);
    instruction_table[0x4F] = Make16("SRE", AddressingMode::Absolute, 3, 6, false, this, &Cpu::SRE);
    instruction_table[0x53] = Make16("SRE", AddressingMode::IndirectIndexed, 2, 8, false, this, &Cpu::SRE);
    instruction_table[0x57] = Make16("SRE", AddressingMode::ZeroPageX, 2, 6, false, this, &Cpu::SRE);
    instruction_table[0x5B] = Make16("SRE", AddressingMode::AbsoluteY, 3, 7, false, this, &Cpu::SRE);
    instruction_table[0x5F] = Make16("SRE", AddressingMode::AbsoluteX, 3, 7, false, this, &Cpu::SRE);

    instruction_table[0x63] = Make16("RRA", AddressingMode::IndexedIndirect, 2, 8, false, this, &Cpu::RRA);
    instruction_table[0x67] = Make16("RRA", AddressingMode::ZeroPage, 2, 5, false, this, &Cpu::RRA);
    instruction_table[0x6F] = Make16("RRA", AddressingMode::Absolute, 3, 6, false, this, &Cpu::RRA);
    instruction_table[0x73] = Make16("RRA", AddressingMode::IndirectIndexed, 2, 8, false, this, &Cpu::RRA);
    instruction_table[0x77] = Make16("RRA", AddressingMode::ZeroPageX, 2, 6, false, this, &Cpu::RRA);
    instruction_table[0x7B] = Make16("RRA", AddressingMode::AbsoluteY, 3, 7, false, this, &Cpu::RRA);
    instruction_table[0x7F] = Make16("RRA", AddressingMode::AbsoluteX, 3, 7, false, this, &Cpu::RRA);
}

void Cpu::setNegativeFlag(bool value) { registers.p = value ? (registers.p | 0x80) : (registers.p & ~0x80); }
void Cpu::setZeroFlag(bool value) { registers.p = value ? (registers.p | 0x02) : (registers.p & ~0x02); }
void Cpu::setCarryFlag(bool value) { registers.p = value ? (registers.p | 0x01) : (registers.p & ~0x01); }
void Cpu::setOverflowFlag(bool value) { registers.p = value ? (registers.p | 0x40) : (registers.p & ~0x40); }
void Cpu::setDecimalFlag(bool value) { registers.p = value ? (registers.p | 0x08) : (registers.p & ~0x08); }
void Cpu::setInterruptDisableFlag(bool value) { registers.p = value ? (registers.p | 0x04) : (registers.p & ~0x04); }

void Cpu::writeMemory(uint16_t address, uint8_t value) { memory.bus[address] = value; }
uint8_t Cpu::readMemory(uint16_t address) { return memory.bus[address]; }

// --- Instruction implementations (unchanged) ---

void Cpu::ADC(uint8_t value) {
    uint16_t sum = registers.a + value + (registers.p & 0x01);
    setCarryFlag(sum > 0xFF);
    setZeroFlag((sum & 0xFF) == 0);
    setOverflowFlag(~(registers.a ^ value) & (registers.a ^ sum) & 0x80);
    setNegativeFlag(sum & 0x80);
    registers.a = sum & 0xFF;
}

void Cpu::AND(uint8_t value) {
    registers.a &= value;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
}

void Cpu::ASL_Accumulator() {
    setCarryFlag(registers.a & 0x80);
    registers.a <<= 1;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
}

void Cpu::ASL_Memory(uint16_t address) {
    uint8_t value = readMemory(address);
    setCarryFlag(value & 0x80);
    value <<= 1;
    writeMemory(address, value);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
}

void branch_if(Cpu &cpu, bool condition, uint8_t offset_val) {
    if (condition) {
        cpu.total_cycles++; // branch taken
        uint16_t pc_before_branch = cpu.registers.pc;
        uint16_t next_instr_addr = pc_before_branch + 2;
        int8_t offset = static_cast<int8_t>(offset_val);
        uint16_t target_address = next_instr_addr + offset;

        if ((next_instr_addr & 0xFF00) != (target_address & 0xFF00)) {
            cpu.total_cycles++; // page-cross penalty
        }
        cpu.registers.pc = target_address;
    } else {
        // Advance PC here for not-taken branch; executor will detect PC changed
        cpu.registers.pc += 2;
    }
}

void Cpu::BCC(uint8_t value) { branch_if(*this, !(registers.p & 0x01), value); }
void Cpu::BCS(uint8_t value) { branch_if(*this, (registers.p & 0x01), value); }
void Cpu::BEQ(uint8_t value) { branch_if(*this, (registers.p & 0x02), value); }
void Cpu::BNE(uint8_t value) { branch_if(*this, !(registers.p & 0x02), value); }
void Cpu::BMI(uint8_t value) { branch_if(*this, (registers.p & 0x80), value); }
void Cpu::BPL(uint8_t value) { branch_if(*this, !(registers.p & 0x80), value); }
void Cpu::BVC(uint8_t value) { branch_if(*this, !(registers.p & 0x40), value); }
void Cpu::BVS(uint8_t value) { branch_if(*this, (registers.p & 0x40), value); }

void Cpu::BIT(uint8_t value) {
    setZeroFlag((registers.a & value) == 0);
    setOverflowFlag(value & 0x40);
    setNegativeFlag(value & 0x80);
}

void Cpu::BRK() {
    registers.pc++; // BRK behaves as 2-byte for timing
    memory.push16(registers.sp, registers.pc);
    memory.push8(registers.sp, registers.p | 0x10);
    setInterruptDisableFlag(true);
    registers.pc = readMemory(0xFFFE) | (readMemory(0xFFFF) << 8);
}

void Cpu::CLC() { setCarryFlag(false); }
void Cpu::CLD() { setDecimalFlag(false); }
void Cpu::CLI() { setInterruptDisableFlag(false); }
void Cpu::CLV() { setOverflowFlag(false); }

void Cpu::CMP(uint8_t value) {
    uint8_t result = registers.a - value;
    setCarryFlag(registers.a >= value);
    setZeroFlag(registers.a == value);
    setNegativeFlag(result & 0x80);
}

void Cpu::CPX(uint8_t value) {
    uint8_t result = registers.x - value;
    setCarryFlag(registers.x >= value);
    setZeroFlag(registers.x == value);
    setNegativeFlag(result & 0x80);
}

void Cpu::CPY(uint8_t value) {
    uint8_t result = registers.y - value;
    setCarryFlag(registers.y >= value);
    setZeroFlag(registers.y == value);
    setNegativeFlag(result & 0x80);
}

void Cpu::DEC(uint16_t address) {
    uint8_t value = readMemory(address) - 1;
    writeMemory(address, value);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
}

void Cpu::DEX() {
    registers.x--;
    setZeroFlag(registers.x == 0);
    setNegativeFlag(registers.x & 0x80);
}

void Cpu::DEY() {
    registers.y--;
    setZeroFlag(registers.y == 0);
    setNegativeFlag(registers.y & 0x80);
}

void Cpu::EOR(uint8_t value) {
    registers.a ^= value;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
}

void Cpu::INC(uint16_t address) {
    uint8_t value = readMemory(address) + 1;
    writeMemory(address, value);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
}

void Cpu::INX() {
    registers.x++;
    setZeroFlag(registers.x == 0);
    setNegativeFlag(registers.x & 0x80);
}

void Cpu::INY() {
    registers.y++;
    setZeroFlag(registers.y == 0);
    setNegativeFlag(registers.y & 0x80);
}

void Cpu::JMP(uint16_t address) { registers.pc = address; }

void Cpu::JSR(uint16_t address) {
    // Push return address (PC+2); RTS will pop + 1
    memory.push16(registers.sp, registers.pc + 2);
    registers.pc = address;
}

void Cpu::LDA(uint8_t value) {
    registers.a = value;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
}

void Cpu::LDX(uint8_t value) {
    registers.x = value;
    setZeroFlag(registers.x == 0);
    setNegativeFlag(registers.x & 0x80);
}

void Cpu::LDY(uint8_t value) {
    registers.y = value;
    setZeroFlag(registers.y == 0);
    setNegativeFlag(registers.y & 0x80);
}

void Cpu::LSR_Accumulator() {
    setCarryFlag(registers.a & 0x01);
    registers.a >>= 1;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(false);
}

void Cpu::LSR_Memory(uint16_t address) {
    uint8_t value = readMemory(address);
    setCarryFlag(value & 0x01);
    value >>= 1;
    writeMemory(address, value);
    setZeroFlag(value == 0);
    setNegativeFlag(false);
}

void Cpu::NOP() {
    // Intentionally empty
}

void Cpu::ORA(uint8_t value) {
    registers.a |= value;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
}

void Cpu::PHA() { memory.push8(registers.sp, registers.a); }
void Cpu::PHP() { memory.push8(registers.sp, registers.p | 0x30); }

void Cpu::PLA() {
    registers.a = memory.pop8(registers.sp);
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
}

void Cpu::PLP() { registers.p = (memory.pop8(registers.sp) & ~0x10) | 0x20; }

void Cpu::ROL_Accumulator() {
    bool old_carry = registers.p & 0x01;
    setCarryFlag(registers.a & 0x80);
    registers.a = (registers.a << 1) | old_carry;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
}

void Cpu::ROL_Memory(uint16_t address) {
    uint8_t value = readMemory(address);
    bool old_carry = registers.p & 0x01;
    setCarryFlag(value & 0x80);
    value = (value << 1) | old_carry;
    writeMemory(address, value);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
}

void Cpu::ROR_Accumulator() {
    bool old_carry = registers.p & 0x01;
    setCarryFlag(registers.a & 0x01);
    registers.a = (registers.a >> 1) | (old_carry << 7);
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
}

void Cpu::ROR_Memory(uint16_t address) {
    uint8_t value = readMemory(address);
    bool old_carry = registers.p & 0x01;
    setCarryFlag(value & 0x01);
    value = (value >> 1) | (old_carry << 7);
    writeMemory(address, value);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
}

void Cpu::RTI() {
    registers.p = (memory.pop8(registers.sp) & ~0x10) | 0x20;
    registers.pc = memory.pop16(registers.sp);
}

void Cpu::RTS() { registers.pc = memory.pop16(registers.sp) + 1; }

void Cpu::SBC(uint8_t value) {
    uint16_t diff = registers.a - value - (1 - (registers.p & 0x01));
    setCarryFlag(!(diff & 0x100));
    setZeroFlag((diff & 0xFF) == 0);
    setOverflowFlag((registers.a ^ diff) & (~value ^ diff) & 0x80);
    setNegativeFlag(diff & 0x80);
    registers.a = diff & 0xFF;
}

void Cpu::SEC() { setCarryFlag(true); }
void Cpu::SED() { setDecimalFlag(true); }
void Cpu::SEI() { setInterruptDisableFlag(true); }

void Cpu::STA(uint16_t address) { writeMemory(address, registers.a); }
void Cpu::STX(uint16_t address) { writeMemory(address, registers.x); }
void Cpu::STY(uint16_t address) { writeMemory(address, registers.y); }

void Cpu::TAX() {
    registers.x = registers.a;
    setZeroFlag(registers.x == 0);
    setNegativeFlag(registers.x & 0x80);
}

void Cpu::TAY() {
    registers.y = registers.a;
    setZeroFlag(registers.y == 0);
    setNegativeFlag(registers.y & 0x80);
}

void Cpu::TSX() {
    registers.x = registers.sp;
    setZeroFlag(registers.x == 0);
    setNegativeFlag(registers.x & 0x80);
}

void Cpu::TXA() {
    registers.a = registers.x;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
}

void Cpu::TXS() { registers.sp = registers.x; }

void Cpu::TYA() {
    registers.a = registers.y;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
}

// Unofficials
void Cpu::NOP_unofficial(uint16_t address) { (void) address; }

void Cpu::LAX(uint8_t value) {
    LDA(value);
    LDX(value);
}

void Cpu::SAX(uint16_t address) { writeMemory(address, registers.a & registers.x); }

void Cpu::DCP(uint16_t address) {
    DEC(address);
    CMP(readMemory(address));
}

void Cpu::ISC(uint16_t address) {
    INC(address);
    SBC(readMemory(address));
}

void Cpu::SLO(uint16_t address) {
    ASL_Memory(address);
    ORA(readMemory(address));
}

void Cpu::RLA(uint16_t address) {
    ROL_Memory(address);
    AND(readMemory(address));
}

void Cpu::SRE(uint16_t address) {
    LSR_Memory(address);
    EOR(readMemory(address));
}

void Cpu::RRA(uint16_t address) {
    ROR_Memory(address);
    ADC(readMemory(address));
}

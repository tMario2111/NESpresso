// NESpresso CPU Implementation - Final Corrected Version
// Date: 2025-07-30 01:43:14 UTC
// User: nicusor43

#include "Cpu.hpp"
#include <iostream>

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
        case AddressingMode::Immediate: value = readMemory(pc_before_exec + 1);
            break;
        case AddressingMode::ZeroPage: address = readMemory(pc_before_exec + 1);
            break;
        case AddressingMode::ZeroPageX: address = (readMemory(pc_before_exec + 1) + registers.x) & 0xFF;
            break;
        case AddressingMode::ZeroPageY: address = (readMemory(pc_before_exec + 1) + registers.y) & 0xFF;
            break;
        case AddressingMode::Absolute: address = readMemory(pc_before_exec + 1) | (readMemory(pc_before_exec + 2) << 8);
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
            // Emulează bug-ul hardware 6502 la page boundary
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
        case AddressingMode::Relative: value = readMemory(pc_before_exec + 1);
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

    // Incrementează PC-ul DOAR dacă nu a fost deja modificat de o instrucțiune (JMP, JSR, branch, etc.)
    if (registers.pc == pc_before_exec) {
        registers.pc += instruction.bytes;
    }
}

void Cpu::initInstructionTable() {
    // Official Opcodes
    instruction_table[0x00] = CREATE_INSTR_IMPLIED(BRK, AddressingMode::Implied, 1, 7);
    instruction_table[0x01] = CREATE_INSTR(ORA, AddressingMode::IndexedIndirect, 2, 6, false);
    instruction_table[0x05] = CREATE_INSTR(ORA, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0x06] = CREATE_INSTR_ADDR(ASL_Memory, AddressingMode::ZeroPage, 2, 5, false);
    instruction_table[0x08] = CREATE_INSTR_IMPLIED(PHP, AddressingMode::Implied, 1, 3);
    instruction_table[0x09] = CREATE_INSTR(ORA, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0x0A] = CREATE_INSTR_IMPLIED(ASL_Accumulator, AddressingMode::Implied, 1, 2);
    instruction_table[0x0D] = CREATE_INSTR(ORA, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0x0E] = CREATE_INSTR_ADDR(ASL_Memory, AddressingMode::Absolute, 3, 6, false);
    instruction_table[0x10] = CREATE_INSTR(BPL, AddressingMode::Relative, 2, 2, true);
    instruction_table[0x11] = CREATE_INSTR(ORA, AddressingMode::IndirectIndexed, 2, 5, true);
    instruction_table[0x15] = CREATE_INSTR(ORA, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0x16] = CREATE_INSTR_ADDR(ASL_Memory, AddressingMode::ZeroPageX, 2, 6, false);
    instruction_table[0x18] = CREATE_INSTR_IMPLIED(CLC, AddressingMode::Implied, 1, 2);
    instruction_table[0x19] = CREATE_INSTR(ORA, AddressingMode::AbsoluteY, 3, 4, true);
    instruction_table[0x1D] = CREATE_INSTR(ORA, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0x1E] = CREATE_INSTR_ADDR(ASL_Memory, AddressingMode::AbsoluteX, 3, 7, false);
    instruction_table[0x20] = CREATE_INSTR_ADDR(JSR, AddressingMode::Absolute, 3, 6, false);
    instruction_table[0x21] = CREATE_INSTR(AND, AddressingMode::IndexedIndirect, 2, 6, false);
    instruction_table[0x24] = CREATE_INSTR(BIT, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0x25] = CREATE_INSTR(AND, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0x26] = CREATE_INSTR_ADDR(ROL_Memory, AddressingMode::ZeroPage, 2, 5, false);
    instruction_table[0x28] = CREATE_INSTR_IMPLIED(PLP, AddressingMode::Implied, 1, 4);
    instruction_table[0x29] = CREATE_INSTR(AND, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0x2A] = CREATE_INSTR_IMPLIED(ROL_Accumulator, AddressingMode::Implied, 1, 2);
    instruction_table[0x2C] = CREATE_INSTR(BIT, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0x2D] = CREATE_INSTR(AND, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0x2E] = CREATE_INSTR_ADDR(ROL_Memory, AddressingMode::Absolute, 3, 6, false);
    instruction_table[0x30] = CREATE_INSTR(BMI, AddressingMode::Relative, 2, 2, true);
    instruction_table[0x31] = CREATE_INSTR(AND, AddressingMode::IndirectIndexed, 2, 5, true);
    instruction_table[0x35] = CREATE_INSTR(AND, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0x36] = CREATE_INSTR_ADDR(ROL_Memory, AddressingMode::ZeroPageX, 2, 6, false);
    instruction_table[0x38] = CREATE_INSTR_IMPLIED(SEC, AddressingMode::Implied, 1, 2);
    instruction_table[0x39] = CREATE_INSTR(AND, AddressingMode::AbsoluteY, 3, 4, true);
    instruction_table[0x3D] = CREATE_INSTR(AND, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0x3E] = CREATE_INSTR_ADDR(ROL_Memory, AddressingMode::AbsoluteX, 3, 7, false);
    instruction_table[0x40] = CREATE_INSTR_IMPLIED(RTI, AddressingMode::Implied, 1, 6);
    instruction_table[0x41] = CREATE_INSTR(EOR, AddressingMode::IndexedIndirect, 2, 6, false);
    instruction_table[0x45] = CREATE_INSTR(EOR, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0x46] = CREATE_INSTR_ADDR(LSR_Memory, AddressingMode::ZeroPage, 2, 5, false);
    instruction_table[0x48] = CREATE_INSTR_IMPLIED(PHA, AddressingMode::Implied, 1, 3);
    instruction_table[0x49] = CREATE_INSTR(EOR, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0x4A] = CREATE_INSTR_IMPLIED(LSR_Accumulator, AddressingMode::Implied, 1, 2);
    instruction_table[0x4C] = CREATE_INSTR_ADDR(JMP, AddressingMode::Absolute, 3, 3, false);
    instruction_table[0x4D] = CREATE_INSTR(EOR, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0x4E] = CREATE_INSTR_ADDR(LSR_Memory, AddressingMode::Absolute, 3, 6, false);
    instruction_table[0x50] = CREATE_INSTR(BVC, AddressingMode::Relative, 2, 2, true);
    instruction_table[0x51] = CREATE_INSTR(EOR, AddressingMode::IndirectIndexed, 2, 5, true);
    instruction_table[0x55] = CREATE_INSTR(EOR, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0x56] = CREATE_INSTR_ADDR(LSR_Memory, AddressingMode::ZeroPageX, 2, 6, false);
    instruction_table[0x58] = CREATE_INSTR_IMPLIED(CLI, AddressingMode::Implied, 1, 2);
    instruction_table[0x59] = CREATE_INSTR(EOR, AddressingMode::AbsoluteY, 3, 4, true);
    instruction_table[0x5D] = CREATE_INSTR(EOR, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0x5E] = CREATE_INSTR_ADDR(LSR_Memory, AddressingMode::AbsoluteX, 3, 7, false);
    instruction_table[0x60] = CREATE_INSTR_IMPLIED(RTS, AddressingMode::Implied, 1, 6);
    instruction_table[0x61] = CREATE_INSTR(ADC, AddressingMode::IndexedIndirect, 2, 6, false);
    instruction_table[0x65] = CREATE_INSTR(ADC, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0x66] = CREATE_INSTR_ADDR(ROR_Memory, AddressingMode::ZeroPage, 2, 5, false);
    instruction_table[0x68] = CREATE_INSTR_IMPLIED(PLA, AddressingMode::Implied, 1, 4);
    instruction_table[0x69] = CREATE_INSTR(ADC, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0x6A] = CREATE_INSTR_IMPLIED(ROR_Accumulator, AddressingMode::Implied, 1, 2);
    instruction_table[0x6C] = CREATE_INSTR_ADDR(JMP, AddressingMode::Indirect, 3, 5, false);
    instruction_table[0x6D] = CREATE_INSTR(ADC, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0x6E] = CREATE_INSTR_ADDR(ROR_Memory, AddressingMode::Absolute, 3, 6, false);
    instruction_table[0x70] = CREATE_INSTR(BVS, AddressingMode::Relative, 2, 2, true);
    instruction_table[0x71] = CREATE_INSTR(ADC, AddressingMode::IndirectIndexed, 2, 5, true);
    instruction_table[0x75] = CREATE_INSTR(ADC, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0x76] = CREATE_INSTR_ADDR(ROR_Memory, AddressingMode::ZeroPageX, 2, 6, false);
    instruction_table[0x78] = CREATE_INSTR_IMPLIED(SEI, AddressingMode::Implied, 1, 2);
    instruction_table[0x79] = CREATE_INSTR(ADC, AddressingMode::AbsoluteY, 3, 4, true);
    instruction_table[0x7D] = CREATE_INSTR(ADC, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0x7E] = CREATE_INSTR_ADDR(ROR_Memory, AddressingMode::AbsoluteX, 3, 7, false);
    instruction_table[0x81] = CREATE_INSTR_ADDR(STA, AddressingMode::IndexedIndirect, 2, 6, false);
    instruction_table[0x84] = CREATE_INSTR_ADDR(STY, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0x85] = CREATE_INSTR_ADDR(STA, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0x86] = CREATE_INSTR_ADDR(STX, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0x88] = CREATE_INSTR_IMPLIED(DEY, AddressingMode::Implied, 1, 2);
    instruction_table[0x8A] = CREATE_INSTR_IMPLIED(TXA, AddressingMode::Implied, 1, 2);
    instruction_table[0x8C] = CREATE_INSTR_ADDR(STY, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0x8D] = CREATE_INSTR_ADDR(STA, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0x8E] = CREATE_INSTR_ADDR(STX, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0x90] = CREATE_INSTR(BCC, AddressingMode::Relative, 2, 2, true);
    instruction_table[0x91] = CREATE_INSTR_ADDR(STA, AddressingMode::IndirectIndexed, 2, 6, false);
    instruction_table[0x94] = CREATE_INSTR_ADDR(STY, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0x95] = CREATE_INSTR_ADDR(STA, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0x96] = CREATE_INSTR_ADDR(STX, AddressingMode::ZeroPageY, 2, 4, false);
    instruction_table[0x98] = CREATE_INSTR_IMPLIED(TYA, AddressingMode::Implied, 1, 2);
    instruction_table[0x99] = CREATE_INSTR_ADDR(STA, AddressingMode::AbsoluteY, 3, 5, false);
    instruction_table[0x9A] = CREATE_INSTR_IMPLIED(TXS, AddressingMode::Implied, 1, 2);
    instruction_table[0x9D] = CREATE_INSTR_ADDR(STA, AddressingMode::AbsoluteX, 3, 5, false);
    instruction_table[0xA0] = CREATE_INSTR(LDY, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0xA1] = CREATE_INSTR(LDA, AddressingMode::IndexedIndirect, 2, 6, false);
    instruction_table[0xA2] = CREATE_INSTR(LDX, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0xA4] = CREATE_INSTR(LDY, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0xA5] = CREATE_INSTR(LDA, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0xA6] = CREATE_INSTR(LDX, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0xA8] = CREATE_INSTR_IMPLIED(TAY, AddressingMode::Implied, 1, 2);
    instruction_table[0xA9] = CREATE_INSTR(LDA, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0xAA] = CREATE_INSTR_IMPLIED(TAX, AddressingMode::Implied, 1, 2);
    instruction_table[0xAC] = CREATE_INSTR(LDY, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0xAD] = CREATE_INSTR(LDA, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0xAE] = CREATE_INSTR(LDX, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0xB0] = CREATE_INSTR(BCS, AddressingMode::Relative, 2, 2, true);
    instruction_table[0xB1] = CREATE_INSTR(LDA, AddressingMode::IndirectIndexed, 2, 5, true);
    instruction_table[0xB4] = CREATE_INSTR(LDY, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0xB5] = CREATE_INSTR(LDA, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0xB6] = CREATE_INSTR(LDX, AddressingMode::ZeroPageY, 2, 4, false);
    instruction_table[0xB8] = CREATE_INSTR_IMPLIED(CLV, AddressingMode::Implied, 1, 2);
    instruction_table[0xB9] = CREATE_INSTR(LDA, AddressingMode::AbsoluteY, 3, 4, true);
    instruction_table[0xBA] = CREATE_INSTR_IMPLIED(TSX, AddressingMode::Implied, 1, 2);
    instruction_table[0xBC] = CREATE_INSTR(LDY, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0xBD] = CREATE_INSTR(LDA, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0xBE] = CREATE_INSTR(LDX, AddressingMode::AbsoluteY, 3, 4, true);
    instruction_table[0xC0] = CREATE_INSTR(CPY, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0xC1] = CREATE_INSTR(CMP, AddressingMode::IndexedIndirect, 2, 6, false);
    instruction_table[0xC4] = CREATE_INSTR(CPY, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0xC5] = CREATE_INSTR(CMP, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0xC6] = CREATE_INSTR_ADDR(DEC, AddressingMode::ZeroPage, 2, 5, false);
    instruction_table[0xC8] = CREATE_INSTR_IMPLIED(INY, AddressingMode::Implied, 1, 2);
    instruction_table[0xC9] = CREATE_INSTR(CMP, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0xCA] = CREATE_INSTR_IMPLIED(DEX, AddressingMode::Implied, 1, 2);
    instruction_table[0xCC] = CREATE_INSTR(CPY, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0xCD] = CREATE_INSTR(CMP, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0xCE] = CREATE_INSTR_ADDR(DEC, AddressingMode::Absolute, 3, 6, false);
    instruction_table[0xD0] = CREATE_INSTR(BNE, AddressingMode::Relative, 2, 2, true);
    instruction_table[0xD1] = CREATE_INSTR(CMP, AddressingMode::IndirectIndexed, 2, 5, true);
    instruction_table[0xD5] = CREATE_INSTR(CMP, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0xD6] = CREATE_INSTR_ADDR(DEC, AddressingMode::ZeroPageX, 2, 6, false);
    instruction_table[0xD8] = CREATE_INSTR_IMPLIED(CLD, AddressingMode::Implied, 1, 2);
    instruction_table[0xD9] = CREATE_INSTR(CMP, AddressingMode::AbsoluteY, 3, 4, true);
    instruction_table[0xDD] = CREATE_INSTR(CMP, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0xDE] = CREATE_INSTR_ADDR(DEC, AddressingMode::AbsoluteX, 3, 7, false);
    instruction_table[0xE0] = CREATE_INSTR(CPX, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0xE1] = CREATE_INSTR(SBC, AddressingMode::IndexedIndirect, 2, 6, false);
    instruction_table[0xE4] = CREATE_INSTR(CPX, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0xE5] = CREATE_INSTR(SBC, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0xE6] = CREATE_INSTR_ADDR(INC, AddressingMode::ZeroPage, 2, 5, false);
    instruction_table[0xE8] = CREATE_INSTR_IMPLIED(INX, AddressingMode::Implied, 1, 2);
    instruction_table[0xE9] = CREATE_INSTR(SBC, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0xEA] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2);
    instruction_table[0xEC] = CREATE_INSTR(CPX, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0xED] = CREATE_INSTR(SBC, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0xEE] = CREATE_INSTR_ADDR(INC, AddressingMode::Absolute, 3, 6, false);
    instruction_table[0xF0] = CREATE_INSTR(BEQ, AddressingMode::Relative, 2, 2, true);
    instruction_table[0xF1] = CREATE_INSTR(SBC, AddressingMode::IndirectIndexed, 2, 5, true);
    instruction_table[0xF5] = CREATE_INSTR(SBC, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0xF6] = CREATE_INSTR_ADDR(INC, AddressingMode::ZeroPageX, 2, 6, false);
    instruction_table[0xF8] = CREATE_INSTR_IMPLIED(SED, AddressingMode::Implied, 1, 2);
    instruction_table[0xF9] = CREATE_INSTR(SBC, AddressingMode::AbsoluteY, 3, 4, true);
    instruction_table[0xFD] = CREATE_INSTR(SBC, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0xFE] = CREATE_INSTR_ADDR(INC, AddressingMode::AbsoluteX, 3, 7, false);

    // Unofficial Opcodes
    instruction_table[0x02] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2); // KIL
    instruction_table[0x12] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2); // KIL
    instruction_table[0x22] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2); // KIL
    instruction_table[0x32] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2); // KIL
    instruction_table[0x42] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2); // KIL
    instruction_table[0x52] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2); // KIL
    instruction_table[0x62] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2); // KIL
    instruction_table[0x72] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2); // KIL
    instruction_table[0x92] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2); // KIL
    instruction_table[0xB2] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2); // KIL
    instruction_table[0xD2] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2); // KIL
    instruction_table[0xF2] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2); // KIL
    instruction_table[0x1A] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2);
    instruction_table[0x3A] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2);
    instruction_table[0x5A] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2);
    instruction_table[0x7A] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2);
    instruction_table[0xDA] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2);
    instruction_table[0xFA] = CREATE_INSTR_IMPLIED(NOP, AddressingMode::Implied, 1, 2);
    instruction_table[0x80] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0x82] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0x89] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0xC2] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0xE2] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0x04] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0x44] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0x64] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0x14] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0x34] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0x54] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0x74] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0xD4] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0xF4] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::ZeroPageX, 2, 4, false);
    instruction_table[0x0C] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0x1C] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0x3C] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0x5C] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0x7C] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0xDC] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0xFC] = CREATE_INSTR_ADDR(NOP_unofficial, AddressingMode::AbsoluteX, 3, 4, true);
    instruction_table[0xA3] = CREATE_INSTR(LAX, AddressingMode::IndexedIndirect, 2, 6, false);
    instruction_table[0xA7] = CREATE_INSTR(LAX, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0xAF] = CREATE_INSTR(LAX, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0xB3] = CREATE_INSTR(LAX, AddressingMode::IndirectIndexed, 2, 5, true);
    instruction_table[0xB7] = CREATE_INSTR(LAX, AddressingMode::ZeroPageY, 2, 4, false);
    instruction_table[0xBF] = CREATE_INSTR(LAX, AddressingMode::AbsoluteY, 3, 4, true);
    instruction_table[0x83] = CREATE_INSTR_ADDR(SAX, AddressingMode::IndexedIndirect, 2, 6, false);
    instruction_table[0x87] = CREATE_INSTR_ADDR(SAX, AddressingMode::ZeroPage, 2, 3, false);
    instruction_table[0x8F] = CREATE_INSTR_ADDR(SAX, AddressingMode::Absolute, 3, 4, false);
    instruction_table[0x97] = CREATE_INSTR_ADDR(SAX, AddressingMode::ZeroPageY, 2, 4, false);
    instruction_table[0xEB] = CREATE_INSTR(SBC, AddressingMode::Immediate, 2, 2, false);
    instruction_table[0xC3] = CREATE_INSTR_ADDR(DCP, AddressingMode::IndexedIndirect, 2, 8, false);
    instruction_table[0xC7] = CREATE_INSTR_ADDR(DCP, AddressingMode::ZeroPage, 2, 5, false);
    instruction_table[0xCF] = CREATE_INSTR_ADDR(DCP, AddressingMode::Absolute, 3, 6, false);
    instruction_table[0xD3] = CREATE_INSTR_ADDR(DCP, AddressingMode::IndirectIndexed, 2, 8, false);
    instruction_table[0xD7] = CREATE_INSTR_ADDR(DCP, AddressingMode::ZeroPageX, 2, 6, false);
    instruction_table[0xDB] = CREATE_INSTR_ADDR(DCP, AddressingMode::AbsoluteY, 3, 7, false);
    instruction_table[0xDF] = CREATE_INSTR_ADDR(DCP, AddressingMode::AbsoluteX, 3, 7, false);
    instruction_table[0xE3] = CREATE_INSTR_ADDR(ISC, AddressingMode::IndexedIndirect, 2, 8, false);
    instruction_table[0xE7] = CREATE_INSTR_ADDR(ISC, AddressingMode::ZeroPage, 2, 5, false);
    instruction_table[0xEF] = CREATE_INSTR_ADDR(ISC, AddressingMode::Absolute, 3, 6, false);
    instruction_table[0xF3] = CREATE_INSTR_ADDR(ISC, AddressingMode::IndirectIndexed, 2, 8, false);
    instruction_table[0xF7] = CREATE_INSTR_ADDR(ISC, AddressingMode::ZeroPageX, 2, 6, false);
    instruction_table[0xFB] = CREATE_INSTR_ADDR(ISC, AddressingMode::AbsoluteY, 3, 7, false);
    instruction_table[0xFF] = CREATE_INSTR_ADDR(ISC, AddressingMode::AbsoluteX, 3, 7, false);
    instruction_table[0x03] = CREATE_INSTR_ADDR(SLO, AddressingMode::IndexedIndirect, 2, 8, false);
    instruction_table[0x07] = CREATE_INSTR_ADDR(SLO, AddressingMode::ZeroPage, 2, 5, false);
    instruction_table[0x0F] = CREATE_INSTR_ADDR(SLO, AddressingMode::Absolute, 3, 6, false);
    instruction_table[0x13] = CREATE_INSTR_ADDR(SLO, AddressingMode::IndirectIndexed, 2, 8, false);
    instruction_table[0x17] = CREATE_INSTR_ADDR(SLO, AddressingMode::ZeroPageX, 2, 6, false);
    instruction_table[0x1B] = CREATE_INSTR_ADDR(SLO, AddressingMode::AbsoluteY, 3, 7, false);
    instruction_table[0x1F] = CREATE_INSTR_ADDR(SLO, AddressingMode::AbsoluteX, 3, 7, false);
    instruction_table[0x23] = CREATE_INSTR_ADDR(RLA, AddressingMode::IndexedIndirect, 2, 8, false);
    instruction_table[0x27] = CREATE_INSTR_ADDR(RLA, AddressingMode::ZeroPage, 2, 5, false);
    instruction_table[0x2F] = CREATE_INSTR_ADDR(RLA, AddressingMode::Absolute, 3, 6, false);
    instruction_table[0x33] = CREATE_INSTR_ADDR(RLA, AddressingMode::IndirectIndexed, 2, 8, false);
    instruction_table[0x37] = CREATE_INSTR_ADDR(RLA, AddressingMode::ZeroPageX, 2, 6, false);
    instruction_table[0x3B] = CREATE_INSTR_ADDR(RLA, AddressingMode::AbsoluteY, 3, 7, false);
    instruction_table[0x3F] = CREATE_INSTR_ADDR(RLA, AddressingMode::AbsoluteX, 3, 7, false);
    instruction_table[0x43] = CREATE_INSTR_ADDR(SRE, AddressingMode::IndexedIndirect, 2, 8, false);
    instruction_table[0x47] = CREATE_INSTR_ADDR(SRE, AddressingMode::ZeroPage, 2, 5, false);
    instruction_table[0x4F] = CREATE_INSTR_ADDR(SRE, AddressingMode::Absolute, 3, 6, false);
    instruction_table[0x53] = CREATE_INSTR_ADDR(SRE, AddressingMode::IndirectIndexed, 2, 8, false);
    instruction_table[0x57] = CREATE_INSTR_ADDR(SRE, AddressingMode::ZeroPageX, 2, 6, false);
    instruction_table[0x5B] = CREATE_INSTR_ADDR(SRE, AddressingMode::AbsoluteY, 3, 7, false);
    instruction_table[0x5F] = CREATE_INSTR_ADDR(SRE, AddressingMode::AbsoluteX, 3, 7, false);
    instruction_table[0x63] = CREATE_INSTR_ADDR(RRA, AddressingMode::IndexedIndirect, 2, 8, false);
    instruction_table[0x67] = CREATE_INSTR_ADDR(RRA, AddressingMode::ZeroPage, 2, 5, false);
    instruction_table[0x6F] = CREATE_INSTR_ADDR(RRA, AddressingMode::Absolute, 3, 6, false);
    instruction_table[0x73] = CREATE_INSTR_ADDR(RRA, AddressingMode::IndirectIndexed, 2, 8, false);
    instruction_table[0x77] = CREATE_INSTR_ADDR(RRA, AddressingMode::ZeroPageX, 2, 6, false);
    instruction_table[0x7B] = CREATE_INSTR_ADDR(RRA, AddressingMode::AbsoluteY, 3, 7, false);
    instruction_table[0x7F] = CREATE_INSTR_ADDR(RRA, AddressingMode::AbsoluteX, 3, 7, false);
}

void Cpu::setNegativeFlag(bool value) { registers.p = value ? (registers.p | 0x80) : (registers.p & ~0x80); }
void Cpu::setZeroFlag(bool value) { registers.p = value ? (registers.p | 0x02) : (registers.p & ~0x02); }
void Cpu::setCarryFlag(bool value) { registers.p = value ? (registers.p | 0x01) : (registers.p & ~0x01); }
void Cpu::setOverflowFlag(bool value) { registers.p = value ? (registers.p | 0x40) : (registers.p & ~0x40); }
void Cpu::setDecimalFlag(bool value) { registers.p = value ? (registers.p | 0x08) : (registers.p & ~0x08); }
void Cpu::setInterruptDisableFlag(bool value) { registers.p = value ? (registers.p | 0x04) : (registers.p & ~0x04); }

void Cpu::writeMemory(uint16_t address, uint8_t value) { memory.bus[address] = value; }
uint8_t Cpu::readMemory(uint16_t address) { return memory.bus[address]; }

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
        cpu.total_cycles++;
        uint16_t pc_before_branch = cpu.registers.pc;
        uint16_t next_instr_addr = pc_before_branch + 2;
        int8_t offset = static_cast<int8_t>(offset_val);
        uint16_t target_address = next_instr_addr + offset;

        if ((next_instr_addr & 0xFF00) != (target_address & 0xFF00)) {
            cpu.total_cycles++;
        }
        cpu.registers.pc = target_address;
    } else {
        // Dacă branch-ul nu este luat, PC-ul va fi incrementat cu 2 (bytes)
        // de bucla principală `executeInstruction`. Nu facem nimic special aici.
        cpu.registers.pc += 2;
    }
}

void Cpu::BCC(uint8_t value) { branch_if(*this, !(registers.p & 0x01), value); }
void Cpu::BCS(uint8_t value) { branch_if(*this, registers.p & 0x01, value); }
void Cpu::BEQ(uint8_t value) { branch_if(*this, registers.p & 0x02, value); }
void Cpu::BNE(uint8_t value) { branch_if(*this, !(registers.p & 0x02), value); }
void Cpu::BMI(uint8_t value) { branch_if(*this, registers.p & 0x80, value); }
void Cpu::BPL(uint8_t value) { branch_if(*this, !(registers.p & 0x80), value); }
void Cpu::BVC(uint8_t value) { branch_if(*this, !(registers.p & 0x40), value); }
void Cpu::BVS(uint8_t value) { branch_if(*this, registers.p & 0x40, value); }

void Cpu::BIT(uint8_t value) {
    setZeroFlag((registers.a & value) == 0);
    setOverflowFlag(value & 0x40);
    setNegativeFlag(value & 0x80);
}

void Cpu::BRK() {
    registers.pc++;
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

// Unofficial Instructions
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

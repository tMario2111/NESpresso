// NESpresso CPU Implementation
// Date: 2025-07-29 23:45:02 UTC
// User: nicusor43

#include "Cpu.hpp"
#include "Memory.hpp"
#include <iostream>

Cpu::Cpu() { this->initInstructionTable(); }

Cpu &Cpu::instance() {
    static Cpu cpu;
    return cpu;
}

// --- Bucla Principală de Execuție (LOGICĂ NOUĂ) ---
void Cpu::executeInstruction() {
    uint8_t opcode = readMemory(registers.pc);
    Instruction &instruction = instruction_table[opcode];
    bool page_crossed = false;

    // Calculează adresa/valoarea
    uint16_t address = 0;
    uint8_t value = 0;

    switch (instruction.mode) {
        case AddressingMode::Implied:
            break; // Nu se face nimic
        case AddressingMode::Immediate:
            value = readMemory(registers.pc + 1);
            break;
        case AddressingMode::ZeroPage:
            address = readMemory(registers.pc + 1);
            break;
        case AddressingMode::ZeroPageX:
            address = (readMemory(registers.pc + 1) + registers.x) & 0xFF;
            break;
        case AddressingMode::ZeroPageY:
            address = (readMemory(registers.pc + 1) + registers.y) & 0xFF;
            break;
        case AddressingMode::Absolute:
            address = readMemory(registers.pc + 1) | (readMemory(registers.pc + 2) << 8);
            break;
        case AddressingMode::AbsoluteX: {
            uint16_t base_addr = readMemory(registers.pc + 1) | (readMemory(registers.pc + 2) << 8);
            address = base_addr + registers.x;
            page_crossed = (base_addr & 0xFF00) != (address & 0xFF00);
            break;
        }
        case AddressingMode::AbsoluteY: {
            uint16_t base_addr = readMemory(registers.pc + 1) | (readMemory(registers.pc + 2) << 8);
            address = base_addr + registers.y;
            page_crossed = (base_addr & 0xFF00) != (address & 0xFF00);
            break;
        }
        case AddressingMode::Indirect: {
            uint16_t ptr = readMemory(registers.pc + 1) | (readMemory(registers.pc + 2) << 8);
            if ((ptr & 0x00FF) == 0x00FF) {
                // Emulează bug-ul hardware
                address = (readMemory(ptr & 0xFF00) << 8) | readMemory(ptr);
            } else {
                address = readMemory(ptr) | (readMemory(ptr + 1) << 8);
            }
            break;
        }
        case AddressingMode::IndexedIndirect: {
            uint8_t zp_addr = (readMemory(registers.pc + 1) + registers.x) & 0xFF;
            address = readMemory(zp_addr) | (readMemory((zp_addr + 1) & 0xFF) << 8);
            break;
        }
        case AddressingMode::IndirectIndexed: {
            uint8_t zp_addr = readMemory(registers.pc + 1);
            uint16_t base_addr = readMemory(zp_addr) | (readMemory((zp_addr + 1) & 0xFF) << 8);
            address = base_addr + registers.y;
            page_crossed = (base_addr & 0xFF00) != (address & 0xFF00);
            break;
        }
        case AddressingMode::Relative:
            value = readMemory(registers.pc + 1); // Offset-ul este tratat ca 'value'
            break;
    }

    // Execută funcția corespunzătoare
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

    // Adaugă ciclurile de bază + penalizarea pentru page cross
    total_cycles += instruction.cycles;
    if (page_crossed && instruction.page_crossed) {
        total_cycles++;
    }
}

// --- Tabelul de Instrucțiuni (complet) ---
void Cpu::initInstructionTable() {
    // ... (Conținutul acestei funcții rămâne identic cu cel din răspunsul meu anterior)
    // Official Opcodes
    instruction_table[0x00] = {std::function<void()>([this]() { BRK(); }), AddressingMode::Implied, 1, 7};
    instruction_table[0x01] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ORA(v); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0x05] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ORA(v); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x06] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ASL_Memory(a); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0x08] = {std::function<void()>([this]() { PHP(); }), AddressingMode::Implied, 1, 3};
    instruction_table[0x09] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ORA(v); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0x0A] = {std::function<void()>([this]() { ASL_Accumulator(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x0D] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ORA(v); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x0E] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ASL_Memory(a); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0x10] = {
        std::function<void(uint8_t)>([this](uint8_t v) { BPL(v); }), AddressingMode::Relative, 2, 2, true
    };
    instruction_table[0x11] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ORA(v); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0x15] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ORA(v); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x16] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ASL_Memory(a); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0x18] = {std::function<void()>([this]() { CLC(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x19] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ORA(v); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0x1D] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ORA(v); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0x1E] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ASL_Memory(a); }), AddressingMode::AbsoluteX, 3, 7
    };
    instruction_table[0x20] = {
        std::function<void(uint16_t)>([this](uint16_t a) { JSR(a); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0x21] = {
        std::function<void(uint8_t)>([this](uint8_t v) { AND(v); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0x24] = {
        std::function<void(uint8_t)>([this](uint8_t v) { BIT(v); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x25] = {
        std::function<void(uint8_t)>([this](uint8_t v) { AND(v); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x26] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ROL_Memory(a); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0x28] = {std::function<void()>([this]() { PLP(); }), AddressingMode::Implied, 1, 4};
    instruction_table[0x29] = {
        std::function<void(uint8_t)>([this](uint8_t v) { AND(v); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0x2A] = {std::function<void()>([this]() { ROL_Accumulator(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x2C] = {
        std::function<void(uint8_t)>([this](uint8_t v) { BIT(v); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x2D] = {
        std::function<void(uint8_t)>([this](uint8_t v) { AND(v); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x2E] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ROL_Memory(a); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0x30] = {
        std::function<void(uint8_t)>([this](uint8_t v) { BMI(v); }), AddressingMode::Relative, 2, 2, true
    };
    instruction_table[0x31] = {
        std::function<void(uint8_t)>([this](uint8_t v) { AND(v); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0x35] = {
        std::function<void(uint8_t)>([this](uint8_t v) { AND(v); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x36] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ROL_Memory(a); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0x38] = {std::function<void()>([this]() { SEC(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x39] = {
        std::function<void(uint8_t)>([this](uint8_t v) { AND(v); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0x3D] = {
        std::function<void(uint8_t)>([this](uint8_t v) { AND(v); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0x3E] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ROL_Memory(a); }), AddressingMode::AbsoluteX, 3, 7
    };
    instruction_table[0x40] = {std::function<void()>([this]() { RTI(); }), AddressingMode::Implied, 1, 6};
    instruction_table[0x41] = {
        std::function<void(uint8_t)>([this](uint8_t v) { EOR(v); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0x45] = {
        std::function<void(uint8_t)>([this](uint8_t v) { EOR(v); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x46] = {
        std::function<void(uint16_t)>([this](uint16_t a) { LSR_Memory(a); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0x48] = {std::function<void()>([this]() { PHA(); }), AddressingMode::Implied, 1, 3};
    instruction_table[0x49] = {
        std::function<void(uint8_t)>([this](uint8_t v) { EOR(v); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0x4A] = {std::function<void()>([this]() { LSR_Accumulator(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x4C] = {
        std::function<void(uint16_t)>([this](uint16_t a) { JMP(a); }), AddressingMode::Absolute, 3, 3
    };
    instruction_table[0x4D] = {
        std::function<void(uint8_t)>([this](uint8_t v) { EOR(v); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x4E] = {
        std::function<void(uint16_t)>([this](uint16_t a) { LSR_Memory(a); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0x50] = {
        std::function<void(uint8_t)>([this](uint8_t v) { BVC(v); }), AddressingMode::Relative, 2, 2, true
    };
    instruction_table[0x51] = {
        std::function<void(uint8_t)>([this](uint8_t v) { EOR(v); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0x55] = {
        std::function<void(uint8_t)>([this](uint8_t v) { EOR(v); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x56] = {
        std::function<void(uint16_t)>([this](uint16_t a) { LSR_Memory(a); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0x58] = {std::function<void()>([this]() { CLI(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x59] = {
        std::function<void(uint8_t)>([this](uint8_t v) { EOR(v); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0x5D] = {
        std::function<void(uint8_t)>([this](uint8_t v) { EOR(v); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0x5E] = {
        std::function<void(uint16_t)>([this](uint16_t a) { LSR_Memory(a); }), AddressingMode::AbsoluteX, 3, 7
    };
    instruction_table[0x60] = {std::function<void()>([this]() { RTS(); }), AddressingMode::Implied, 1, 6};
    instruction_table[0x61] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ADC(v); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0x65] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ADC(v); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x66] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ROR_Memory(a); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0x68] = {std::function<void()>([this]() { PLA(); }), AddressingMode::Implied, 1, 4};
    instruction_table[0x69] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ADC(v); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0x6A] = {std::function<void()>([this]() { ROR_Accumulator(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x6C] = {
        std::function<void(uint16_t)>([this](uint16_t a) { JMP(a); }), AddressingMode::Indirect, 3, 5
    };
    instruction_table[0x6D] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ADC(v); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x6E] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ROR_Memory(a); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0x70] = {
        std::function<void(uint8_t)>([this](uint8_t v) { BVS(v); }), AddressingMode::Relative, 2, 2, true
    };
    instruction_table[0x71] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ADC(v); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0x75] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ADC(v); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x76] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ROR_Memory(a); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0x78] = {std::function<void()>([this]() { SEI(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x79] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ADC(v); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0x7D] = {
        std::function<void(uint8_t)>([this](uint8_t v) { ADC(v); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0x7E] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ROR_Memory(a); }), AddressingMode::AbsoluteX, 3, 7
    };
    instruction_table[0x81] = {
        std::function<void(uint16_t)>([this](uint16_t a) { STA(a); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0x84] = {
        std::function<void(uint16_t)>([this](uint16_t a) { STY(a); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x85] = {
        std::function<void(uint16_t)>([this](uint16_t a) { STA(a); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x86] = {
        std::function<void(uint16_t)>([this](uint16_t a) { STX(a); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x88] = {std::function<void()>([this]() { DEY(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x8A] = {std::function<void()>([this]() { TXA(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x8C] = {
        std::function<void(uint16_t)>([this](uint16_t a) { STY(a); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x8D] = {
        std::function<void(uint16_t)>([this](uint16_t a) { STA(a); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x8E] = {
        std::function<void(uint16_t)>([this](uint16_t a) { STX(a); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x90] = {
        std::function<void(uint8_t)>([this](uint8_t v) { BCC(v); }), AddressingMode::Relative, 2, 2, true
    };
    instruction_table[0x91] = {
        std::function<void(uint16_t)>([this](uint16_t a) { STA(a); }), AddressingMode::IndirectIndexed, 2, 6
    };
    instruction_table[0x94] = {
        std::function<void(uint16_t)>([this](uint16_t a) { STY(a); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x95] = {
        std::function<void(uint16_t)>([this](uint16_t a) { STA(a); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x96] = {
        std::function<void(uint16_t)>([this](uint16_t a) { STX(a); }), AddressingMode::ZeroPageY, 2, 4
    };
    instruction_table[0x98] = {std::function<void()>([this]() { TYA(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x99] = {
        std::function<void(uint16_t)>([this](uint16_t a) { STA(a); }), AddressingMode::AbsoluteY, 3, 5
    };
    instruction_table[0x9A] = {std::function<void()>([this]() { TXS(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x9D] = {
        std::function<void(uint16_t)>([this](uint16_t a) { STA(a); }), AddressingMode::AbsoluteX, 3, 5
    };
    instruction_table[0xA0] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDY(v); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xA1] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDA(v); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0xA2] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDX(v); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xA4] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDY(v); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xA5] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDA(v); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xA6] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDX(v); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xA8] = {std::function<void()>([this]() { TAY(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xA9] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDA(v); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xAA] = {std::function<void()>([this]() { TAX(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xAC] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDY(v); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xAD] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDA(v); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xAE] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDX(v); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xB0] = {
        std::function<void(uint8_t)>([this](uint8_t v) { BCS(v); }), AddressingMode::Relative, 2, 2, true
    };
    instruction_table[0xB1] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDA(v); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0xB4] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDY(v); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0xB5] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDA(v); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0xB6] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDX(v); }), AddressingMode::ZeroPageY, 2, 4
    };
    instruction_table[0xB8] = {std::function<void()>([this]() { CLV(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xB9] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDA(v); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0xBA] = {std::function<void()>([this]() { TSX(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xBC] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDY(v); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0xBD] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDA(v); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0xBE] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LDX(v); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0xC0] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CPY(v); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xC1] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CMP(v); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0xC4] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CPY(v); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xC5] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CMP(v); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xC6] = {
        std::function<void(uint16_t)>([this](uint16_t a) { DEC(a); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0xC8] = {std::function<void()>([this]() { INY(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xC9] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CMP(v); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xCA] = {std::function<void()>([this]() { DEX(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xCC] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CPY(v); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xCD] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CMP(v); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xCE] = {
        std::function<void(uint16_t)>([this](uint16_t a) { DEC(a); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0xD0] = {
        std::function<void(uint8_t)>([this](uint8_t v) { BNE(v); }), AddressingMode::Relative, 2, 2, true
    };
    instruction_table[0xD1] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CMP(v); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0xD5] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CMP(v); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0xD6] = {
        std::function<void(uint16_t)>([this](uint16_t a) { DEC(a); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0xD8] = {std::function<void()>([this]() { CLD(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xD9] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CMP(v); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0xDD] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CMP(v); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0xDE] = {
        std::function<void(uint16_t)>([this](uint16_t a) { DEC(a); }), AddressingMode::AbsoluteX, 3, 7
    };
    instruction_table[0xE0] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CPX(v); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xE1] = {
        std::function<void(uint8_t)>([this](uint8_t v) { SBC(v); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0xE4] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CPX(v); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xE5] = {
        std::function<void(uint8_t)>([this](uint8_t v) { SBC(v); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xE6] = {
        std::function<void(uint16_t)>([this](uint16_t a) { INC(a); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0xE8] = {std::function<void()>([this]() { INX(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xE9] = {
        std::function<void(uint8_t)>([this](uint8_t v) { SBC(v); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xEA] = {std::function<void()>([this]() { NOP(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xEC] = {
        std::function<void(uint8_t)>([this](uint8_t v) { CPX(v); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xED] = {
        std::function<void(uint8_t)>([this](uint8_t v) { SBC(v); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xEE] = {
        std::function<void(uint16_t)>([this](uint16_t a) { INC(a); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0xF0] = {
        std::function<void(uint8_t)>([this](uint8_t v) { BEQ(v); }), AddressingMode::Relative, 2, 2, true
    };
    instruction_table[0xF1] = {
        std::function<void(uint8_t)>([this](uint8_t v) { SBC(v); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0xF5] = {
        std::function<void(uint8_t)>([this](uint8_t v) { SBC(v); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0xF6] = {
        std::function<void(uint16_t)>([this](uint16_t a) { INC(a); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0xF8] = {std::function<void()>([this]() { SED(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xF9] = {
        std::function<void(uint8_t)>([this](uint8_t v) { SBC(v); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0xFD] = {
        std::function<void(uint8_t)>([this](uint8_t v) { SBC(v); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0xFE] = {
        std::function<void(uint16_t)>([this](uint16_t a) { INC(a); }), AddressingMode::AbsoluteX, 3, 7
    };

    // Unofficial Opcodes
    instruction_table[0x02] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    }; // KIL
    instruction_table[0x12] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    }; // KIL
    instruction_table[0x22] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    }; // KIL
    instruction_table[0x32] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    }; // KIL
    instruction_table[0x42] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    }; // KIL
    instruction_table[0x52] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    }; // KIL
    instruction_table[0x62] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    }; // KIL
    instruction_table[0x72] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    }; // KIL
    instruction_table[0x92] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    }; // KIL
    instruction_table[0xB2] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    }; // KIL
    instruction_table[0xD2] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    }; // KIL
    instruction_table[0xF2] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    }; // KIL

    instruction_table[0x1A] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    };
    instruction_table[0x3A] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    };
    instruction_table[0x5A] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    };
    instruction_table[0x7A] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    };
    instruction_table[0xDA] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    };
    instruction_table[0xFA] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Implied, 1, 2
    };

    instruction_table[0x80] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0x82] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0x89] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xC2] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xE2] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Immediate, 2, 2
    };

    instruction_table[0x04] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x44] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x64] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x14] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x34] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x54] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x74] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0xD4] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0xF4] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::ZeroPageX, 2, 4
    };

    instruction_table[0x0C] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x1C] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0x3C] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0x5C] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0x7C] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0xDC] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0xFC] = {
        std::function<void(uint16_t)>([this](uint16_t a) { NOP_unofficial(a); }), AddressingMode::AbsoluteX, 3, 4, true
    };

    // LAX
    instruction_table[0xA3] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LAX(v); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0xA7] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LAX(v); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xAF] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LAX(v); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xB3] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LAX(v); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0xB7] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LAX(v); }), AddressingMode::ZeroPageY, 2, 4
    };
    instruction_table[0xBF] = {
        std::function<void(uint8_t)>([this](uint8_t v) { LAX(v); }), AddressingMode::AbsoluteY, 3, 4, true
    };

    // SAX
    instruction_table[0x83] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SAX(a); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0x87] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SAX(a); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x8F] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SAX(a); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x97] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SAX(a); }), AddressingMode::ZeroPageY, 2, 4
    };

    // SBC (unofficial)
    instruction_table[0xEB] = {
        std::function<void(uint8_t)>([this](uint8_t v) { SBC(v); }), AddressingMode::Immediate, 2, 2
    };

    // DCP
    instruction_table[0xC3] = {
        std::function<void(uint16_t)>([this](uint16_t a) { DCP(a); }), AddressingMode::IndexedIndirect, 2, 8
    };
    instruction_table[0xC7] = {
        std::function<void(uint16_t)>([this](uint16_t a) { DCP(a); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0xCF] = {
        std::function<void(uint16_t)>([this](uint16_t a) { DCP(a); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0xD3] = {
        std::function<void(uint16_t)>([this](uint16_t a) { DCP(a); }), AddressingMode::IndirectIndexed, 2, 8
    };
    instruction_table[0xD7] = {
        std::function<void(uint16_t)>([this](uint16_t a) { DCP(a); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0xDB] = {
        std::function<void(uint16_t)>([this](uint16_t a) { DCP(a); }), AddressingMode::AbsoluteY, 3, 7
    };
    instruction_table[0xDF] = {
        std::function<void(uint16_t)>([this](uint16_t a) { DCP(a); }), AddressingMode::AbsoluteX, 3, 7
    };

    // ISC
    instruction_table[0xE3] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ISC(a); }), AddressingMode::IndexedIndirect, 2, 8
    };
    instruction_table[0xE7] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ISC(a); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0xEF] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ISC(a); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0xF3] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ISC(a); }), AddressingMode::IndirectIndexed, 2, 8
    };
    instruction_table[0xF7] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ISC(a); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0xFB] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ISC(a); }), AddressingMode::AbsoluteY, 3, 7
    };
    instruction_table[0xFF] = {
        std::function<void(uint16_t)>([this](uint16_t a) { ISC(a); }), AddressingMode::AbsoluteX, 3, 7
    };

    // SLO
    instruction_table[0x03] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SLO(a); }), AddressingMode::IndexedIndirect, 2, 8
    };
    instruction_table[0x07] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SLO(a); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0x0F] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SLO(a); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0x13] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SLO(a); }), AddressingMode::IndirectIndexed, 2, 8
    };
    instruction_table[0x17] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SLO(a); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0x1B] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SLO(a); }), AddressingMode::AbsoluteY, 3, 7
    };
    instruction_table[0x1F] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SLO(a); }), AddressingMode::AbsoluteX, 3, 7
    };

    // RLA
    instruction_table[0x23] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RLA(a); }), AddressingMode::IndexedIndirect, 2, 8
    };
    instruction_table[0x27] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RLA(a); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0x2F] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RLA(a); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0x33] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RLA(a); }), AddressingMode::IndirectIndexed, 2, 8
    };
    instruction_table[0x37] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RLA(a); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0x3B] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RLA(a); }), AddressingMode::AbsoluteY, 3, 7
    };
    instruction_table[0x3F] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RLA(a); }), AddressingMode::AbsoluteX, 3, 7
    };

    // SRE
    instruction_table[0x43] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SRE(a); }), AddressingMode::IndexedIndirect, 2, 8
    };
    instruction_table[0x47] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SRE(a); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0x4F] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SRE(a); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0x53] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SRE(a); }), AddressingMode::IndirectIndexed, 2, 8
    };
    instruction_table[0x57] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SRE(a); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0x5B] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SRE(a); }), AddressingMode::AbsoluteY, 3, 7
    };
    instruction_table[0x5F] = {
        std::function<void(uint16_t)>([this](uint16_t a) { SRE(a); }), AddressingMode::AbsoluteX, 3, 7
    };

    // RRA
    instruction_table[0x63] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RRA(a); }), AddressingMode::IndexedIndirect, 2, 8
    };
    instruction_table[0x67] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RRA(a); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0x6F] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RRA(a); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0x73] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RRA(a); }), AddressingMode::IndirectIndexed, 2, 8
    };
    instruction_table[0x77] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RRA(a); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0x7B] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RRA(a); }), AddressingMode::AbsoluteY, 3, 7
    };
    instruction_table[0x7F] = {
        std::function<void(uint16_t)>([this](uint16_t a) { RRA(a); }), AddressingMode::AbsoluteX, 3, 7
    };
}

// --- Implementări Instrucțiuni (LOGICĂ NOUĂ PENTRU BRANCH ȘI JUMPS) ---
// Flag Setters
void Cpu::setNegativeFlag(bool value) { registers.p = value ? (registers.p | 0x80) : (registers.p & ~0x80); }
void Cpu::setZeroFlag(bool value) { registers.p = value ? (registers.p | 0x02) : (registers.p & ~0x02); }
void Cpu::setCarryFlag(bool value) { registers.p = value ? (registers.p | 0x01) : (registers.p & ~0x01); }
void Cpu::setOverflowFlag(bool value) { registers.p = value ? (registers.p | 0x40) : (registers.p & ~0x40); }
void Cpu::setDecimalFlag(bool value) { registers.p = value ? (registers.p | 0x08) : (registers.p & ~0x08); }
void Cpu::setInterruptDisableFlag(bool value) { registers.p = value ? (registers.p | 0x04) : (registers.p & ~0x04); }

// Memory Access
void Cpu::writeMemory(uint16_t address, uint8_t value) { memory.bus[address] = value; }
uint8_t Cpu::readMemory(uint16_t address) { return memory.bus[address]; }

// Official Instructions
void Cpu::ADC(uint8_t value) {
    uint16_t sum = registers.a + value + (registers.p & 0x01);
    setCarryFlag(sum > 0xFF);
    setZeroFlag((sum & 0xFF) == 0);
    setOverflowFlag(~(registers.a ^ value) & (registers.a ^ sum) & 0x80);
    setNegativeFlag(sum & 0x80);
    registers.a = sum & 0xFF;
    registers.pc += instruction_table[0x69].bytes; // Generic bytes, ADC Immediate
}

//...
void Cpu::AND(uint8_t value) {
    registers.a &= value;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
    registers.pc += instruction_table[0x29].bytes;
}

void Cpu::ASL_Accumulator() {
    setCarryFlag(registers.a & 0x80);
    registers.a <<= 1;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
    registers.pc += 1;
}

void Cpu::ASL_Memory(uint16_t address) {
    uint8_t value = readMemory(address);
    setCarryFlag(value & 0x80);
    value <<= 1;
    writeMemory(address, value);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
    registers.pc += instruction_table[0x06].bytes;
}

void branch_if(Cpu &cpu, bool condition, uint8_t offset) {
    cpu.registers.pc += 2; // Trecem peste instrucțiunea de branch
    if (condition) {
        cpu.total_cycles++;
        uint16_t old_pc = cpu.registers.pc;
        cpu.registers.pc += static_cast<int8_t>(offset);
        if ((old_pc & 0xFF00) != (cpu.registers.pc & 0xFF00)) {
            cpu.total_cycles++;
        }
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
    registers.pc += instruction_table[0x24].bytes;
}

void Cpu::BRK() {
    registers.pc++;
    memory.push16(registers.sp, registers.pc);
    memory.push8(registers.sp, registers.p | 0x10);
    setInterruptDisableFlag(true);
    registers.pc = readMemory(0xFFFE) | (readMemory(0xFFFF) << 8);
}

void Cpu::CLC() {
    setCarryFlag(false);
    registers.pc++;
}

void Cpu::CLD() {
    setDecimalFlag(false);
    registers.pc++;
}

void Cpu::CLI() {
    setInterruptDisableFlag(false);
    registers.pc++;
}

void Cpu::CLV() {
    setOverflowFlag(false);
    registers.pc++;
}

void Cpu::CMP(uint8_t value) {
    uint8_t result = registers.a - value;
    setCarryFlag(registers.a >= value);
    setZeroFlag(registers.a == value);
    setNegativeFlag(result & 0x80);
    registers.pc += instruction_table[0xC9].bytes;
}

void Cpu::CPX(uint8_t value) {
    uint8_t result = registers.x - value;
    setCarryFlag(registers.x >= value);
    setZeroFlag(registers.x == value);
    setNegativeFlag(result & 0x80);
    registers.pc += instruction_table[0xE0].bytes;
}

void Cpu::CPY(uint8_t value) {
    uint8_t result = registers.y - value;
    setCarryFlag(registers.y >= value);
    setZeroFlag(registers.y == value);
    setNegativeFlag(result & 0x80);
    registers.pc += instruction_table[0xC0].bytes;
}

void Cpu::DEC(uint16_t address) {
    uint8_t value = readMemory(address) - 1;
    writeMemory(address, value);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
    registers.pc += instruction_table[0xC6].bytes;
}

void Cpu::DEX() {
    registers.x--;
    setZeroFlag(registers.x == 0);
    setNegativeFlag(registers.x & 0x80);
    registers.pc++;
}

void Cpu::DEY() {
    registers.y--;
    setZeroFlag(registers.y == 0);
    setNegativeFlag(registers.y & 0x80);
    registers.pc++;
}

void Cpu::EOR(uint8_t value) {
    registers.a ^= value;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
    registers.pc += instruction_table[0x49].bytes;
}

void Cpu::INC(uint16_t address) {
    uint8_t value = readMemory(address) + 1;
    writeMemory(address, value);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
    registers.pc += instruction_table[0xE6].bytes;
}

void Cpu::INX() {
    registers.x++;
    setZeroFlag(registers.x == 0);
    setNegativeFlag(registers.x & 0x80);
    registers.pc++;
}

void Cpu::INY() {
    registers.y++;
    setZeroFlag(registers.y == 0);
    setNegativeFlag(registers.y & 0x80);
    registers.pc++;
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
    registers.pc += instruction_table[0xA9].bytes;
}

void Cpu::LDX(uint8_t value) {
    registers.x = value;
    setZeroFlag(registers.x == 0);
    setNegativeFlag(registers.x & 0x80);
    registers.pc += instruction_table[0xA2].bytes;
}

void Cpu::LDY(uint8_t value) {
    registers.y = value;
    setZeroFlag(registers.y == 0);
    setNegativeFlag(registers.y & 0x80);
    registers.pc += instruction_table[0xA0].bytes;
}

void Cpu::LSR_Accumulator() {
    setCarryFlag(registers.a & 0x01);
    registers.a >>= 1;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(false);
    registers.pc++;
}

void Cpu::LSR_Memory(uint16_t address) {
    uint8_t value = readMemory(address);
    setCarryFlag(value & 0x01);
    value >>= 1;
    writeMemory(address, value);
    setZeroFlag(value == 0);
    setNegativeFlag(false);
    registers.pc += instruction_table[0x46].bytes;
}

void Cpu::NOP() { registers.pc++; }

void Cpu::ORA(uint8_t value) {
    registers.a |= value;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
    registers.pc += instruction_table[0x09].bytes;
}

void Cpu::PHA() {
    memory.push8(registers.sp, registers.a);
    registers.pc++;
}

void Cpu::PHP() {
    memory.push8(registers.sp, registers.p | 0x30);
    registers.pc++;
}

void Cpu::PLA() {
    registers.a = memory.pop8(registers.sp);
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
    registers.pc++;
}

void Cpu::PLP() {
    registers.p = (memory.pop8(registers.sp) & ~0x10) | 0x20;
    registers.pc++;
}

void Cpu::ROL_Accumulator() {
    bool old_carry = registers.p & 0x01;
    setCarryFlag(registers.a & 0x80);
    registers.a = (registers.a << 1) | old_carry;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
    registers.pc++;
}

void Cpu::ROL_Memory(uint16_t address) {
    uint8_t value = readMemory(address);
    bool old_carry = registers.p & 0x01;
    setCarryFlag(value & 0x80);
    value = (value << 1) | old_carry;
    writeMemory(address, value);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
    registers.pc += instruction_table[0x26].bytes;
}

void Cpu::ROR_Accumulator() {
    bool old_carry = registers.p & 0x01;
    setCarryFlag(registers.a & 0x01);
    registers.a = (registers.a >> 1) | (old_carry << 7);
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
    registers.pc++;
}

void Cpu::ROR_Memory(uint16_t address) {
    uint8_t value = readMemory(address);
    bool old_carry = registers.p & 0x01;
    setCarryFlag(value & 0x01);
    value = (value >> 1) | (old_carry << 7);
    writeMemory(address, value);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
    registers.pc += instruction_table[0x66].bytes;
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
    registers.pc += instruction_table[0xE9].bytes;
}

void Cpu::SEC() {
    setCarryFlag(true);
    registers.pc++;
}

void Cpu::SED() {
    setDecimalFlag(true);
    registers.pc++;
}

void Cpu::SEI() {
    setInterruptDisableFlag(true);
    registers.pc++;
}

void Cpu::STA(uint16_t address) {
    writeMemory(address, registers.a);
    registers.pc += instruction_table[0x85].bytes;
}

void Cpu::STX(uint16_t address) {
    writeMemory(address, registers.x);
    registers.pc += instruction_table[0x86].bytes;
}

void Cpu::STY(uint16_t address) {
    writeMemory(address, registers.y);
    registers.pc += instruction_table[0x84].bytes;
}

void Cpu::TAX() {
    registers.x = registers.a;
    setZeroFlag(registers.x == 0);
    setNegativeFlag(registers.x & 0x80);
    registers.pc++;
}

void Cpu::TAY() {
    registers.y = registers.a;
    setZeroFlag(registers.y == 0);
    setNegativeFlag(registers.y & 0x80);
    registers.pc++;
}

void Cpu::TSX() {
    registers.x = registers.sp;
    setZeroFlag(registers.x == 0);
    setNegativeFlag(registers.x & 0x80);
    registers.pc++;
}

void Cpu::TXA() {
    registers.a = registers.x;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
    registers.pc++;
}

void Cpu::TXS() {
    registers.sp = registers.x;
    registers.pc++;
}

void Cpu::TYA() {
    registers.a = registers.y;
    setZeroFlag(registers.a == 0);
    setNegativeFlag(registers.a & 0x80);
    registers.pc++;
}

// --- Unofficial Instructions ---
void Cpu::NOP_unofficial(uint16_t address) {
    uint8_t opcode = readMemory(registers.pc);
    registers.pc += instruction_table[opcode].bytes;
}

void Cpu::LAX(uint8_t value) {
    LDA(value);
    LDX(value);
}

void Cpu::SAX(uint16_t address) {
    writeMemory(address, registers.a & registers.x);
    registers.pc += instruction_table[0x87].bytes;
}

void Cpu::DCP(uint16_t address) {
    INC(address);
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

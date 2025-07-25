#include "Cpu.hpp"

void Cpu::initInstructionTable()
{
    instruction_table[0x00] = {"BRK", AddressingMode::Immediate, 1, 7};
    instruction_table[0x01] = {"ORA", AddressingMode::IndexedIndirect, 2, 6};
    instruction_table[0x05] = {"ORA", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0x06] = {"ASL", AddressingMode::ZeroPage, 2, 5};
    instruction_table[0x08] = {"PHP", AddressingMode::Immediate, 1, 3};
    instruction_table[0x09] = {"ORA", AddressingMode::Immediate, 2, 2};
    instruction_table[0x0A] = {"ASL", AddressingMode::Immediate, 1, 2}; // Accumulator
    instruction_table[0x0D] = {"ORA", AddressingMode::Absolute, 3, 4};
    instruction_table[0x0E] = {"ASL", AddressingMode::Absolute, 3, 6};

    instruction_table[0x10] = {"BPL", AddressingMode::Immediate, 2, 2}; // Relative
    instruction_table[0x11] = {"ORA", AddressingMode::IndirectIndexed, 2, 5, 1};
    instruction_table[0x15] = {"ORA", AddressingMode::ZeroPageX, 2, 4};
    instruction_table[0x16] = {"ASL", AddressingMode::ZeroPageX, 2, 6};
    instruction_table[0x18] = {"CLC", AddressingMode::Immediate, 1, 2};
    instruction_table[0x19] = {"ORA", AddressingMode::AbsoluteY, 3, 4, 1};
    instruction_table[0x1D] = {"ORA", AddressingMode::AbsoluteX, 3, 4, 1};
    instruction_table[0x1E] = {"ASL", AddressingMode::AbsoluteX, 3, 7};

    instruction_table[0x20] = {"JSR", AddressingMode::Absolute, 3, 6};
    instruction_table[0x21] = {"AND", AddressingMode::IndexedIndirect, 2, 6};
    instruction_table[0x24] = {"BIT", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0x25] = {"AND", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0x26] = {"ROL", AddressingMode::ZeroPage, 2, 5};
    instruction_table[0x28] = {"PLP", AddressingMode::Immediate, 1, 4};
    instruction_table[0x29] = {"AND", AddressingMode::Immediate, 2, 2};
    instruction_table[0x2A] = {"ROL", AddressingMode::Immediate, 1, 2}; // Accumulator
    instruction_table[0x2C] = {"BIT", AddressingMode::Absolute, 3, 4};
    instruction_table[0x2D] = {"AND", AddressingMode::Absolute, 3, 4};
    instruction_table[0x2E] = {"ROL", AddressingMode::Absolute, 3, 6};

    instruction_table[0x30] = {"BMI", AddressingMode::Immediate, 2, 2}; // Relative
    instruction_table[0x31] = {"AND", AddressingMode::IndirectIndexed, 2, 5, 1};
    instruction_table[0x35] = {"AND", AddressingMode::ZeroPageX, 2, 4};
    instruction_table[0x36] = {"ROL", AddressingMode::ZeroPageX, 2, 6};
    instruction_table[0x38] = {"SEC", AddressingMode::Immediate, 1, 2};
    instruction_table[0x39] = {"AND", AddressingMode::AbsoluteY, 3, 4, 1};
    instruction_table[0x3D] = {"AND", AddressingMode::AbsoluteX, 3, 4, 1};
    instruction_table[0x3E] = {"ROL", AddressingMode::AbsoluteX, 3, 7};

    instruction_table[0x40] = {"RTI", AddressingMode::Immediate, 1, 6};
    instruction_table[0x41] = {"EOR", AddressingMode::IndexedIndirect, 2, 6};
    instruction_table[0x45] = {"EOR", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0x46] = {"LSR", AddressingMode::ZeroPage, 2, 5};
    instruction_table[0x48] = {"PHA", AddressingMode::Immediate, 1, 3};
    instruction_table[0x49] = {"EOR", AddressingMode::Immediate, 2, 2};
    instruction_table[0x4A] = {"LSR", AddressingMode::Immediate, 1, 2}; // Accumulator
    instruction_table[0x4C] = {"JMP", AddressingMode::Absolute, 3, 3};
    instruction_table[0x4D] = {"EOR", AddressingMode::Absolute, 3, 4};
    instruction_table[0x4E] = {"LSR", AddressingMode::Absolute, 3, 6};

    instruction_table[0x50] = {"BVC", AddressingMode::Immediate, 2, 2}; // Relative
    instruction_table[0x51] = {"EOR", AddressingMode::IndirectIndexed, 2, 5, 1};
    instruction_table[0x55] = {"EOR", AddressingMode::ZeroPageX, 2, 4};
    instruction_table[0x56] = {"LSR", AddressingMode::ZeroPageX, 2, 6};
    instruction_table[0x58] = {"CLI", AddressingMode::Immediate, 1, 2};
    instruction_table[0x59] = {"EOR", AddressingMode::AbsoluteY, 3, 4, 1};
    instruction_table[0x5D] = {"EOR", AddressingMode::AbsoluteX, 3, 4, 1};
    instruction_table[0x5E] = {"LSR", AddressingMode::AbsoluteX, 3, 7};

    instruction_table[0x60] = {"RTS", AddressingMode::Immediate, 1, 6};
    instruction_table[0x61] = {"ADC", AddressingMode::IndexedIndirect, 2, 6};
    instruction_table[0x65] = {"ADC", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0x66] = {"ROR", AddressingMode::ZeroPage, 2, 5};
    instruction_table[0x68] = {"PLA", AddressingMode::Immediate, 1, 4};
    instruction_table[0x69] = {"ADC", AddressingMode::Immediate, 2, 2};
    instruction_table[0x6A] = {"ROR", AddressingMode::Immediate, 1, 2}; // Accumulator
    instruction_table[0x6C] = {"JMP", AddressingMode::Indirect, 3, 5};
    instruction_table[0x6D] = {"ADC", AddressingMode::Absolute, 3, 4};
    instruction_table[0x6E] = {"ROR", AddressingMode::Absolute, 3, 6};

    instruction_table[0x70] = {"BVS", AddressingMode::Immediate, 2, 2}; // Relative
    instruction_table[0x71] = {"ADC", AddressingMode::IndirectIndexed, 2, 5, 1};
    instruction_table[0x75] = {"ADC", AddressingMode::ZeroPageX, 2, 4};
    instruction_table[0x76] = {"ROR", AddressingMode::ZeroPageX, 2, 6};
    instruction_table[0x78] = {"SEI", AddressingMode::Immediate, 1, 2};
    instruction_table[0x79] = {"ADC", AddressingMode::AbsoluteY, 3, 4, 1};
    instruction_table[0x7D] = {"ADC", AddressingMode::AbsoluteX, 3, 4, 1};
    instruction_table[0x7E] = {"ROR", AddressingMode::AbsoluteX, 3, 7};

    instruction_table[0x81] = {"STA", AddressingMode::IndexedIndirect, 2, 6};
    instruction_table[0x84] = {"STY", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0x85] = {"STA", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0x86] = {"STX", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0x88] = {"DEY", AddressingMode::Immediate, 1, 2};
    instruction_table[0x8A] = {"TXA", AddressingMode::Immediate, 1, 2};
    instruction_table[0x8C] = {"STY", AddressingMode::Absolute, 3, 4};
    instruction_table[0x8D] = {"STA", AddressingMode::Absolute, 3, 4};
    instruction_table[0x8E] = {"STX", AddressingMode::Absolute, 3, 4};

    instruction_table[0x90] = {"BCC", AddressingMode::Immediate, 2, 2}; // Relative
    instruction_table[0x91] = {"STA", AddressingMode::IndirectIndexed, 2, 6};
    instruction_table[0x94] = {"STY", AddressingMode::ZeroPageX, 2, 4};
    instruction_table[0x95] = {"STA", AddressingMode::ZeroPageX, 2, 4};
    instruction_table[0x96] = {"STX", AddressingMode::ZeroPageY, 2, 4};
    instruction_table[0x98] = {"TYA", AddressingMode::Immediate, 1, 2};
    instruction_table[0x99] = {"STA", AddressingMode::AbsoluteY, 3, 5};
    instruction_table[0x9A] = {"TXS", AddressingMode::Immediate, 1, 2};
    instruction_table[0x9D] = {"STA", AddressingMode::AbsoluteX, 3, 5};

    instruction_table[0xA0] = {"LDY", AddressingMode::Immediate, 2, 2};
    instruction_table[0xA1] = {"LDA", AddressingMode::IndexedIndirect, 2, 6};
    instruction_table[0xA2] = {"LDX", AddressingMode::Immediate, 2, 2};
    instruction_table[0xA4] = {"LDY", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0xA5] = {"LDA", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0xA6] = {"LDX", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0xA8] = {"TAY", AddressingMode::Immediate, 1, 2};
    instruction_table[0xA9] = {"LDA", AddressingMode::Immediate, 2, 2};
    instruction_table[0xAA] = {"TAX", AddressingMode::Immediate, 1, 2};
    instruction_table[0xAC] = {"LDY", AddressingMode::Absolute, 3, 4};
    instruction_table[0xAD] = {"LDA", AddressingMode::Absolute, 3, 4};
    instruction_table[0xAE] = {"LDX", AddressingMode::Absolute, 3, 4};

    instruction_table[0xB0] = {"BCS", AddressingMode::Immediate, 2, 2}; // Relative
    instruction_table[0xB1] = {"LDA", AddressingMode::IndirectIndexed, 2, 5, 1};
    instruction_table[0xB4] = {"LDY", AddressingMode::ZeroPageX, 2, 4};
    instruction_table[0xB5] = {"LDA", AddressingMode::ZeroPageX, 2, 4};
    instruction_table[0xB6] = {"LDX", AddressingMode::ZeroPageY, 2, 4};
    instruction_table[0xB8] = {"CLV", AddressingMode::Immediate, 1, 2};
    instruction_table[0xB9] = {"LDA", AddressingMode::AbsoluteY, 3, 4, 1};
    instruction_table[0xBA] = {"TSX", AddressingMode::Immediate, 1, 2};
    instruction_table[0xBC] = {"LDY", AddressingMode::AbsoluteX, 3, 4, 1};
    instruction_table[0xBD] = {"LDA", AddressingMode::AbsoluteX, 3, 4, 1};
    instruction_table[0xBE] = {"LDX", AddressingMode::AbsoluteY, 3, 4, 1};

    instruction_table[0xC0] = {"CPY", AddressingMode::Immediate, 2, 2};
    instruction_table[0xC1] = {"CMP", AddressingMode::IndexedIndirect, 2, 6};
    instruction_table[0xC4] = {"CPY", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0xC5] = {"CMP", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0xC6] = {"DEC", AddressingMode::ZeroPage, 2, 5};
    instruction_table[0xC8] = {"INY", AddressingMode::Immediate, 1, 2};
    instruction_table[0xC9] = {"CMP", AddressingMode::Immediate, 2, 2};
    instruction_table[0xCA] = {"DEX", AddressingMode::Immediate, 1, 2};
    instruction_table[0xCC] = {"CPY", AddressingMode::Absolute, 3, 4};
    instruction_table[0xCD] = {"CMP", AddressingMode::Absolute, 3, 4};
    instruction_table[0xCE] = {"DEC", AddressingMode::Absolute, 3, 6};

    instruction_table[0xD0] = {"BNE", AddressingMode::Immediate, 2, 2}; // Relative
    instruction_table[0xD1] = {"CMP", AddressingMode::IndirectIndexed, 2, 5, 1};
    instruction_table[0xD5] = {"CMP", AddressingMode::ZeroPageX, 2, 4};
    instruction_table[0xD6] = {"DEC", AddressingMode::ZeroPageX, 2, 6};
    instruction_table[0xD8] = {"CLD", AddressingMode::Immediate, 1, 2};
    instruction_table[0xD9] = {"CMP", AddressingMode::AbsoluteY, 3, 4, 1};
    instruction_table[0xDD] = {"CMP", AddressingMode::AbsoluteX, 3, 4, 1};
    instruction_table[0xDE] = {"DEC", AddressingMode::AbsoluteX, 3, 7};

    instruction_table[0xE0] = {"CPX", AddressingMode::Immediate, 2, 2};
    instruction_table[0xE1] = {"SBC", AddressingMode::IndexedIndirect, 2, 6};
    instruction_table[0xE4] = {"CPX", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0xE5] = {"SBC", AddressingMode::ZeroPage, 2, 3};
    instruction_table[0xE6] = {"INC", AddressingMode::ZeroPage, 2, 5};
    instruction_table[0xE8] = {"INX", AddressingMode::Immediate, 1, 2};
    instruction_table[0xE9] = {"SBC", AddressingMode::Immediate, 2, 2};
    instruction_table[0xEA] = {"NOP", AddressingMode::Immediate, 1, 2};
    instruction_table[0xEC] = {"CPX", AddressingMode::Absolute, 3, 4};
    instruction_table[0xED] = {"SBC", AddressingMode::Absolute, 3, 4};
    instruction_table[0xEE] = {"INC", AddressingMode::Absolute, 3, 6};

    instruction_table[0xF0] = {"BEQ", AddressingMode::Immediate, 2, 2}; // Relative
    instruction_table[0xF1] = {"SBC", AddressingMode::IndirectIndexed, 2, 5, 1};
    instruction_table[0xF5] = {"SBC", AddressingMode::ZeroPageX, 2, 4};
    instruction_table[0xF6] = {"INC", AddressingMode::ZeroPageX, 2, 6};
    instruction_table[0xF8] = {"SED", AddressingMode::Immediate, 1, 2};
    instruction_table[0xF9] = {"SBC", AddressingMode::AbsoluteY, 3, 4, 1};
    instruction_table[0xFD] = {"SBC", AddressingMode::AbsoluteX, 3, 4, 1};
    instruction_table[0xFE] = {"INC", AddressingMode::AbsoluteX, 3, 7};
}

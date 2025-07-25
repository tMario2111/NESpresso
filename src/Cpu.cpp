#include "Cpu.hpp"

#include "Memory.hpp"

Cpu &Cpu::instance()
{
    static Cpu cpu;
    return cpu;
}

void Cpu::initInstructionTable()
{
    // These are only the OFFICIAL 6502 instructions.
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
    instruction_table[0x11] = {"ORA", AddressingMode::IndirectIndexed, 2, 5, true};
    instruction_table[0x15] = {"ORA", AddressingMode::ZeroPageX, 2, 4};
    instruction_table[0x16] = {"ASL", AddressingMode::ZeroPageX, 2, 6};
    instruction_table[0x18] = {"CLC", AddressingMode::Immediate, 1, 2};
    instruction_table[0x19] = {"ORA", AddressingMode::AbsoluteY, 3, 4, true};
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

void Cpu::setNegativeFlag(bool value)
{
    if (value)
        registers.p |= 0x80; // Set Negative Flag
    else
        registers.p &= ~0x80; // Clear Negative Flag
}

void Cpu::setZeroFlag(bool value)
{
    if (value)
        registers.p |= 0x02; // Set Zero Flag
    else
        registers.p &= ~0x02; // Clear Zero Flag
}

void Cpu::setCarryFlag(bool value)
{
    if (value)
        registers.p |= 0x01; // Set Carry Flag
    else
        registers.p &= ~0x01; // Clear Carry Flag
}

void Cpu::setOverflowFlag(bool value)
{
    if (value)
        registers.p |= 0x40; // Set Overflow Flag
    else
        registers.p &= ~0x40; // Clear Overflow Flag
}

void Cpu::setDecimalFlag(bool value)
{
    if (value)
        registers.p |= 0x08; // Set Decimal Flag
    else
        registers.p &= ~0x08; // Clear Decimal Flag
}

void Cpu::setInterruptDisableFlag(bool value)
{
    if (value)
        registers.p |= 0x04; // Set Interrupt Disable Flag
    else
        registers.p &= ~0x04; // Clear Interrupt Disable Flag
}

inline void Cpu::writeMemory(uint16_t address, uint8_t value)
{
    memory.ram[address] = value;
}


inline uint8_t Cpu::readMemory(uint16_t uint16)
{
    return memory.ram[uint16];
}

// All of the official instructions are defined in the functions below:
void Cpu::ADC(uint8_t value)
{
    uint8_t a = registers.a;
    uint8_t carry = (registers.p & 0x01); // C = bit 0

    uint16_t result = a + value + carry;

    // === Set Flags ===

    // Carry Flag (C)
    setCarryFlag(result > 0xFF);

    // Zero Flag (Z)
    setZeroFlag((result & 0xFF) == 0);

    // Negative Flag (N)
    setNegativeFlag((result & 0x80) != 0);

    // Overflow Flag (V)
    int overflow = ((a ^ value) & 0x80) == 0 && ((a ^ result) & 0x80) != 0;
    setOverflowFlag(overflow);

    // === Store result ===
    registers.a = result & 0xFF;
}

// AND (self-explanatory)
void Cpu::AND(uint8_t value)
{
    registers.a &= value;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.a == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.a & 0x80) != 0);
}

// Arithmetic Shift Left (ASL) - shifts bits left and sets flags accordingly
void Cpu::ASL_Accumulator()
{
    uint8_t &a = registers.a;
    setCarryFlag((a & 0x80) != 0);
    a <<= 1;
    setZeroFlag(a == 0);
    setNegativeFlag((a & 0x80) != 0);
}


void Cpu::ASL_Memory(uint16_t address)
{
    uint8_t value = readMemory(address);
    setCarryFlag((value & 0x80) != 0);
    value <<= 1;
    setZeroFlag(value == 0);
    setNegativeFlag((value & 0x80) != 0);
    writeMemory(address, value);
}

// Branch if Carry Clear (BCC) - branches if the Carry Flag is clear
void Cpu::BCC(uint8_t value)
{
    // Branch if Carry Clear
    if (!(registers.p & 0x01))
    {
        // Check Carry Flag
        registers.pc += value; // Adjust Program Counter
    }
}

// Branch if Carry Set (BCS) - branches if the Carry Flag is set
void Cpu::BCS(uint8_t value)
{
    // Branch if Carry Set
    if (registers.p & 0x01)
    {
        // Check Carry Flag
        registers.pc += value; // Adjust Program Counter
    }
}

void Cpu::BEQ(uint8_t value)
{
    // Branch if Equal (Zero Flag is set)
    if (registers.p & 0x02)
    {
        // Check Zero Flag
        registers.pc += value; // Adjust Program Counter
    }
}

void Cpu::BIT(uint8_t value)
{
    // Bit Test - sets flags based on the value
    setZeroFlag((registers.a & value) == 0);
    setNegativeFlag((value & 0x80) != 0);
    setOverflowFlag((value & 0x40) != 0);
}

void Cpu::BMI(uint8_t value)
{
    // Branch if Minus (Negative Flag is set)
    if (registers.p & 0x80)
    {
        // Check Negative Flag
        registers.pc += value; // Adjust Program Counter
    }
}

void Cpu::BNE(uint8_t value)
{
    // Branch if Not Equal (Zero Flag is clear)
    if (!(registers.p & 0x02))
    {
        // Check Zero Flag
        registers.pc += value; // Adjust Program Counter
    }
}

void Cpu::BPL(uint8_t value)
{
    // Branch if Positive (Negative Flag is clear)
    if (!(registers.p & 0x80))
    {
        // Check Negative Flag
        registers.pc += value; // Adjust Program Counter
    }
}

// void Cpu::BRK
// TODO: Mario fa stack

void Cpu::BVC(uint8_t value)
{
    // Branch if Overflow Clear (Overflow Flag is clear)
    if (!(registers.p & 0x40))
    {
        // Check Overflow Flag
        registers.pc += value; // Adjust Program Counter
    }
}

void Cpu::BVS(uint8_t value)
{
    // Branch if Overflow Set (Overflow Flag is set)
    if (registers.p & 0x40)
    {
        // Check Overflow Flag
        registers.pc += value; // Adjust Program Counter
    }
}

void Cpu::CLC()
{
    // Clear Carry Flag
    setCarryFlag(false);
}

void Cpu::CLD()
{
    // Clear Decimal Flag
    setDecimalFlag(false);
}

void Cpu::CLI()
{
    // Clear Interrupt Disable Flag
    setInterruptDisableFlag(false);
}

void Cpu::CLV()
{
    setOverflowFlag(false);
}

void Cpu::CMP(uint8_t value)
{
    // Compare - sets flags based on the comparison
    uint8_t a = registers.a;

    setCarryFlag(a >= value);
    setZeroFlag(a == value);
    setNegativeFlag((a - value) & 0x80);
}

void Cpu::CPX(uint8_t value)
{
    // Compare X Register
    uint8_t x = registers.x;

    setCarryFlag(x >= value);
    setZeroFlag(x == value);
    setNegativeFlag((x - value) & 0x80);
}

void Cpu::CPY(uint8_t value)
{
    // Compare Y Register
    uint8_t y = registers.y;

    setCarryFlag(y >= value);
    setZeroFlag(y == value);
    setNegativeFlag((y - value) & 0x80);
}

// Decrement Memory (DEC) - decrements the value at the specified address
void Cpu::DEC(uint16_t address)
{
    uint8_t value = readMemory(address);
    value--;
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
    writeMemory(address, value);
}


void Cpu::DEX()
{
    // Decrement X Register
    registers.x--;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.x == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.x & 0x80) != 0);
}

void Cpu::DEY()
{
    // Decrement Y Register
    registers.y--;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.y == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.y & 0x80) != 0);
}

void Cpu::EOR(uint8_t value)
{
    // Exclusive OR - performs bitwise XOR with the accumulator
    registers.a ^= value;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.a == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.a & 0x80) != 0);
}

// Increment Memory - increments the value at the specified address
void Cpu::INC(uint16_t address)
{
    uint8_t value = readMemory(address);
    value++;
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
    writeMemory(address, value);
}


void Cpu::INX()
{
    // Increment X Register
    registers.x++;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.x == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.x & 0x80) != 0);
}

void Cpu::INY()
{
    // Increment Y Register
    registers.y++;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.y == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.y & 0x80) != 0);
}

void Cpu::JMP(uint16_t address)
{
    // Jump to a specific address
    registers.pc = address;
}

void Cpu::JSR(const uint16_t address)
{
    memory.push16(registers.sp, registers.pc + 1);
    registers.pc = address;
}

void Cpu::LDA(uint8_t value)
{
    // Load Accumulator
    registers.a = value;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.a == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.a & 0x80) != 0);
}

void Cpu::LDX(uint8_t value)
{
    // Load X Register
    registers.x = value;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.x == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.x & 0x80) != 0);
}

void Cpu::LDY(uint8_t value)
{
    // Load Y Register
    registers.y = value;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.y == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.y & 0x80) != 0);
}

// Logical Shift Right (LSR) - shifts bits right and sets flags accordingly
void Cpu::LSR_Accumulator()
{
    uint8_t &a = registers.a;
    setCarryFlag(a & 0x01); // Bit 0 pleacă
    a >>= 1;
    setZeroFlag(a == 0);
    setNegativeFlag(false); // LSR setează întotdeauna N = 0
}

void Cpu::LSR_Memory(uint16_t address)
{
    uint8_t value = readMemory(address);
    setCarryFlag(value & 0x01);
    value >>= 1;
    setZeroFlag(value == 0);
    setNegativeFlag(false);
    writeMemory(address, value);
}


void Cpu::NOP()
{
    // No Operation - does nothing
    // As useful as me
}

void Cpu::ORA(uint8_t value)
{
    // Logical OR - performs bitwise OR with the accumulator
    registers.a |= value;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.a == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.a & 0x80) != 0);
}

void Cpu::PHA()
{
    memory.push8(registers.sp, registers.a);
}

// TODO: I have no idea if this is correct (~Mario)
// The wiki isn't too helpful
void Cpu::PHP()
{
    memory.push8(registers.sp, registers.p | 0x30);
}

void Cpu::PLA()
{
    registers.a = memory.pop8(registers.sp);
}

// TODO: Again, not too sure (~Mario)
void Cpu::PLP()
{
    registers.p = memory.pop8(registers.sp) | 0x20;
}

// Rotate Left (ROL) - shifts bits left and wraps the leftmost bit to the right
void Cpu::ROL_Accumulator()
{
    uint8_t &a = registers.a;
    bool oldCarry = (registers.p & 0x01) != 0; // Carry Flag (C)
    setCarryFlag(a & 0x80);
    a = (a << 1) | (oldCarry ? 1 : 0);
    setZeroFlag(a == 0);
    setNegativeFlag(a & 0x80);
}

void Cpu::ROL_Memory(uint16_t address)
{
    uint8_t value = readMemory(address);
    bool oldCarry = (registers.p & 0x01) != 0; // Carry Flag (C)
    setCarryFlag(value & 0x80);
    value = (value << 1) | (oldCarry ? 1 : 0);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
    writeMemory(address, value);
}

// Rotate Right (ROR) - shifts bits right and wraps the rightmost bit to the left
void Cpu::ROR_Accumulator()
{
    uint8_t &a = registers.a;
    bool oldCarry = (registers.p & 0x01) != 0; // Carry Flag (C)
    setCarryFlag(a & 0x01);
    a = (a >> 1) | (oldCarry ? 0x80 : 0);
    setZeroFlag(a == 0);
    setNegativeFlag(a & 0x80);
}

void Cpu::ROR_Memory(uint16_t address)
{
    uint8_t value = readMemory(address);
    bool oldCarry = (registers.p & 0x01) != 0; // Carry Flag (C)
    setCarryFlag(value & 0x01);
    value = (value >> 1) | (oldCarry ? 0x80 : 0);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
    writeMemory(address, value);
}

void Cpu::RTI()
{
    registers.p = memory.pop8(registers.sp) | 0x20;
    registers.pc = memory.pop16(registers.sp);
}

void Cpu::RTS()
{
    registers.pc = memory.pop16(registers.sp);
    ++registers.pc;
}

void Cpu::SBC(uint8_t value)
{
    // Subtract with Carry - subtracts value and Carry Flag from the accumulator
    uint8_t a = registers.a;
    uint8_t carry = ~(registers.p & 0x01); // C = bit 0

    uint16_t result = a - value - carry;

    // === Set Flags ===

    // Carry Flag (C)
    setCarryFlag(result < 0x100);

    // Zero Flag (Z)
    setZeroFlag((result & 0xFF) == 0);

    // Negative Flag (N)
    setNegativeFlag((result & 0x80) != 0);

    // Overflow Flag (V)
    int overflow = ((a ^ value) & 0x80) != 0 && ((a ^ result) & 0x80) != 0;
    setOverflowFlag(overflow);

    // === Store result ===
    registers.a = result & 0xFF;
}

void Cpu::SEC()
{
    // Set Carry Flag
    setCarryFlag(true);
}

void Cpu::SED()
{
    // Set Decimal Flag
    setDecimalFlag(true);
}

void Cpu::SEI()
{
    // Set Interrupt Disable Flag
    setInterruptDisableFlag(true);
}

void Cpu::STA(uint16_t address)
{
    // Store Accumulator
    writeMemory(address, registers.a);
}

void Cpu::STX(uint16_t address)
{
    // Store X Register
    writeMemory(address, registers.x);
}

void Cpu::STY(uint16_t address)
{
    // Store Y Register
    writeMemory(address, registers.y);
}

void Cpu::TAX()
{
    // Transfer Accumulator to X Register
    registers.x = registers.a;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.x == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.x & 0x80) != 0);
}

void Cpu::TAY()
{
    // Transfer Accumulator to Y Register
    registers.y = registers.a;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.y == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.y & 0x80) != 0);
}

void Cpu::TSX()
{
    // Transfer Stack Pointer to X Register
    registers.x = registers.sp;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.x == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.x & 0x80) != 0);
}

void Cpu::TXA()
{
    // Transfer X Register to Accumulator
    registers.a = registers.x;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.a == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.a & 0x80) != 0);
}

void Cpu::TXS()
{
    // Transfer X Register to Stack Pointer
    registers.sp = registers.x;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.sp == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.sp & 0x80) != 0);
}

void Cpu::TYA()
{
    // Transfer Y Register to Accumulator
    registers.a = registers.y;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.a == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.a & 0x80) != 0);
}

// NESpresso CPU Header
// Date: 2025-07-30 00:42:53 UTC
// User: nicusor43

#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include "Memory.hpp"

#define CREATE_INSTR(func, mode, bytes, cycles, page_cross_penalty) \
    Instruction{#func, std::function<void(uint8_t)>([this](uint8_t v) { this->func(v); }), mode, bytes, cycles, page_cross_penalty}
#define CREATE_INSTR_ADDR(func, mode, bytes, cycles, page_cross_penalty) \
    Instruction{#func, std::function<void(uint16_t)>([this](uint16_t a) { this->func(a); }), mode, bytes, cycles, page_cross_penalty}
#define CREATE_INSTR_IMPLIED(func, mode, bytes, cycles) \
    Instruction{#func, std::function<void()>([this]() { this->func(); }), mode, bytes, cycles, false}

class Cpu {
public:
    static Cpu &instance();

    Cpu(const Cpu &) = delete;

    Cpu &operator=(const Cpu &) = delete;

    void executeInstruction();

    uint64_t total_cycles = 0;

    struct Registers {
        uint8_t a{};
        uint8_t x{}, y{};
        uint16_t pc = 0xC000;
        uint8_t sp = 0xFD;
        uint8_t p = 0x24;
    } registers;

    enum class AddressingMode {
        Immediate, ZeroPage, ZeroPageX, ZeroPageY,
        Absolute, AbsoluteX, AbsoluteY,
        Indirect, IndexedIndirect, IndirectIndexed,
        Implied, Relative
    };

    struct Instruction {
        std::string mnemonic;
        std::variant<
            std::function<void(uint8_t)>,
            std::function<void(uint16_t)>,
            std::function<void()> >
        execute;
        AddressingMode mode;
        uint8_t bytes;
        uint8_t cycles;
        bool page_crossed_penalty;
    };

    std::array<Instruction, 256> instruction_table;

    uint8_t readMemory(uint16_t address);

    void writeMemory(uint16_t address, uint8_t value);

private:
    Memory &memory = Memory::instance();

    Cpu();

    ~Cpu() = default;

    void initInstructionTable();

    

    // Flag Setters
    void setNegativeFlag(bool value);

    void setZeroFlag(bool value);

    void setCarryFlag(bool value);

    void setOverflowFlag(bool value);

    void setDecimalFlag(bool value);

    void setInterruptDisableFlag(bool value);

    // Official Instructions
    void ADC(uint8_t value);

    void AND(uint8_t value);

    void ASL_Accumulator();

    void ASL_Memory(uint16_t address);

    void BCC(uint8_t value);

    void BCS(uint8_t value);

    void BEQ(uint8_t value);

    void BIT(uint8_t value);

    void BMI(uint8_t value);

    void BNE(uint8_t value);

    void BPL(uint8_t value);

    void BRK();

    void BVC(uint8_t value);

    void BVS(uint8_t value);

    void CLC();

    void CLD();

    void CLI();

    void CLV();

    void CMP(uint8_t value);

    void CPX(uint8_t value);

    void CPY(uint8_t value);

    void DEC(uint16_t address);

    void DEX();

    void DEY();

    void EOR(uint8_t value);

    void INC(uint16_t address);

    void INX();

    void INY();

    void JMP(uint16_t address);

    void JSR(uint16_t address);

    void LDA(uint8_t value);

    void LDX(uint8_t value);

    void LDY(uint8_t value);

    void LSR_Accumulator();

    void LSR_Memory(uint16_t address);

    void NOP();

    void ORA(uint8_t value);

    void PHA();

    void PHP();

    void PLA();

    void PLP();

    void ROL_Accumulator();

    void ROL_Memory(uint16_t address);

    void ROR_Accumulator();

    void ROR_Memory(uint16_t address);

    void RTI();

    void RTS();

    void SBC(uint8_t value);

    void SEC();

    void SED();

    void SEI();

    void STA(uint16_t address);

    void STX(uint16_t address);

    void STY(uint16_t address);

    void TAX();

    void TAY();

    void TSX();

    void TXA();

    void TXS();

    void TYA();

    // Unofficial Instructions
    void NOP_unofficial(uint16_t address);

    void LAX(uint8_t value);

    void SAX(uint16_t address);

    void DCP(uint16_t address);

    void ISC(uint16_t address);

    void SLO(uint16_t address);

    void RLA(uint16_t address);

    void SRE(uint16_t address);

    void RRA(uint16_t address);
};

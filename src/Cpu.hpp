#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <variant>

#include "Memory.hpp"

class Cpu {
public:
    Cpu(const Cpu &) = delete;

    Cpu &operator=(const Cpu &) = delete;

    static Cpu &instance();

    uint8_t executeInstruction();

    struct Registers {
        uint8_t a;
        uint8_t x, y;
        uint16_t pc;

        // sp is initialized to 0xFD on console startup
        uint8_t sp = 0xFD;

        uint8_t p;
    } registers{};

    enum class AddressingMode {
        Immediate,
        ZeroPage,
        ZeroPageX,
        ZeroPageY,
        Absolute,
        AbsoluteX,
        AbsoluteY,
        Indirect,
        IndexedIndirect,
        IndirectIndexed,
        Implied,
        Relative
    };

    struct Instruction {
        // std::string mnemonic;
        std::variant<
            std::function<void(uint8_t)>,
            std::function<void(uint16_t)>,
            std::function<void()>
        > execute;
        AddressingMode mode;
        uint8_t bytes;
        uint8_t cycles;
        bool page_crossed;
    };

    std::array<Instruction, 256> instruction_table;

private:
    Memory &memory = Memory::instance();

    Cpu() = default;

    ~Cpu() = default;

    void initInstructionTable();

    inline void setNegativeFlag(bool value);

    inline void setZeroFlag(bool value);

    inline void setCarryFlag(bool value);

    inline void setOverflowFlag(bool value);

    inline void setDecimalFlag(bool value);

    inline void setInterruptDisableFlag(bool value);

    inline uint8_t readMemory(uint16_t uint16);

    inline void writeMemory(uint16_t address, uint8_t value);

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
};

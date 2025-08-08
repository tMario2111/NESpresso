// NESpresso CPU Header
// Date: 2025-08-08 15:02:25 UTC
// User: nicusor43

#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include "Memory.hpp"

class Cpu {
public:
    static Cpu &instance();

    Cpu(const Cpu &) = delete;
    Cpu &operator=(const Cpu &) = delete;

    // Execute exactly one instruction at current PC:
    // - decodes addressing (address/value/page_crossed),
    // - dispatches via std::variant,
    // - accounts base cycles (+ page penalty when applicable),
    // - advances PC if instruction did not set it explicitly.
    void executeInstruction();

    // Global CPU cycle counter (for PPU/logs)
    uint64_t total_cycles = 0;

    // 6502 registers
    struct Registers {
        uint8_t a{};
        uint8_t x{}, y{};
        uint16_t pc = 0xC000;
        uint8_t sp = 0xFD;
        uint8_t p = 0x24;
    } registers;

    // Addressing modes
    enum class AddressingMode {
        Immediate, ZeroPage, ZeroPageX, ZeroPageY,
        Absolute, AbsoluteX, AbsoluteY,
        Indirect, IndexedIndirect, IndirectIndexed,
        Implied, Relative
    };

    // Instruction descriptor (one per opcode)
    struct Instruction {
        std::string mnemonic; // used by disassembler/log only
        std::variant<
            std::function<void(uint8_t)>,   // ex: LDA #imm, ADC #imm, branches
            std::function<void(uint16_t)>,  // ex: STA $addr, RMW, JMP/JSR with address
            std::function<void()>           // ex: implied: CLC, NOP, RTS, etc.
        > execute;
        AddressingMode mode;
        uint8_t bytes;
        uint8_t cycles;
        bool page_crossed_penalty;
    };

    // 256-opcode table
    std::array<Instruction, 256> instruction_table;

    // Bus API
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

// Inline/constexpr factories (replace the former macros)
// Keep std::variant; avoid preprocessor; pass member-function pointer and bind to 'self'.
namespace cpu_instr_factory {

// Implied (void()) — page penalty is always false
inline Cpu::Instruction Make0(const char* name,
                              Cpu::AddressingMode mode,
                              uint8_t bytes,
                              uint8_t cycles,
                              Cpu* self,
                              void (Cpu::*fn)()) {
    return Cpu::Instruction{
        name,
        std::function<void()>([self, fn]() { (self->*fn)(); }),
        mode,
        bytes,
        cycles,
        false
    };
}

// Byte operand (void(uint8_t))
inline Cpu::Instruction Make8(const char* name,
                              Cpu::AddressingMode mode,
                              uint8_t bytes,
                              uint8_t cycles,
                              bool page_penalty,
                              Cpu* self,
                              void (Cpu::*fn)(uint8_t)) {
    return Cpu::Instruction{
        name,
        std::function<void(uint8_t)>([self, fn](uint8_t v) { (self->*fn)(v); }),
        mode,
        bytes,
        cycles,
        page_penalty
    };
}

// Address operand (void(uint16_t))
inline Cpu::Instruction Make16(const char* name,
                               Cpu::AddressingMode mode,
                               uint8_t bytes,
                               uint8_t cycles,
                               bool page_penalty,
                               Cpu* self,
                               void (Cpu::*fn)(uint16_t)) {
    return Cpu::Instruction{
        name,
        std::function<void(uint16_t)>([self, fn](uint16_t a) { (self->*fn)(a); }),
        mode,
        bytes,
        cycles,
        page_penalty
    };
}

} // namespace cpu_instr_factory
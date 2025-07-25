#pragma once

#include <array>
#include <cstdint>
#include <string>

class Cpu {
public:
    Cpu(const Cpu &) = delete;

    Cpu &operator=(const Cpu &) = delete;

    static Cpu &instance() {
        static Cpu cpu;
        return cpu;
    }

    struct Registers {
        uint8_t a;
        uint8_t x, y;
        uint16_t pc;
        uint8_t s;
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
        IndirectIndexed
    };

    struct Instruction {
        std::string mnemonic;
        AddressingMode mode;
        uint8_t bytes;
        uint8_t cycles;
        bool page_crossed;
    } instruction{};

    std::array<Instruction, 256> instruction_table;

private:
    Cpu() = default;

    ~Cpu() = default;

    void initInstructionTable();
};

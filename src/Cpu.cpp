#include "Cpu.hpp"

#include <iostream>

#include "Memory.hpp"

Cpu::Cpu() {
    this->initInstructionTable();
}

Cpu &Cpu::instance() {
    static Cpu cpu;
    return cpu;
}

uint8_t Cpu::executeInstruction() {
    uint16_t initial_pc = registers.pc;
    uint8_t opcode = readMemory(registers.pc);
    Instruction &instruction = instruction_table[opcode];
    bool page_crossed = false;

    // Determină valoarea sau adresa în funcție de modul de adresare
    switch (instruction.mode) {
        case AddressingMode::Immediate: {
            uint8_t value = readMemory(registers.pc + 1);
            if (std::holds_alternative<std::function<void(uint8_t)> >(instruction.execute)) {
                std::get<std::function<void(uint8_t)> >(instruction.execute)(value);
            }
            break;
        }
        case AddressingMode::ZeroPage: {
            uint8_t addr_low = readMemory(registers.pc + 1);
            uint16_t address = addr_low;
            if (std::holds_alternative<std::function<void(uint16_t)> >(instruction.execute)) {
                std::get<std::function<void(uint16_t)> >(instruction.execute)(address);
            } else if (std::holds_alternative<std::function<void(uint8_t)> >(instruction.execute)) {
                std::get<std::function<void(uint8_t)> >(instruction.execute)(readMemory(address));
            }
            break;
        }
        case AddressingMode::ZeroPageX: {
            uint8_t addr_low = readMemory(registers.pc + 1);
            uint16_t address = (addr_low + registers.x) & 0xFF;
            if (std::holds_alternative<std::function<void(uint16_t)> >(instruction.execute)) {
                std::get<std::function<void(uint16_t)> >(instruction.execute)(address);
            } else if (std::holds_alternative<std::function<void(uint8_t)> >(instruction.execute)) {
                std::get<std::function<void(uint8_t)> >(instruction.execute)(readMemory(address));
            }
            break;
        }
        case AddressingMode::ZeroPageY: {
            uint8_t addr_low = readMemory(registers.pc + 1);
            uint16_t address = (addr_low + registers.y) & 0xFF;
            if (std::holds_alternative<std::function<void(uint16_t)> >(instruction.execute)) {
                std::get<std::function<void(uint16_t)> >(instruction.execute)(address);
            } else if (std::holds_alternative<std::function<void(uint8_t)> >(instruction.execute)) {
                std::get<std::function<void(uint8_t)> >(instruction.execute)(readMemory(address));
            }
            break;
        }
        case AddressingMode::Absolute: {
            uint16_t address = readMemory(registers.pc + 1) | (readMemory(registers.pc + 2) << 8);
            if (std::holds_alternative<std::function<void(uint16_t)> >(instruction.execute)) {
                std::get<std::function<void(uint16_t)> >(instruction.execute)(address);
            } else if (std::holds_alternative<std::function<void(uint8_t)> >(instruction.execute)) {
                std::get<std::function<void(uint8_t)> >(instruction.execute)(readMemory(address));
            }
            break;
        }
        case AddressingMode::AbsoluteX: {
            uint16_t base_address = readMemory(registers.pc + 1) | (readMemory(registers.pc + 2) << 8);
            uint16_t effective_address = base_address + registers.x;
            page_crossed = (base_address & 0xFF00) != (effective_address & 0xFF00);
            if (std::holds_alternative<std::function<void(uint16_t)> >(instruction.execute)) {
                std::get<std::function<void(uint16_t)> >(instruction.execute)(effective_address);
            } else if (std::holds_alternative<std::function<void(uint8_t)> >(instruction.execute)) {
                std::get<std::function<void(uint8_t)> >(instruction.execute)(readMemory(effective_address));
            }
            break;
        }
        case AddressingMode::AbsoluteY: {
            uint16_t base_address = readMemory(registers.pc + 1) | (readMemory(registers.pc + 2) << 8);
            uint16_t effective_address = base_address + registers.y;
            page_crossed = (base_address & 0xFF00) != (effective_address & 0xFF00);

            if (std::holds_alternative<std::function<void(uint16_t)> >(instruction.execute)) {
                std::get<std::function<void(uint16_t)> >(instruction.execute)(effective_address);
            } else if (std::holds_alternative<std::function<void(uint8_t)> >(instruction.execute)) {
                std::get<std::function<void(uint8_t)> >(instruction.execute)(readMemory(effective_address));
            }
            break;
        }
        case AddressingMode::Indirect: {
            uint16_t ptr_address = readMemory(registers.pc + 1) | (readMemory(registers.pc + 2) << 8);

            uint16_t address;
            if ((ptr_address & 0x00FF) == 0x00FF) {
                // Bug hardware: la limita paginii, high byte se citește de la începutul paginii
                uint8_t low_byte = readMemory(ptr_address);
                uint8_t high_byte = readMemory(ptr_address & 0xFF00); // Forțează adresa la xx00
                address = (high_byte << 8) | low_byte;
            } else {
                address = readMemory(ptr_address) | (readMemory(ptr_address + 1) << 8);
            }

            if (std::holds_alternative<std::function<void(uint16_t)> >(instruction.execute)) {
                std::get<std::function<void(uint16_t)> >(instruction.execute)(address);
            }
            break;
        }
        case AddressingMode::IndexedIndirect: {
            uint8_t zp_addr = readMemory(registers.pc + 1);
            uint8_t effective_zp = (zp_addr + registers.x) & 0xFF; // Forțează wrap-around pe pagina zero

            // Citim adresa efectivă de la (effective_zp) și (effective_zp + 1)
            uint8_t low_byte = readMemory(effective_zp);
            uint8_t high_byte = readMemory((effective_zp + 1) & 0xFF); // Wrap-around în pagina zero
            uint16_t effective_address = (high_byte << 8) | low_byte;

            if (std::holds_alternative<std::function<void(uint16_t)> >(instruction.execute)) {
                std::get<std::function<void(uint16_t)> >(instruction.execute)(effective_address);
            } else if (std::holds_alternative<std::function<void(uint8_t)> >(instruction.execute)) {
                std::get<std::function<void(uint8_t)> >(instruction.execute)(readMemory(effective_address));
            }
            break;
        }
        case AddressingMode::IndirectIndexed: {
            uint8_t zp_addr = readMemory(registers.pc + 1);

            // Citim adresa de bază din pagina zero
            uint8_t low_byte = readMemory(zp_addr);
            uint8_t high_byte = readMemory((zp_addr + 1) & 0xFF); // Wrap-around în pagina zero
            uint16_t base_address = (high_byte << 8) | low_byte;

            // Adăugăm Y pentru a obține adresa efectivă
            uint16_t effective_address = base_address + registers.y;
            page_crossed = (base_address & 0xFF00) != (effective_address & 0xFF00);

            if (std::holds_alternative<std::function<void(uint16_t)> >(instruction.execute)) {
                std::get<std::function<void(uint16_t)> >(instruction.execute)(effective_address);
            } else if (std::holds_alternative<std::function<void(uint8_t)> >(instruction.execute)) {
                std::get<std::function<void(uint8_t)> >(instruction.execute)(readMemory(effective_address));
            }
            break;
        }
        case AddressingMode::Implied: {
            if (std::holds_alternative<std::function<void()> >(instruction.execute)) {
                std::get<std::function<void()> >(instruction.execute)();
            }
            break;
        }
        case AddressingMode::Relative: {
            uint8_t offset = readMemory(registers.pc + 1);
            if (std::holds_alternative<std::function<void(uint8_t)> >(instruction.execute)) {
                std::get<std::function<void(uint8_t)> >(instruction.execute)(offset);
            }
            break;
        }
    }

    // Actualizează PC-ul dacă nu a fost modificat de o instrucțiune de salt (JMP, JSR, branch, etc.)
    if (registers.pc == initial_pc) {
        registers.pc += instruction.bytes;
    }

    uint8_t instruction_cycles = instruction.cycles + (instruction.page_crossed && page_crossed ? 1 : 0);
    total_cycles += instruction_cycles;

    return instruction_cycles;
}


void Cpu::initInstructionTable() {
    instruction_table[0x00] = {std::function<void()>([this]() { BRK(); }), AddressingMode::Implied, 1, 7};
    instruction_table[0x01] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ORA(value); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0x05] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ORA(value); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x06] = {
        std::function<void(uint16_t)>([this](uint16_t address) { ASL_Memory(address); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0x08] = {std::function<void()>([this]() { PHP(); }), AddressingMode::Implied, 1, 3};
    instruction_table[0x09] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ORA(value); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0x0A] = {std::function<void()>([this]() { ASL_Accumulator(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x0D] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ORA(value); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x0E] = {
        std::function<void(uint16_t)>([this](uint16_t address) { ASL_Memory(address); }), AddressingMode::Absolute, 3, 6
    };

    instruction_table[0x10] = {
        std::function<void(uint8_t)>([this](uint8_t value) { BPL(value); }), AddressingMode::Relative, 2, 2
    };
    instruction_table[0x11] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ORA(value); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0x15] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ORA(value); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x16] = {
        std::function<void(uint16_t)>([this](uint16_t address) { ASL_Memory(address); }), AddressingMode::ZeroPageX, 2,
        6
    };
    instruction_table[0x18] = {std::function<void()>([this]() { CLC(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x19] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ORA(value); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0x1D] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ORA(value); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0x1E] = {
        std::function<void(uint16_t)>([this](uint16_t address) { ASL_Memory(address); }), AddressingMode::AbsoluteX, 3,
        7
    };

    instruction_table[0x20] = {
        std::function<void(uint16_t)>([this](uint16_t address) { JSR(address); }), AddressingMode::Absolute, 3, 6
    };
    instruction_table[0x21] = {
        std::function<void(uint8_t)>([this](uint8_t value) { AND(value); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0x24] = {
        std::function<void(uint8_t)>([this](uint8_t value) { BIT(value); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x25] = {
        std::function<void(uint8_t)>([this](uint8_t value) { AND(value); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x26] = {
        std::function<void(uint16_t)>([this](uint16_t address) { ROL_Memory(address); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0x28] = {std::function<void()>([this]() { PLP(); }), AddressingMode::Implied, 1, 4};
    instruction_table[0x29] = {
        std::function<void(uint8_t)>([this](uint8_t value) { AND(value); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0x2A] = {std::function<void()>([this]() { ROL_Accumulator(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x2C] = {
        std::function<void(uint8_t)>([this](uint8_t value) { BIT(value); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x2D] = {
        std::function<void(uint8_t)>([this](uint8_t value) { AND(value); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x2E] = {
        std::function<void(uint16_t)>([this](uint16_t address) { ROL_Memory(address); }), AddressingMode::Absolute, 3, 6
    };

    instruction_table[0x30] = {
        std::function<void(uint8_t)>([this](uint8_t value) { BMI(value); }), AddressingMode::Relative, 2, 2
    };
    instruction_table[0x31] = {
        std::function<void(uint8_t)>([this](uint8_t value) { AND(value); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0x35] = {
        std::function<void(uint8_t)>([this](uint8_t value) { AND(value); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x36] = {
        std::function<void(uint16_t)>([this](uint16_t address) { ROL_Memory(address); }), AddressingMode::ZeroPageX, 2,
        6
    };
    instruction_table[0x38] = {std::function<void()>([this]() { SEC(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x39] = {
        std::function<void(uint8_t)>([this](uint8_t value) { AND(value); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0x3D] = {
        std::function<void(uint8_t)>([this](uint8_t value) { AND(value); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0x3E] = {
        std::function<void(uint16_t)>([this](uint16_t address) { ROL_Memory(address); }), AddressingMode::AbsoluteX, 3,
        7
    };

    instruction_table[0x40] = {std::function<void()>([this]() { RTI(); }), AddressingMode::Implied, 1, 6};
    instruction_table[0x41] = {
        std::function<void(uint8_t)>([this](uint8_t value) { EOR(value); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0x45] = {
        std::function<void(uint8_t)>([this](uint8_t value) { EOR(value); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x46] = {
        std::function<void(uint16_t)>([this](uint16_t address) { LSR_Memory(address); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0x48] = {std::function<void()>([this]() { PHA(); }), AddressingMode::Implied, 1, 3};
    instruction_table[0x49] = {
        std::function<void(uint8_t)>([this](uint8_t value) { EOR(value); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0x4A] = {std::function<void()>([this]() { LSR_Accumulator(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x4C] = {
        std::function<void(uint16_t)>([this](uint16_t address) { JMP(address); }), AddressingMode::Absolute, 3, 3
    };
    instruction_table[0x4D] = {
        std::function<void(uint8_t)>([this](uint8_t value) { EOR(value); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x4E] = {
        std::function<void(uint16_t)>([this](uint16_t address) { LSR_Memory(address); }), AddressingMode::Absolute, 3, 6
    };

    instruction_table[0x50] = {
        std::function<void(uint8_t)>([this](uint8_t value) { BVC(value); }), AddressingMode::Relative, 2, 2
    };
    instruction_table[0x51] = {
        std::function<void(uint8_t)>([this](uint8_t value) { EOR(value); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0x55] = {
        std::function<void(uint8_t)>([this](uint8_t value) { EOR(value); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x56] = {
        std::function<void(uint16_t)>([this](uint16_t address) { LSR_Memory(address); }), AddressingMode::ZeroPageX, 2,
        6
    };
    instruction_table[0x58] = {std::function<void()>([this]() { CLI(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x59] = {
        std::function<void(uint8_t)>([this](uint8_t value) { EOR(value); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0x5D] = {
        std::function<void(uint8_t)>([this](uint8_t value) { EOR(value); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0x5E] = {
        std::function<void(uint16_t)>([this](uint16_t address) { LSR_Memory(address); }), AddressingMode::AbsoluteX, 3,
        7
    };

    instruction_table[0x60] = {std::function<void()>([this]() { RTS(); }), AddressingMode::Implied, 1, 6};
    instruction_table[0x61] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ADC(value); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0x65] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ADC(value); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x66] = {
        std::function<void(uint16_t)>([this](uint16_t address) { ROR_Memory(address); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0x68] = {std::function<void()>([this]() { PLA(); }), AddressingMode::Implied, 1, 4};
    instruction_table[0x69] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ADC(value); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0x6A] = {std::function<void()>([this]() { ROR_Accumulator(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x6C] = {
        std::function<void(uint16_t)>([this](uint16_t address) { JMP(address); }), AddressingMode::Indirect, 3, 5
    };
    instruction_table[0x6D] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ADC(value); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x6E] = {
        std::function<void(uint16_t)>([this](uint16_t address) { ROR_Memory(address); }), AddressingMode::Absolute, 3, 6
    };

    instruction_table[0x70] = {
        std::function<void(uint8_t)>([this](uint8_t value) { BVS(value); }), AddressingMode::Relative, 2, 2
    };
    instruction_table[0x71] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ADC(value); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0x75] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ADC(value); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x76] = {
        std::function<void(uint16_t)>([this](uint16_t address) { ROR_Memory(address); }), AddressingMode::ZeroPageX, 2,
        6
    };
    instruction_table[0x78] = {std::function<void()>([this]() { SEI(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x79] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ADC(value); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0x7D] = {
        std::function<void(uint8_t)>([this](uint8_t value) { ADC(value); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0x7E] = {
        std::function<void(uint16_t)>([this](uint16_t address) { ROR_Memory(address); }), AddressingMode::AbsoluteX, 3,
        7
    };

    instruction_table[0x81] = {
        std::function<void(uint16_t)>([this](uint16_t address) { STA(address); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0x84] = {
        std::function<void(uint16_t)>([this](uint16_t address) { STY(address); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x85] = {
        std::function<void(uint16_t)>([this](uint16_t address) { STA(address); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x86] = {
        std::function<void(uint16_t)>([this](uint16_t address) { STX(address); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0x88] = {std::function<void()>([this]() { DEY(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x8A] = {std::function<void()>([this]() { TXA(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x8C] = {
        std::function<void(uint16_t)>([this](uint16_t address) { STY(address); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x8D] = {
        std::function<void(uint16_t)>([this](uint16_t address) { STA(address); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0x8E] = {
        std::function<void(uint16_t)>([this](uint16_t address) { STX(address); }), AddressingMode::Absolute, 3, 4
    };

    instruction_table[0x90] = {
        std::function<void(uint8_t)>([this](uint8_t value) { BCC(value); }), AddressingMode::Relative, 2, 2
    };
    instruction_table[0x91] = {
        std::function<void(uint16_t)>([this](uint16_t address) { STA(address); }), AddressingMode::IndirectIndexed, 2, 6
    };
    instruction_table[0x94] = {
        std::function<void(uint16_t)>([this](uint16_t address) { STY(address); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x95] = {
        std::function<void(uint16_t)>([this](uint16_t address) { STA(address); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0x96] = {
        std::function<void(uint16_t)>([this](uint16_t address) { STX(address); }), AddressingMode::ZeroPageY, 2, 4
    };
    instruction_table[0x98] = {std::function<void()>([this]() { TYA(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x99] = {
        std::function<void(uint16_t)>([this](uint16_t address) { STA(address); }), AddressingMode::AbsoluteY, 3, 5
    };
    instruction_table[0x9A] = {std::function<void()>([this]() { TXS(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0x9D] = {
        std::function<void(uint16_t)>([this](uint16_t address) { STA(address); }), AddressingMode::AbsoluteX, 3, 5
    };

    instruction_table[0xA0] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDY(value); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xA1] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDA(value); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0xA2] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDX(value); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xA4] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDY(value); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xA5] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDA(value); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xA6] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDX(value); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xA8] = {std::function<void()>([this]() { TAY(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xA9] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDA(value); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xAA] = {std::function<void()>([this]() { TAX(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xAC] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDY(value); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xAD] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDA(value); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xAE] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDX(value); }), AddressingMode::Absolute, 3, 4
    };

    instruction_table[0xB0] = {
        std::function<void(uint8_t)>([this](uint8_t value) { BCS(value); }), AddressingMode::Relative, 2, 2
    };
    instruction_table[0xB1] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDA(value); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0xB4] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDY(value); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0xB5] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDA(value); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0xB6] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDX(value); }), AddressingMode::ZeroPageY, 2, 4
    };
    instruction_table[0xB8] = {std::function<void()>([this]() { CLV(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xB9] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDA(value); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0xBA] = {std::function<void()>([this]() { TSX(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xBC] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDY(value); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0xBD] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDA(value); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0xBE] = {
        std::function<void(uint8_t)>([this](uint8_t value) { LDX(value); }), AddressingMode::AbsoluteY, 3, 4, true
    };

    instruction_table[0xC0] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CPY(value); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xC1] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CMP(value); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0xC4] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CPY(value); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xC5] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CMP(value); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xC6] = {
        std::function<void(uint16_t)>([this](uint16_t address) { DEC(address); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0xC8] = {std::function<void()>([this]() { INY(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xC9] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CMP(value); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xCA] = {std::function<void()>([this]() { DEX(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xCC] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CPY(value); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xCD] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CMP(value); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xCE] = {
        std::function<void(uint16_t)>([this](uint16_t address) { DEC(address); }), AddressingMode::Absolute, 3, 6
    };

    instruction_table[0xD0] = {
        std::function<void(uint8_t)>([this](uint8_t value) { BNE(value); }), AddressingMode::Relative, 2, 2
    };
    instruction_table[0xD1] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CMP(value); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0xD5] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CMP(value); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0xD6] = {
        std::function<void(uint16_t)>([this](uint16_t address) { DEC(address); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0xD8] = {std::function<void()>([this]() { CLD(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xD9] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CMP(value); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0xDD] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CMP(value); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0xDE] = {
        std::function<void(uint16_t)>([this](uint16_t address) { DEC(address); }), AddressingMode::AbsoluteX, 3, 7
    };

    instruction_table[0xE0] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CPX(value); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xE1] = {
        std::function<void(uint8_t)>([this](uint8_t value) { SBC(value); }), AddressingMode::IndexedIndirect, 2, 6
    };
    instruction_table[0xE4] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CPX(value); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xE5] = {
        std::function<void(uint8_t)>([this](uint8_t value) { SBC(value); }), AddressingMode::ZeroPage, 2, 3
    };
    instruction_table[0xE6] = {
        std::function<void(uint16_t)>([this](uint16_t address) { INC(address); }), AddressingMode::ZeroPage, 2, 5
    };
    instruction_table[0xE8] = {std::function<void()>([this]() { INX(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xE9] = {
        std::function<void(uint8_t)>([this](uint8_t value) { SBC(value); }), AddressingMode::Immediate, 2, 2
    };
    instruction_table[0xEA] = {std::function<void()>([this]() { NOP(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xEC] = {
        std::function<void(uint8_t)>([this](uint8_t value) { CPX(value); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xED] = {
        std::function<void(uint8_t)>([this](uint8_t value) { SBC(value); }), AddressingMode::Absolute, 3, 4
    };
    instruction_table[0xEE] = {
        std::function<void(uint16_t)>([this](uint16_t address) { INC(address); }), AddressingMode::Absolute, 3, 6
    };

    instruction_table[0xF0] = {
        std::function<void(uint8_t)>([this](uint8_t value) { BEQ(value); }), AddressingMode::Relative, 2, 2
    };
    instruction_table[0xF1] = {
        std::function<void(uint8_t)>([this](uint8_t value) { SBC(value); }), AddressingMode::IndirectIndexed, 2, 5, true
    };
    instruction_table[0xF5] = {
        std::function<void(uint8_t)>([this](uint8_t value) { SBC(value); }), AddressingMode::ZeroPageX, 2, 4
    };
    instruction_table[0xF6] = {
        std::function<void(uint16_t)>([this](uint16_t address) { INC(address); }), AddressingMode::ZeroPageX, 2, 6
    };
    instruction_table[0xF8] = {std::function<void()>([this]() { SED(); }), AddressingMode::Implied, 1, 2};
    instruction_table[0xF9] = {
        std::function<void(uint8_t)>([this](uint8_t value) { SBC(value); }), AddressingMode::AbsoluteY, 3, 4, true
    };
    instruction_table[0xFD] = {
        std::function<void(uint8_t)>([this](uint8_t value) { SBC(value); }), AddressingMode::AbsoluteX, 3, 4, true
    };
    instruction_table[0xFE] = {
        std::function<void(uint16_t)>([this](uint16_t address) { INC(address); }), AddressingMode::AbsoluteX, 3, 7
    };
}

void Cpu::setNegativeFlag(bool value) {
    if (value)
        registers.p |= 0x80; // Set Negative Flag
    else
        registers.p &= ~0x80; // Clear Negative Flag
}

void Cpu::setZeroFlag(bool value) {
    if (value)
        registers.p |= 0x02; // Set Zero Flag
    else
        registers.p &= ~0x02; // Clear Zero Flag
}

void Cpu::setCarryFlag(bool value) {
    if (value)
        registers.p |= 0x01; // Set Carry Flag
    else
        registers.p &= ~0x01; // Clear Carry Flag
}

void Cpu::setOverflowFlag(bool value) {
    if (value)
        registers.p |= 0x40; // Set Overflow Flag
    else
        registers.p &= ~0x40; // Clear Overflow Flag
}

void Cpu::setDecimalFlag(bool value) {
    if (value)
        registers.p |= 0x08; // Set Decimal Flag
    else
        registers.p &= ~0x08; // Clear Decimal Flag
}

void Cpu::setInterruptDisableFlag(bool value) {
    if (value)
        registers.p |= 0x04; // Set Interrupt Disable Flag
    else
        registers.p &= ~0x04; // Clear Interrupt Disable Flag
}

void Cpu::writeMemory(uint16_t address, uint8_t value) {
    memory.bus[address] = value;
}


uint8_t Cpu::readMemory(uint16_t address) {
    return memory.bus[address];
}

// All of the official instructions are defined in the functions below:
void Cpu::ADC(uint8_t value) {
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
void Cpu::AND(uint8_t value) {
    registers.a &= value;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.a == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.a & 0x80) != 0);
}

// Arithmetic Shift Left (ASL) - shifts bits left and sets flags accordingly
void Cpu::ASL_Accumulator() {
    uint8_t &a = registers.a;
    setCarryFlag((a & 0x80) != 0);
    a <<= 1;
    setZeroFlag(a == 0);
    setNegativeFlag((a & 0x80) != 0);
}


void Cpu::ASL_Memory(uint16_t address) {
    uint8_t value = readMemory(address);
    setCarryFlag((value & 0x80) != 0);
    value <<= 1;
    setZeroFlag(value == 0);
    setNegativeFlag((value & 0x80) != 0);
    writeMemory(address, value);
}

// Branch if Carry Clear (BCC) - branches if the Carry Flag is clear
void Cpu::BCC(uint8_t value) {
    int8_t offset = static_cast<int8_t>(value); // Convert to signed 8-bit integer
    // Branch if Carry Clear
    if (!(registers.p & 0x01)) {
        uint16_t old_pc = registers.pc + 2;
        registers.pc += offset;
        total_cycles += 1 + ((old_pc & 0xFF00) != (registers.pc & 0xFF00) ? 1 : 0);
        registers.pc += 2;
    } else {
        registers.pc += 2;
    }
}

// Branch if Carry Set (BCS) - branches if the Carry Flag is set
void Cpu::BCS(uint8_t value) {
    auto offset = static_cast<int8_t>(value); // Convert to signed 8-bit integer
    // Branch if Carry Set
    if (registers.p & 0x01) {
        uint16_t old_pc = registers.pc + 2;
        registers.pc += offset;
        total_cycles += 1 + ((old_pc & 0xFF00) != (registers.pc & 0xFF00) ? 1 : 0);
        registers.pc += 2;
    } else {
        registers.pc += 2;
    }
}

void Cpu::BEQ(uint8_t value) {
    auto offset = static_cast<int8_t>(value); // Convert to signed 8-bit integer
    // Branch if Equal (Zero Flag is set)
    if (registers.p & 0x02) {
        uint16_t old_pc = registers.pc + 2;
        registers.pc += offset;
        total_cycles += 1 + ((old_pc & 0xFF00) != (registers.pc & 0xFF00) ? 1 : 0);
        registers.pc += 2;
    } else {
        registers.pc += 2;
    }
}

void Cpu::BIT(uint8_t value) {
    // Bit Test - sets flags based on the value
    setZeroFlag((registers.a & value) == 0);
    setNegativeFlag((value & 0x80) != 0);
    setOverflowFlag((value & 0x40) != 0);
}

void Cpu::BMI(uint8_t value) {
    auto offset = static_cast<int8_t>(value); // Convert to signed 8-bit integer
    // Branch if Minus (Negative Flag is set)
    if (registers.p & 0x80) {
        uint16_t old_pc = registers.pc + 2;
        registers.pc += offset;
        total_cycles += 1 + ((old_pc & 0xFF00) != (registers.pc & 0xFF00) ? 1 : 0);
        registers.pc += 2;
    } else {
        registers.pc += 2;
    }
}

void Cpu::BNE(uint8_t value) {
    int8_t offset = static_cast<int8_t>(value); // Convert to signed 8-bit integer
    // Branch if Not Equal (Zero Flag is clear)
    if (!(registers.p & 0x02)) {
        uint16_t old_pc = registers.pc + 2;
        registers.pc += offset;
        total_cycles += 1 + ((old_pc & 0xFF00) != (registers.pc & 0xFF00) ? 1 : 0);
        registers.pc += 2;
    } else {
        registers.pc += 2;
    }
}

void Cpu::BPL(uint8_t value) {
    int8_t offset = static_cast<int8_t>(value); // Convert to signed 8-bit integer
    // Branch if Positive (Negative Flag is clear)
    if (!(registers.p & 0x80)) {
        uint16_t old_pc = registers.pc + 2;
        registers.pc += offset;
        total_cycles += 1 + ((old_pc & 0xFF00) != (registers.pc & 0xFF00) ? 1 : 0);
        registers.pc += 2;
    } else {
        registers.pc += 2;
    }
}

void Cpu::BRK() {
    // BRK este o instrucțiune de 1 byte, dar incrementează PC cu 2
    registers.pc++;

    // Push PC pe stivă
    memory.push16(registers.sp, registers.pc);

    // Push status register cu B flag setat
    memory.push8(registers.sp, registers.p | 0x10);

    // Set Interrupt Disable flag
    setInterruptDisableFlag(true);

    // Load PC from IRQ vector
    registers.pc = readMemory(0xFFFE) | (readMemory(0xFFFF) << 8);
}


void Cpu::BVC(uint8_t value) {
    int8_t offset = static_cast<int8_t>(value); // Convert to signed 8-bit integer
    // Branch if Overflow Clear (Overflow Flag is clear)
    if (!(registers.p & 0x40)) {
        uint16_t old_pc = registers.pc + 2;
        registers.pc += offset;
        total_cycles += 1 + ((old_pc & 0xFF00) != (registers.pc & 0xFF00) ? 1 : 0);
        registers.pc += 2;
    } else {
        registers.pc += 2;
    }
}

void Cpu::BVS(uint8_t value) {
    int8_t offset = static_cast<int8_t>(value); // Convert to signed 8-bit integer
    // Branch if Overflow Set (Overflow Flag is set)
    if (registers.p & 0x40) {
        uint16_t old_pc = registers.pc + 2;
        registers.pc += offset;
        total_cycles += 1 + ((old_pc & 0xFF00) != (registers.pc & 0xFF00) ? 1 : 0);
        registers.pc += 2;
    } else {
        registers.pc += 2;
    }
}

void Cpu::CLC() {
    // Clear Carry Flag
    setCarryFlag(false);
}

void Cpu::CLD() {
    // Clear Decimal Flag
    setDecimalFlag(false);
}

void Cpu::CLI() {
    // Clear Interrupt Disable Flag
    setInterruptDisableFlag(false);
}

void Cpu::CLV() {
    setOverflowFlag(false);
}

void Cpu::CMP(uint8_t value) {
    // Compare - sets flags based on the comparison
    uint8_t a = registers.a;
    uint8_t result = a - value;

    setCarryFlag(a >= value);
    setZeroFlag(a == value);
    setNegativeFlag(result & 0x80);
}

void Cpu::CPX(uint8_t value) {
    // Compare X Register
    uint8_t x = registers.x;
    uint8_t result = x - value;

    setCarryFlag(x >= value);
    setZeroFlag(x == value);
    setNegativeFlag(result & 0x80);
}

void Cpu::CPY(uint8_t value) {
    // Compare Y Register
    uint8_t y = registers.y;
    uint8_t result = y - value;

    setCarryFlag(y >= value);
    setZeroFlag(y == value);
    setNegativeFlag(result & 0x80);
}

// Decrement Memory (DEC) - decrements the value at the specified address
void Cpu::DEC(uint16_t address) {
    uint8_t value = readMemory(address);
    value--;
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
    writeMemory(address, value);
}


void Cpu::DEX() {
    // Decrement X Register
    registers.x--;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.x == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.x & 0x80) != 0);
}

void Cpu::DEY() {
    // Decrement Y Register
    registers.y--;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.y == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.y & 0x80) != 0);
}

void Cpu::EOR(uint8_t value) {
    // Exclusive OR - performs bitwise XOR with the accumulator
    registers.a ^= value;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.a == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.a & 0x80) != 0);
}

// Increment Memory - increments the value at the specified address
void Cpu::INC(uint16_t address) {
    uint8_t value = readMemory(address);
    value++;
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
    writeMemory(address, value);
}


void Cpu::INX() {
    // Increment X Register
    registers.x++;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.x == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.x & 0x80) != 0);
}

void Cpu::INY() {
    // Increment Y Register
    registers.y++;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.y == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.y & 0x80) != 0);
}

void Cpu::JMP(uint16_t address) {
    // Jump to a specific address
    registers.pc = address;
}

void Cpu::JSR(const uint16_t address) {
    memory.push16(registers.sp, registers.pc + 2);
    registers.pc = address;
}

void Cpu::LDA(uint8_t value) {
    // Load Accumulator
    registers.a = value;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.a == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.a & 0x80) != 0);
}

void Cpu::LDX(uint8_t value) {
    // Load X Register
    registers.x = value;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.x == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.x & 0x80) != 0);
}

void Cpu::LDY(uint8_t value) {
    // Load Y Register
    registers.y = value;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.y == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.y & 0x80) != 0);
}

// Logical Shift Right (LSR) - shifts bits right and sets flags accordingly
void Cpu::LSR_Accumulator() {
    uint8_t &a = registers.a;
    setCarryFlag(a & 0x01); // Bit 0 pleacă
    a >>= 1;
    setZeroFlag(a == 0);
    setNegativeFlag(false); // LSR setează întotdeauna N = 0
}

void Cpu::LSR_Memory(uint16_t address) {
    uint8_t value = readMemory(address);
    setCarryFlag(value & 0x01);
    value >>= 1;
    setZeroFlag(value == 0);
    setNegativeFlag(false);
    writeMemory(address, value);
}


void Cpu::NOP() {
    // No Operation - does nothing
    // As useful as me
}

void Cpu::ORA(uint8_t value) {
    // Logical OR - performs bitwise OR with the accumulator
    registers.a |= value;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.a == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.a & 0x80) != 0);
}

void Cpu::PHA() {
    memory.push8(registers.sp, registers.a);
}

// repaired / bit 4 (break) is set to 1 when pushed
// The wiki isn't too helpful
void Cpu::PHP() {
    memory.push8(registers.sp, registers.p | 0x30); // 0x30 = B flag + bit 5
}

void Cpu::PLA() {
    registers.a = memory.pop8(registers.sp);
    setZeroFlag(registers.a == 0);
    setNegativeFlag((registers.a & 0x80) != 0);
}

// TODO: Again, not too sure (~Mario)
void Cpu::PLP() {
    registers.p = (memory.pop8(registers.sp) & ~0x10) | 0x20;
}

// Rotate Left (ROL) - shifts bits left and wraps the leftmost bit to the right
void Cpu::ROL_Accumulator() {
    uint8_t &a = registers.a;
    bool oldCarry = (registers.p & 0x01) != 0; // Carry Flag (C)
    setCarryFlag(a & 0x80);
    a = (a << 1) | (oldCarry ? 1 : 0);
    setZeroFlag(a == 0);
    setNegativeFlag(a & 0x80);
}

void Cpu::ROL_Memory(uint16_t address) {
    uint8_t value = readMemory(address);
    bool oldCarry = (registers.p & 0x01) != 0; // Carry Flag (C)
    setCarryFlag(value & 0x80);
    value = (value << 1) | (oldCarry ? 1 : 0);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
    writeMemory(address, value);
}

// Rotate Right (ROR) - shifts bits right and wraps the rightmost bit to the left
void Cpu::ROR_Accumulator() {
    uint8_t &a = registers.a;
    bool oldCarry = (registers.p & 0x01) != 0; // Carry Flag (C)
    setCarryFlag(a & 0x01);
    a = (a >> 1) | (oldCarry ? 0x80 : 0);
    setZeroFlag(a == 0);
    setNegativeFlag(a & 0x80);
}

void Cpu::ROR_Memory(uint16_t address) {
    uint8_t value = readMemory(address);
    bool oldCarry = (registers.p & 0x01) != 0; // Carry Flag (C)
    setCarryFlag(value & 0x01);
    value = (value >> 1) | (oldCarry ? 0x80 : 0);
    setZeroFlag(value == 0);
    setNegativeFlag(value & 0x80);
    writeMemory(address, value);
}

void Cpu::RTI() {
    // Restaurează status register (ignoră B flag)
    registers.p = (memory.pop8(registers.sp) & ~0x10) | 0x20;

    // Restaurează PC
    registers.pc = memory.pop16(registers.sp);
}

void Cpu::RTS() {
    registers.pc = memory.pop16(registers.sp) + 1;
}

void Cpu::SBC(uint8_t value) {
    // Subtract with Carry - subtracts value and Carry Flag from the accumulator
    uint8_t a = registers.a;
    uint8_t carry = (registers.p & 0x01) ? 0 : 1;

    uint16_t result = a - value - carry;

    // === Set Flags ===

    // Carry Flag (C)
    setCarryFlag(result < 0x100);

    // Zero Flag (Z)
    setZeroFlag((result & 0xFF) == 0);

    // Negative Flag (N)
    setNegativeFlag((result & 0x80) != 0);

    // Overflow Flag (V)
    int overflow = ((a ^ result) & 0x80) && ((a ^ value) & 0x80);
    setOverflowFlag(overflow);

    // === Store result ===
    registers.a = result & 0xFF;
}

void Cpu::SEC() {
    // Set Carry Flag
    setCarryFlag(true);
}

void Cpu::SED() {
    // Set Decimal Flag
    setDecimalFlag(true);
}

void Cpu::SEI() {
    // Set Interrupt Disable Flag
    setInterruptDisableFlag(true);
}

void Cpu::STA(uint16_t address) {
    // Store Accumulator
    writeMemory(address, registers.a);
}

void Cpu::STX(uint16_t address) {
    // Store X Register
    writeMemory(address, registers.x);
}

void Cpu::STY(uint16_t address) {
    // Store Y Register
    writeMemory(address, registers.y);
}

void Cpu::TAX() {
    // Transfer Accumulator to X Register
    registers.x = registers.a;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.x == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.x & 0x80) != 0);
}

void Cpu::TAY() {
    // Transfer Accumulator to Y Register
    registers.y = registers.a;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.y == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.y & 0x80) != 0);
}

void Cpu::TSX() {
    // Transfer Stack Pointer to X Register
    registers.x = registers.sp;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.x == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.x & 0x80) != 0);
}

void Cpu::TXA() {
    // Transfer X Register to Accumulator
    registers.a = registers.x;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.a == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.a & 0x80) != 0);
}

void Cpu::TXS() {
    // Transfer X Register to Stack Pointer
    registers.sp = registers.x;
}

void Cpu::TYA() {
    // Transfer Y Register to Accumulator
    registers.a = registers.y;

    // === Set Flags ===

    // Zero Flag (Z)
    setZeroFlag(registers.a == 0);

    // Negative Flag (N)
    setNegativeFlag((registers.a & 0x80) != 0);
}

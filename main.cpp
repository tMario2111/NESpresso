#include "src/Cpu.hpp"
#include "src/Memory.hpp"

#include <spdlog/spdlog.h>
#include <fstream>

// TODO: Delete this
void createRomFile()
{
    std::ofstream file{"../simple_rom.nes", std::ios::binary};
    file << static_cast<char>(0xa0);
    file << static_cast<char>(0x0f);
    file.close();
}

int main()
{
    spdlog::info("Welcome to NESpresso!");
    Memory::instance().loadROM("../xplusy.nes");
    auto &cpu = Cpu::instance();
    while (Memory::instance().bus[cpu.registers.pc] != 0x00)
        cpu.executeInstruction();
    spdlog::info("A: {}", cpu.registers.a);
    spdlog::info("X: {}", cpu.registers.x);
    spdlog::info("Y: {}", cpu.registers.y);
    return 0;
}

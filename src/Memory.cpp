#include "Memory.hpp"

Memory &Memory::instance()
{
    static Memory memory;
    return memory;
}

// TODO: This is incomplete, only for testing purposes atm
void Memory::loadROM(const std::string &filename)
{
    std::ifstream file{filename, std::ios::binary};
    if (!file)
    {
        spdlog::error("Failed to open ROM file {}", filename);
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::memcpy(bus.data() + ROM_BOTTOM, buffer.str().data(), buffer.str().size());
    spdlog::info("ROM \"{}\" loaded", filename);
    file.close();
}

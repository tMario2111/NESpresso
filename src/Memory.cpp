#include "Memory.hpp"

Memory &Memory::instance()
{
    static Memory memory;
    return memory;
}

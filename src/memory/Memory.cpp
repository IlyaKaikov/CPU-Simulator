#include "memory/Memory.hpp"

#include <stdexcept>
#include <string>

namespace sim {

void Memory::reset()
{
    data_.fill(0);
}

std::uint8_t Memory::readByte(std::uint32_t address) const
{
    ensureRange(address, 1);
    return data_.at(address);
}

void Memory::writeByte(std::uint32_t address, std::uint8_t value)
{
    ensureRange(address, 1);
    data_.at(address) = value;
}

EncodedInstruction Memory::readInstruction(std::uint32_t address) const
{
    ensureRange(address, instruction_size);

    const auto low = static_cast<std::uint16_t>(data_.at(address + 2));
    const auto high = static_cast<std::uint16_t>(data_.at(address + 3)) << 8U;
    const auto b = static_cast<std::int16_t>(low | high);

    return EncodedInstruction{
        data_.at(address),
        data_.at(address + 1),
        b,
    };
}

void Memory::writeInstruction(std::uint32_t address, const EncodedInstruction& instruction)
{
    ensureRange(address, instruction_size);

    const auto b = static_cast<std::uint16_t>(instruction.b);
    data_.at(address) = instruction.opcode;
    data_.at(address + 1) = instruction.a;
    data_.at(address + 2) = static_cast<std::uint8_t>(b & 0x00ffU);
    data_.at(address + 3) = static_cast<std::uint8_t>((b >> 8U) & 0x00ffU);
}

void Memory::ensureRange(std::uint32_t address, std::size_t length) const
{
    if (address > size || length > size - address) {
        throw std::out_of_range("memory access out of range at address " + std::to_string(address));
    }
}

}

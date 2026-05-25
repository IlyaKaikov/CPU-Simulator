#pragma once

#include "isa/Instruction.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace sim {

class Memory {
public:
    static constexpr std::size_t size = 64 * 1024;
    static constexpr std::size_t instruction_size = 4;

    void reset();

    [[nodiscard]] std::uint8_t readByte(std::uint32_t address) const;
    void writeByte(std::uint32_t address, std::uint8_t value);

    [[nodiscard]] std::int32_t readInt32(std::uint32_t address) const;
    void writeInt32(std::uint32_t address, std::int32_t value);

    [[nodiscard]] EncodedInstruction readInstruction(std::uint32_t address) const;
    void writeInstruction(std::uint32_t address, const EncodedInstruction& instruction);

private:
    void ensureRange(std::uint32_t address, std::size_t length) const;

    std::array<std::uint8_t, size> data_{};
};

}

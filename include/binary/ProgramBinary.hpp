#pragma once

#include "isa/Instruction.hpp"
#include "memory/Memory.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace sim {

class BinaryFormatError : public std::runtime_error {
public:
    explicit BinaryFormatError(const std::string& message);
};

namespace program_binary {

inline constexpr std::array<std::uint8_t, 4> magic{'C', 'S', 'I', 'M'};
inline constexpr std::uint16_t version = 1;
inline constexpr std::size_t header_size = 16;
inline constexpr std::size_t instruction_size = Memory::instruction_size;

}

[[nodiscard]] std::vector<std::uint8_t> writeProgramBinary(std::span<const EncodedInstruction> program);
[[nodiscard]] std::vector<EncodedInstruction> readProgramBinary(std::span<const std::uint8_t> bytes);

}

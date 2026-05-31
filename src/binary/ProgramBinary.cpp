#include "binary/ProgramBinary.hpp"

#include <limits>

namespace sim {

namespace {

void appendUInt16(std::vector<std::uint8_t>& bytes, std::uint16_t value)
{
    bytes.push_back(static_cast<std::uint8_t>(value & 0x00ffU));
    bytes.push_back(static_cast<std::uint8_t>((value >> 8U) & 0x00ffU));
}

void appendUInt32(std::vector<std::uint8_t>& bytes, std::uint32_t value)
{
    bytes.push_back(static_cast<std::uint8_t>(value & 0x000000ffU));
    bytes.push_back(static_cast<std::uint8_t>((value >> 8U) & 0x000000ffU));
    bytes.push_back(static_cast<std::uint8_t>((value >> 16U) & 0x000000ffU));
    bytes.push_back(static_cast<std::uint8_t>((value >> 24U) & 0x000000ffU));
}

void appendInstruction(std::vector<std::uint8_t>& bytes, const EncodedInstruction& instruction)
{
    bytes.push_back(instruction.opcode);
    bytes.push_back(instruction.a);
    appendUInt16(bytes, static_cast<std::uint16_t>(instruction.b));
}

}

BinaryFormatError::BinaryFormatError(const std::string& message)
    : std::runtime_error(message)
{
}

std::vector<std::uint8_t> writeProgramBinary(std::span<const EncodedInstruction> program)
{
    if (program.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw BinaryFormatError("program has too many instructions");
    }

    std::vector<std::uint8_t> bytes;
    bytes.reserve(program_binary::header_size + program.size() * program_binary::instruction_size);

    bytes.insert(bytes.end(), program_binary::magic.begin(), program_binary::magic.end());
    appendUInt16(bytes, program_binary::version);
    appendUInt16(bytes, static_cast<std::uint16_t>(program_binary::header_size));
    appendUInt32(bytes, static_cast<std::uint32_t>(program.size()));
    appendUInt32(bytes, 0);

    for (const auto& instruction : program) {
        appendInstruction(bytes, instruction);
    }

    return bytes;
}

std::vector<EncodedInstruction> readProgramBinary(std::span<const std::uint8_t> bytes)
{
    (void)bytes;
    throw BinaryFormatError("binary reader is not implemented yet");
}

}

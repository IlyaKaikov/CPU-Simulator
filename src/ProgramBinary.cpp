#include "ProgramBinary.hpp"

#include <algorithm>
#include <fstream>
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

std::uint16_t readUInt16(std::span<const std::uint8_t> bytes, std::size_t offset)
{
    const auto low = static_cast<std::uint16_t>(bytes[offset]);
    const auto high = static_cast<std::uint16_t>(bytes[offset + 1]) << 8U;
    return static_cast<std::uint16_t>(low | high);
}

std::uint32_t readUInt32(std::span<const std::uint8_t> bytes, std::size_t offset)
{
    const auto byte0 = static_cast<std::uint32_t>(bytes[offset]);
    const auto byte1 = static_cast<std::uint32_t>(bytes[offset + 1]) << 8U;
    const auto byte2 = static_cast<std::uint32_t>(bytes[offset + 2]) << 16U;
    const auto byte3 = static_cast<std::uint32_t>(bytes[offset + 3]) << 24U;
    return byte0 | byte1 | byte2 | byte3;
}

EncodedInstruction readInstruction(std::span<const std::uint8_t> bytes, std::size_t offset)
{
    return EncodedInstruction{
        bytes[offset],
        bytes[offset + 1],
        static_cast<std::int16_t>(readUInt16(bytes, offset + 2)),
    };
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
    if (bytes.size() < program_binary::header_size) {
        throw BinaryFormatError("binary is shorter than the header");
    }

    if (!std::equal(program_binary::magic.begin(), program_binary::magic.end(), bytes.begin())) {
        throw BinaryFormatError("invalid binary magic");
    }

    const auto version = readUInt16(bytes, 4);
    if (version != program_binary::version) {
        throw BinaryFormatError("unsupported binary version");
    }

    const auto headerSize = readUInt16(bytes, 6);
    if (headerSize != program_binary::header_size) {
        throw BinaryFormatError("unsupported binary header size");
    }

    const auto instructionCount = readUInt32(bytes, 8);
    const auto entryPoint = readUInt32(bytes, 12);
    if (entryPoint != 0) {
        throw BinaryFormatError("unsupported binary entry point");
    }

    const auto expectedSize = program_binary::header_size +
        static_cast<std::size_t>(instructionCount) * program_binary::instruction_size;
    if (bytes.size() != expectedSize) {
        throw BinaryFormatError("binary size does not match instruction count");
    }

    std::vector<EncodedInstruction> program;
    program.reserve(instructionCount);

    auto offset = program_binary::header_size;
    for (std::uint32_t index = 0; index < instructionCount; ++index) {
        program.push_back(readInstruction(bytes, offset));
        offset += program_binary::instruction_size;
    }

    return program;
}

void writeProgramBinaryFile(const std::filesystem::path& path, std::span<const EncodedInstruction> program)
{
    const auto bytes = writeProgramBinary(program);

    std::ofstream output(path, std::ios::binary);
    if (!output) {
        throw BinaryFormatError("failed to open binary file for writing: " + path.string());
    }

    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!output) {
        throw BinaryFormatError("failed to write binary file: " + path.string());
    }
}

std::vector<EncodedInstruction> readProgramBinaryFile(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw BinaryFormatError("failed to open binary file for reading: " + path.string());
    }

    std::vector<std::uint8_t> bytes;
    char byte = 0;
    while (input.get(byte)) {
        bytes.push_back(static_cast<std::uint8_t>(static_cast<unsigned char>(byte)));
    }

    if (!input.eof()) {
        throw BinaryFormatError("failed to read binary file: " + path.string());
    }

    return readProgramBinary(bytes);
}

}

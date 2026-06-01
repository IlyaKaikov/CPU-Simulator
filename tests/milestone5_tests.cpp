#include "binary/ProgramBinary.hpp"
#include "isa/Instruction.hpp"
#include "memory/Memory.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

bool contains(const std::string& text, const std::string& expected)
{
    return text.find(expected) != std::string::npos;
}

void exposes_binary_format_constants()
{
    static_assert(sim::program_binary::magic.size() == 4);
    static_assert(sim::program_binary::version == 1);
    static_assert(sim::program_binary::header_size == 16);
    static_assert(sim::program_binary::instruction_size == sim::Memory::instruction_size);

    assert(sim::program_binary::magic.at(0) == static_cast<std::uint8_t>('C'));
    assert(sim::program_binary::magic.at(1) == static_cast<std::uint8_t>('S'));
    assert(sim::program_binary::magic.at(2) == static_cast<std::uint8_t>('I'));
    assert(sim::program_binary::magic.at(3) == static_cast<std::uint8_t>('M'));
}

void reports_binary_format_errors()
{
    bool threw = false;

    try {
        throw sim::BinaryFormatError("bad binary");
    } catch (const std::runtime_error& error) {
        threw = true;
        assert(contains(error.what(), "bad binary"));
    }

    assert(threw);
}

void expectBinaryFormatError(const std::vector<std::uint8_t>& bytes, const std::string& expected)
{
    bool threw = false;

    try {
        const auto program = sim::readProgramBinary(bytes);
        (void)program;
    } catch (const sim::BinaryFormatError& error) {
        threw = true;
        assert(contains(error.what(), expected));
    }

    assert(threw);
}

bool sameInstruction(const sim::EncodedInstruction& left, const sim::EncodedInstruction& right)
{
    return left.opcode == right.opcode && left.a == right.a && left.b == right.b;
}

void assertProgramEquals(
    const std::vector<sim::EncodedInstruction>& actual,
    const std::vector<sim::EncodedInstruction>& expected)
{
    assert(actual.size() == expected.size());
    for (std::size_t index = 0; index < expected.size(); ++index) {
        assert(sameInstruction(actual.at(index), expected.at(index)));
    }
}

void writes_empty_program_header()
{
    const auto bytes = sim::writeProgramBinary(std::vector<sim::EncodedInstruction>{});

    assert(bytes.size() == sim::program_binary::header_size);
    assert(bytes.at(0) == static_cast<std::uint8_t>('C'));
    assert(bytes.at(1) == static_cast<std::uint8_t>('S'));
    assert(bytes.at(2) == static_cast<std::uint8_t>('I'));
    assert(bytes.at(3) == static_cast<std::uint8_t>('M'));
    assert(bytes.at(4) == 0x01);
    assert(bytes.at(5) == 0x00);
    assert(bytes.at(6) == 0x10);
    assert(bytes.at(7) == 0x00);
    assert(bytes.at(8) == 0x00);
    assert(bytes.at(9) == 0x00);
    assert(bytes.at(10) == 0x00);
    assert(bytes.at(11) == 0x00);
    assert(bytes.at(12) == 0x00);
    assert(bytes.at(13) == 0x00);
    assert(bytes.at(14) == 0x00);
    assert(bytes.at(15) == 0x00);
}

void writes_instruction_body_after_header()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeMov(sim::Register::R1, -5),
        sim::makeLoad(sim::Register::R2, 0x8000),
        sim::makeHalt(),
    };

    const auto bytes = sim::writeProgramBinary(program);

    assert(bytes.size() == sim::program_binary::header_size + program.size() * sim::program_binary::instruction_size);
    assert(bytes.at(8) == 0x03);
    assert(bytes.at(9) == 0x00);
    assert(bytes.at(10) == 0x00);
    assert(bytes.at(11) == 0x00);

    auto offset = sim::program_binary::header_size;
    assert(bytes.at(offset + 0) == static_cast<std::uint8_t>(sim::OpCode::Mov));
    assert(bytes.at(offset + 1) == static_cast<std::uint8_t>(sim::Register::R1));
    assert(bytes.at(offset + 2) == 0xfb);
    assert(bytes.at(offset + 3) == 0xff);

    offset += sim::program_binary::instruction_size;
    assert(bytes.at(offset + 0) == static_cast<std::uint8_t>(sim::OpCode::Load));
    assert(bytes.at(offset + 1) == static_cast<std::uint8_t>(sim::Register::R2));
    assert(bytes.at(offset + 2) == 0x00);
    assert(bytes.at(offset + 3) == 0x80);

    offset += sim::program_binary::instruction_size;
    assert(bytes.at(offset + 0) == static_cast<std::uint8_t>(sim::OpCode::Halt));
    assert(bytes.at(offset + 1) == 0x00);
    assert(bytes.at(offset + 2) == 0x00);
    assert(bytes.at(offset + 3) == 0x00);
}

void reads_empty_program()
{
    const auto bytes = sim::writeProgramBinary(std::vector<sim::EncodedInstruction>{});
    const auto program = sim::readProgramBinary(bytes);

    assert(program.empty());
}

void reads_instruction_body()
{
    const std::vector<sim::EncodedInstruction> expected{
        sim::makeMov(sim::Register::R1, -5),
        sim::makeLoad(sim::Register::R2, 0x8000),
        sim::makeHalt(),
    };

    const auto bytes = sim::writeProgramBinary(expected);
    const auto actual = sim::readProgramBinary(bytes);

    assertProgramEquals(actual, expected);
}

void rejects_short_header()
{
    expectBinaryFormatError({'C', 'S', 'I'}, "shorter than the header");
}

void rejects_bad_magic()
{
    auto bytes = sim::writeProgramBinary(std::vector<sim::EncodedInstruction>{});
    bytes.at(0) = static_cast<std::uint8_t>('X');

    expectBinaryFormatError(bytes, "invalid binary magic");
}

void rejects_unsupported_version()
{
    auto bytes = sim::writeProgramBinary(std::vector<sim::EncodedInstruction>{});
    bytes.at(4) = 0x02;

    expectBinaryFormatError(bytes, "unsupported binary version");
}

void rejects_unsupported_header_size()
{
    auto bytes = sim::writeProgramBinary(std::vector<sim::EncodedInstruction>{});
    bytes.at(6) = 0x14;

    expectBinaryFormatError(bytes, "unsupported binary header size");
}

void rejects_unsupported_entry_point()
{
    auto bytes = sim::writeProgramBinary(std::vector<sim::EncodedInstruction>{});
    bytes.at(12) = 0x04;

    expectBinaryFormatError(bytes, "unsupported binary entry point");
}

void rejects_body_size_mismatch()
{
    auto bytes = sim::writeProgramBinary(std::vector<sim::EncodedInstruction>{sim::makeHalt()});
    bytes.pop_back();

    expectBinaryFormatError(bytes, "binary size does not match instruction count");

    bytes = sim::writeProgramBinary(std::vector<sim::EncodedInstruction>{});
    bytes.push_back(0x00);

    expectBinaryFormatError(bytes, "binary size does not match instruction count");
}

void writes_and_reads_binary_file()
{
    const auto path = std::filesystem::temp_directory_path() / "cpu_sim_milestone5_roundtrip.bin";
    std::filesystem::remove(path);

    const std::vector<sim::EncodedInstruction> expected{
        sim::makeMov(sim::Register::R1, 42),
        sim::makeCall(3 * sim::Memory::instruction_size),
        sim::makeHalt(),
        sim::makeRet(),
    };

    sim::writeProgramBinaryFile(path, expected);
    const auto actual = sim::readProgramBinaryFile(path);

    assertProgramEquals(actual, expected);
    std::filesystem::remove(path);
}

void reports_missing_binary_file()
{
    bool threw = false;
    const auto path = std::filesystem::temp_directory_path() / "cpu_sim_milestone5_missing.bin";
    std::filesystem::remove(path);

    try {
        const auto program = sim::readProgramBinaryFile(path);
        (void)program;
    } catch (const sim::BinaryFormatError& error) {
        threw = true;
        assert(contains(error.what(), "failed to open binary file for reading"));
    }

    assert(threw);
}

int main()
{
    exposes_binary_format_constants();
    reports_binary_format_errors();
    writes_empty_program_header();
    writes_instruction_body_after_header();
    reads_empty_program();
    reads_instruction_body();
    rejects_short_header();
    rejects_bad_magic();
    rejects_unsupported_version();
    rejects_unsupported_header_size();
    rejects_unsupported_entry_point();
    rejects_body_size_mismatch();
    writes_and_reads_binary_file();
    reports_missing_binary_file();
}

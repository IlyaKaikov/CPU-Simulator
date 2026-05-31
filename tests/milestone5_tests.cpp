#include "binary/ProgramBinary.hpp"
#include "isa/Instruction.hpp"
#include "memory/Memory.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <string>

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

int main()
{
    exposes_binary_format_constants();
    reports_binary_format_errors();
    writes_empty_program_header();
    writes_instruction_body_after_header();
}

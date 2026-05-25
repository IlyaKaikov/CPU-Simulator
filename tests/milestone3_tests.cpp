#include "isa/Instruction.hpp"
#include "memory/Memory.hpp"

#include <cassert>
#include <cstdint>
#include <stdexcept>

void reads_and_writes_signed_32_bit_words()
{
    sim::Memory memory;

    memory.writeInt32(0x8000, -123456789);

    assert(memory.readInt32(0x8000) == -123456789);
    assert(memory.readByte(0x8000) == 0xeb);
    assert(memory.readByte(0x8001) == 0x32);
    assert(memory.readByte(0x8002) == 0xa4);
    assert(memory.readByte(0x8003) == 0xf8);
}

void rejects_out_of_range_32_bit_words()
{
    sim::Memory memory;
    bool threw = false;

    try {
        memory.writeInt32(sim::Memory::size - 3, 1);
    } catch (const std::out_of_range&) {
        threw = true;
    }

    assert(threw);
}

void encodes_milestone3_instructions()
{
    {
        const auto instruction = sim::makeLoad(sim::Register::R2, 0x8000);
        assert(instruction.opcode == static_cast<std::uint8_t>(sim::OpCode::Load));
        assert(instruction.a == static_cast<std::uint8_t>(sim::Register::R2));
        assert(static_cast<std::uint16_t>(instruction.b) == 0x8000);
    }

    {
        const auto instruction = sim::makeStore(sim::Register::R1, 0x8000);
        assert(instruction.opcode == static_cast<std::uint8_t>(sim::OpCode::Store));
        assert(instruction.a == static_cast<std::uint8_t>(sim::Register::R1));
        assert(static_cast<std::uint16_t>(instruction.b) == 0x8000);
    }

    {
        const auto instruction = sim::makePush(sim::Register::R3);
        assert(instruction.opcode == static_cast<std::uint8_t>(sim::OpCode::Push));
        assert(instruction.a == static_cast<std::uint8_t>(sim::Register::R3));
        assert(instruction.b == 0);
    }

    {
        const auto instruction = sim::makePop(sim::Register::R4);
        assert(instruction.opcode == static_cast<std::uint8_t>(sim::OpCode::Pop));
        assert(instruction.a == static_cast<std::uint8_t>(sim::Register::R4));
        assert(instruction.b == 0);
    }

    {
        const auto instruction = sim::makeCall(0x1234);
        assert(instruction.opcode == static_cast<std::uint8_t>(sim::OpCode::Call));
        assert(instruction.a == 0);
        assert(static_cast<std::uint16_t>(instruction.b) == 0x1234);
    }

    {
        const auto instruction = sim::makeRet();
        assert(instruction.opcode == static_cast<std::uint8_t>(sim::OpCode::Ret));
        assert(instruction.a == 0);
        assert(instruction.b == 0);
    }
}

int main()
{
    reads_and_writes_signed_32_bit_words();
    rejects_out_of_range_32_bit_words();
    encodes_milestone3_instructions();
}

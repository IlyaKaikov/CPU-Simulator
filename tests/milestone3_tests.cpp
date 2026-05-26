#include "cpu/CPU.hpp"
#include "isa/Instruction.hpp"
#include "memory/Memory.hpp"

#include <array>
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

void executes_load_and_store()
{
    const std::array program{
        sim::makeMov(sim::Register::R1, 123),
        sim::makeStore(sim::Register::R1, 0x8000),
        sim::makeLoad(sim::Register::R2, 0x8000),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.halted());
    assert(cpu.reg(sim::Register::R1) == 123);
    assert(cpu.reg(sim::Register::R2) == 123);
    assert(cpu.memory().readInt32(0x8000) == 123);
    assert(cpu.sp() == sim::CPU::initial_sp);
}

void executes_push_and_pop()
{
    const std::array program{
        sim::makeMov(sim::Register::R1, 10),
        sim::makeMov(sim::Register::R2, 7),
        sim::makePush(sim::Register::R1),
        sim::makePush(sim::Register::R2),
        sim::makePop(sim::Register::R1),
        sim::makePop(sim::Register::R2),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.halted());
    assert(cpu.reg(sim::Register::R1) == 7);
    assert(cpu.reg(sim::Register::R2) == 10);
    assert(cpu.sp() == sim::CPU::initial_sp);
}

void executes_call_and_ret()
{
    constexpr auto functionAddress = static_cast<std::uint16_t>(3 * sim::Memory::instruction_size);
    const std::array program{
        sim::makeCall(functionAddress),
        sim::makeMov(sim::Register::R2, 7),
        sim::makeHalt(),
        sim::makeMov(sim::Register::R1, 42),
        sim::makeRet(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.halted());
    assert(cpu.reg(sim::Register::R1) == 42);
    assert(cpu.reg(sim::Register::R2) == 7);
    assert(cpu.pc() == 3 * sim::Memory::instruction_size);
    assert(cpu.sp() == sim::CPU::initial_sp);
}

void supports_recursive_calls()
{
    constexpr auto recursiveAddress = static_cast<std::uint16_t>(3 * sim::Memory::instruction_size);
    constexpr auto returnAddress = static_cast<std::uint16_t>(11 * sim::Memory::instruction_size);
    const std::array program{
        sim::makeMov(sim::Register::R1, 3),
        sim::makeCall(recursiveAddress),
        sim::makeHalt(),
        sim::makeCmp(sim::Register::R1, sim::Register::R0),
        sim::makeJe(returnAddress),
        sim::makePush(sim::Register::R1),
        sim::makeMov(sim::Register::R2, 1),
        sim::makeSub(sim::Register::R1, sim::Register::R2),
        sim::makeCall(recursiveAddress),
        sim::makePop(sim::Register::R2),
        sim::makeAdd(sim::Register::R3, sim::Register::R2),
        sim::makeRet(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.halted());
    assert(cpu.reg(sim::Register::R1) == 0);
    assert(cpu.reg(sim::Register::R2) == 3);
    assert(cpu.reg(sim::Register::R3) == 6);
    assert(cpu.pc() == 3 * sim::Memory::instruction_size);
    assert(cpu.sp() == sim::CPU::initial_sp);
}

int main()
{
    reads_and_writes_signed_32_bit_words();
    rejects_out_of_range_32_bit_words();
    encodes_milestone3_instructions();
    executes_load_and_store();
    executes_push_and_pop();
    executes_call_and_ret();
    supports_recursive_calls();
}

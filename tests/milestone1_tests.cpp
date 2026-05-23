#include "cpu/CPU.hpp"
#include "isa/Instruction.hpp"
#include "memory/Memory.hpp"

#include <array>
#include <cassert>
#include <cstdint>

void executes_mov_add_halt()
{
    const std::array program{
        sim::makeMov(sim::Register::R1, 5),
        sim::makeMov(sim::Register::R2, 7),
        sim::makeAdd(sim::Register::R1, sim::Register::R2),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.halted());
    assert(cpu.reg(sim::Register::R1) == 12);
    assert(cpu.reg(sim::Register::R2) == 7);
    assert(cpu.pc() == program.size() * sim::Memory::instruction_size);
    assert(cpu.sp() == sim::CPU::initial_sp);
}

void encodes_instructions_as_four_bytes()
{
    sim::Memory memory;
    memory.writeInstruction(0, sim::makeMov(sim::Register::R3, -42));

    assert(memory.readByte(0) == static_cast<std::uint8_t>(sim::OpCode::Mov));
    assert(memory.readByte(1) == static_cast<std::uint8_t>(sim::Register::R3));
    assert(memory.readInstruction(0).b == -42);
}

int main()
{
    executes_mov_add_halt();
    encodes_instructions_as_four_bytes();
}

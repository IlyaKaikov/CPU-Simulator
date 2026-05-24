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
    assert(!cpu.zeroFlag());
    assert(!cpu.signFlag());
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

void executes_sub_and_mul()
{
    const std::array program{
        sim::makeMov(sim::Register::R1, 10),
        sim::makeMov(sim::Register::R2, 3),
        sim::makeSub(sim::Register::R1, sim::Register::R2),
        sim::makeMul(sim::Register::R1, sim::Register::R2),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.halted());
    assert(cpu.reg(sim::Register::R1) == 21);
    assert(cpu.reg(sim::Register::R2) == 3);
    assert(!cpu.zeroFlag());
    assert(!cpu.signFlag());
}

void updates_zero_flag_after_arithmetic()
{
    const std::array program{
        sim::makeMov(sim::Register::R1, 3),
        sim::makeMov(sim::Register::R2, 3),
        sim::makeSub(sim::Register::R1, sim::Register::R2),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.reg(sim::Register::R1) == 0);
    assert(cpu.zeroFlag());
    assert(!cpu.signFlag());
}

void updates_sign_flag_after_arithmetic()
{
    const std::array program{
        sim::makeMov(sim::Register::R1, 3),
        sim::makeMov(sim::Register::R2, 5),
        sim::makeSub(sim::Register::R1, sim::Register::R2),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.reg(sim::Register::R1) == -2);
    assert(!cpu.zeroFlag());
    assert(cpu.signFlag());
}

void executes_cmp(int16_t value1, int16_t value2)
{
    const std::array program{
        sim::makeMov(sim::Register::R1, value1),
        sim::makeMov(sim::Register::R2, value2),
        sim::makeCmp(sim::Register::R1, sim::Register::R2),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.reg(sim::Register::R1) == value1);
    assert(cpu.reg(sim::Register::R2) == value2);
    if (value1 == value2) {
        assert(cpu.zeroFlag());
        assert(!cpu.signFlag());
    } else if (value1 < value2) {
        assert(!cpu.zeroFlag());
        assert(cpu.signFlag());
    } else {
        assert(!cpu.zeroFlag());
        assert(!cpu.signFlag());
    }
}

int main()
{
    executes_mov_add_halt();
    encodes_instructions_as_four_bytes();
    executes_sub_and_mul();
    updates_zero_flag_after_arithmetic();
    updates_sign_flag_after_arithmetic();
    executes_cmp(5, 5);
    executes_cmp(3, 7);
    executes_cmp(7, 3);
}

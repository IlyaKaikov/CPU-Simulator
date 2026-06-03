#include "cpu/CPU.hpp"
#include "debugger/Debugger.hpp"
#include "isa/Instruction.hpp"
#include "memory/Memory.hpp"

#include <cassert>
#include <cstdint>
#include <vector>

bool sameInstruction(const sim::EncodedInstruction& left, const sim::EncodedInstruction& right)
{
    return left.opcode == right.opcode && left.a == right.a && left.b == right.b;
}

void step_executes_one_instruction()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeMov(sim::Register::R1, 5),
        sim::makeMov(sim::Register::R2, 7),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);

    const auto result = debugger.step();

    assert(result.reason == sim::DebugStopReason::StepComplete);
    assert(result.pcBefore == sim::CPU::initial_pc);
    assert(result.pcAfter == sim::CPU::initial_pc + sim::Memory::instruction_size);
    assert(sameInstruction(result.instruction, program.at(0)));
    assert(cpu.reg(sim::Register::R1) == 5);
    assert(cpu.reg(sim::Register::R2) == 0);
    assert(!cpu.halted());
}

void step_reports_halt_instruction()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);

    const auto result = debugger.step();

    assert(result.reason == sim::DebugStopReason::Halted);
    assert(result.pcBefore == sim::CPU::initial_pc);
    assert(result.pcAfter == sim::CPU::initial_pc + sim::Memory::instruction_size);
    assert(sameInstruction(result.instruction, program.at(0)));
    assert(cpu.halted());
}

void step_on_halted_cpu_does_not_advance()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();
    const auto haltedPc = cpu.pc();

    sim::Debugger debugger(cpu);
    const auto result = debugger.step();

    assert(result.reason == sim::DebugStopReason::Halted);
    assert(result.pcBefore == haltedPc);
    assert(result.pcAfter == haltedPc);
    assert(result.instruction.opcode == 0);
    assert(result.instruction.a == 0);
    assert(result.instruction.b == 0);
    assert(cpu.pc() == haltedPc);
    assert(cpu.halted());
}

int main()
{
    step_executes_one_instruction();
    step_reports_halt_instruction();
    step_on_halted_cpu_does_not_advance();
}

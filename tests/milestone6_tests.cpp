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

void continue_execution_runs_until_halt()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeMov(sim::Register::R1, 5),
        sim::makeMov(sim::Register::R2, 7),
        sim::makeAdd(sim::Register::R1, sim::Register::R2),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);

    const auto result = debugger.continueExecution();

    assert(result.reason == sim::DebugStopReason::Halted);
    assert(result.pcBefore == 3 * sim::Memory::instruction_size);
    assert(result.pcAfter == 4 * sim::Memory::instruction_size);
    assert(sameInstruction(result.instruction, program.at(3)));
    assert(cpu.halted());
    assert(cpu.reg(sim::Register::R1) == 12);
    assert(cpu.reg(sim::Register::R2) == 7);
}

void continue_execution_reports_max_steps_exceeded()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeJmp(0),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);

    const auto result = debugger.continueExecution(3);

    assert(result.reason == sim::DebugStopReason::MaxStepsExceeded);
    assert(result.pcBefore == 0);
    assert(result.pcAfter == 0);
    assert(result.instruction.opcode == 0);
    assert(result.instruction.a == 0);
    assert(result.instruction.b == 0);
    assert(!cpu.halted());
    assert(cpu.pc() == 0);
}

void continue_execution_on_halted_cpu_does_not_advance()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();
    const auto haltedPc = cpu.pc();

    sim::Debugger debugger(cpu);
    const auto result = debugger.continueExecution();

    assert(result.reason == sim::DebugStopReason::Halted);
    assert(result.pcBefore == haltedPc);
    assert(result.pcAfter == haltedPc);
    assert(cpu.pc() == haltedPc);
}

void manages_breakpoints()
{
    sim::CPU cpu;
    sim::Debugger debugger(cpu);

    const auto address = 2 * sim::Memory::instruction_size;

    assert(!debugger.hasBreakpoint(address));

    debugger.addBreakpoint(address);
    assert(debugger.hasBreakpoint(address));

    debugger.removeBreakpoint(address);
    assert(!debugger.hasBreakpoint(address));

    debugger.addBreakpoint(address);
    debugger.clearBreakpoints();
    assert(!debugger.hasBreakpoint(address));
}

void continue_execution_stops_before_breakpoint()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeMov(sim::Register::R1, 5),
        sim::makeMov(sim::Register::R2, 7),
        sim::makeHalt(),
    };
    const auto breakpointAddress = sim::Memory::instruction_size;

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);
    debugger.addBreakpoint(breakpointAddress);

    const auto result = debugger.continueExecution();

    assert(result.reason == sim::DebugStopReason::Breakpoint);
    assert(result.pcBefore == breakpointAddress);
    assert(result.pcAfter == breakpointAddress);
    assert(sameInstruction(result.instruction, program.at(1)));
    assert(cpu.pc() == breakpointAddress);
    assert(cpu.reg(sim::Register::R1) == 5);
    assert(cpu.reg(sim::Register::R2) == 0);
    assert(!cpu.halted());
}

void breakpoint_stop_is_repeatable_until_stepped_or_removed()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeMov(sim::Register::R1, 5),
        sim::makeMov(sim::Register::R2, 7),
        sim::makeHalt(),
    };
    const auto breakpointAddress = sim::Memory::instruction_size;

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);
    debugger.addBreakpoint(breakpointAddress);

    const auto firstStop = debugger.continueExecution();
    const auto secondStop = debugger.continueExecution();

    assert(firstStop.reason == sim::DebugStopReason::Breakpoint);
    assert(secondStop.reason == sim::DebugStopReason::Breakpoint);
    assert(cpu.pc() == breakpointAddress);
    assert(cpu.reg(sim::Register::R2) == 0);

    const auto stepResult = debugger.step();
    assert(stepResult.reason == sim::DebugStopReason::StepComplete);
    assert(stepResult.pcBefore == breakpointAddress);
    assert(stepResult.pcAfter == 2 * sim::Memory::instruction_size);
    assert(cpu.reg(sim::Register::R2) == 7);
}

void removing_breakpoint_allows_continue_execution()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeMov(sim::Register::R1, 5),
        sim::makeMov(sim::Register::R2, 7),
        sim::makeHalt(),
    };
    const auto breakpointAddress = sim::Memory::instruction_size;

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);
    debugger.addBreakpoint(breakpointAddress);

    const auto breakpointResult = debugger.continueExecution();
    assert(breakpointResult.reason == sim::DebugStopReason::Breakpoint);

    debugger.removeBreakpoint(breakpointAddress);
    const auto haltResult = debugger.continueExecution();

    assert(haltResult.reason == sim::DebugStopReason::Halted);
    assert(cpu.halted());
    assert(cpu.reg(sim::Register::R1) == 5);
    assert(cpu.reg(sim::Register::R2) == 7);
}

int main()
{
    step_executes_one_instruction();
    step_reports_halt_instruction();
    step_on_halted_cpu_does_not_advance();
    continue_execution_runs_until_halt();
    continue_execution_reports_max_steps_exceeded();
    continue_execution_on_halted_cpu_does_not_advance();
    manages_breakpoints();
    continue_execution_stops_before_breakpoint();
    breakpoint_stop_is_repeatable_until_stepped_or_removed();
    removing_breakpoint_allows_continue_execution();
}

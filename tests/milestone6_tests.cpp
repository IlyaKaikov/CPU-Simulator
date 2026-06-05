#include "CPU.hpp"
#include "Debugger.hpp"
#include "Instruction.hpp"
#include "Memory.hpp"

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

bool contains(const std::string& text, const std::string& expected)
{
    return text.find(expected) != std::string::npos;
}

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

void register_dump_includes_all_registers_and_state()
{
    sim::CPU cpu;
    sim::Debugger debugger(cpu);

    const auto dump = debugger.dumpRegisters();

    assert(contains(dump, "halted = 0"));
    assert(contains(dump, "R0 = 0"));
    assert(contains(dump, "R1 = 0"));
    assert(contains(dump, "R2 = 0"));
    assert(contains(dump, "R3 = 0"));
    assert(contains(dump, "R4 = 0"));
    assert(contains(dump, "R5 = 0"));
    assert(contains(dump, "R6 = 0"));
    assert(contains(dump, "R7 = 0"));
    assert(contains(dump, "ZF = 0"));
    assert(contains(dump, "SF = 0"));
    assert(contains(dump, "PC = 0x0"));
    assert(contains(dump, "SP = 0x10000"));
}

void register_dump_reflects_current_cpu_state()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeMov(sim::Register::R1, 5),
        sim::makeMov(sim::Register::R2, 7),
        sim::makeCmp(sim::Register::R1, sim::Register::R2),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);
    const auto result = debugger.continueExecution(3);

    const auto dump = debugger.dumpRegisters();

    assert(result.reason == sim::DebugStopReason::MaxStepsExceeded);
    assert(contains(dump, "halted = 0"));
    assert(contains(dump, "R1 = 5"));
    assert(contains(dump, "R2 = 7"));
    assert(contains(dump, "ZF = 0"));
    assert(contains(dump, "SF = 1"));
    assert(contains(dump, "PC = 0xc"));
    assert(contains(dump, "SP = 0x10000"));
}

void memory_dump_includes_little_endian_bytes()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeMov(sim::Register::R1, 0x1234),
        sim::makeStore(sim::Register::R1, 0x8000),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);
    const auto result = debugger.continueExecution();

    const auto dump = debugger.dumpMemory(0x8000, 4);

    assert(result.reason == sim::DebugStopReason::Halted);
    assert(contains(dump, "0x8000: 34 12 00 00"));
}

void memory_dump_wraps_after_sixteen_bytes()
{
    sim::CPU cpu;
    sim::Debugger debugger(cpu);

    const auto dump = debugger.dumpMemory(0, 17);

    assert(contains(dump, "0x0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"));
    assert(contains(dump, "0x10: 00"));
}

void memory_dump_rejects_out_of_range_requests()
{
    sim::CPU cpu;
    sim::Debugger debugger(cpu);
    bool threw = false;

    try {
        const auto dump = debugger.dumpMemory(sim::Memory::size - 1, 2);
        (void)dump;
    } catch (const std::out_of_range& error) {
        threw = true;
        assert(contains(error.what(), "memory access out of range"));
    }

    assert(threw);
}

void tracing_is_disabled_by_default()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeMov(sim::Register::R1, 5),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);

    const auto result = debugger.continueExecution();

    assert(result.reason == sim::DebugStopReason::Halted);
    assert(debugger.trace().empty());
}

void tracing_records_step_execution()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeMov(sim::Register::R1, 5),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);
    debugger.enableTracing(true);

    const auto result = debugger.step();
    const auto& trace = debugger.trace();

    assert(result.reason == sim::DebugStopReason::StepComplete);
    assert(trace.size() == 1);
    assert(trace.at(0).pcBefore == 0);
    assert(trace.at(0).pcAfter == sim::Memory::instruction_size);
    assert(sameInstruction(trace.at(0).instruction, program.at(0)));
}

void tracing_records_continue_execution()
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
    debugger.enableTracing(true);

    const auto result = debugger.continueExecution();
    const auto& trace = debugger.trace();

    assert(result.reason == sim::DebugStopReason::Halted);
    assert(trace.size() == program.size());
    assert(trace.at(0).pcBefore == 0);
    assert(trace.at(0).pcAfter == sim::Memory::instruction_size);
    assert(sameInstruction(trace.at(0).instruction, program.at(0)));
    assert(trace.at(3).pcBefore == 3 * sim::Memory::instruction_size);
    assert(trace.at(3).pcAfter == 4 * sim::Memory::instruction_size);
    assert(sameInstruction(trace.at(3).instruction, program.at(3)));
}

void tracing_captures_branch_pc_after()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeJmp(2 * sim::Memory::instruction_size),
        sim::makeMov(sim::Register::R1, 99),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);
    debugger.enableTracing(true);

    const auto result = debugger.step();
    const auto& trace = debugger.trace();

    assert(result.reason == sim::DebugStopReason::StepComplete);
    assert(trace.size() == 1);
    assert(trace.at(0).pcBefore == 0);
    assert(trace.at(0).pcAfter == 2 * sim::Memory::instruction_size);
    assert(sameInstruction(trace.at(0).instruction, program.at(0)));
}

void tracing_skips_breakpoint_stops_and_halted_steps()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeMov(sim::Register::R1, 5),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);
    debugger.enableTracing(true);
    debugger.addBreakpoint(0);

    const auto breakpointResult = debugger.continueExecution();
    assert(breakpointResult.reason == sim::DebugStopReason::Breakpoint);
    assert(debugger.trace().empty());

    debugger.clearBreakpoints();
    const auto haltResult = debugger.continueExecution();
    assert(haltResult.reason == sim::DebugStopReason::Halted);
    assert(debugger.trace().size() == 2);

    const auto haltedStep = debugger.step();
    assert(haltedStep.reason == sim::DebugStopReason::Halted);
    assert(debugger.trace().size() == 2);
}

void tracing_can_be_cleared_and_disabled()
{
    const std::vector<sim::EncodedInstruction> program{
        sim::makeMov(sim::Register::R1, 5),
        sim::makeMov(sim::Register::R2, 7),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    sim::Debugger debugger(cpu);
    debugger.enableTracing(true);

    const auto firstStep = debugger.step();
    assert(firstStep.reason == sim::DebugStopReason::StepComplete);
    assert(debugger.trace().size() == 1);

    debugger.clearTrace();
    assert(debugger.trace().empty());

    debugger.enableTracing(false);
    const auto secondStep = debugger.step();
    assert(secondStep.reason == sim::DebugStopReason::StepComplete);
    assert(debugger.trace().empty());
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
    register_dump_includes_all_registers_and_state();
    register_dump_reflects_current_cpu_state();
    memory_dump_includes_little_endian_bytes();
    memory_dump_wraps_after_sixteen_bytes();
    memory_dump_rejects_out_of_range_requests();
    tracing_is_disabled_by_default();
    tracing_records_step_execution();
    tracing_records_continue_execution();
    tracing_captures_branch_pc_after();
    tracing_skips_breakpoint_stops_and_halted_steps();
    tracing_can_be_cleared_and_disabled();
}

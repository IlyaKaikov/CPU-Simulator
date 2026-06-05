#include "Debugger.hpp"

#include <iomanip>
#include <sstream>

namespace sim {

Debugger::Debugger(CPU& cpu)
    : cpu_(cpu)
{
}

DebugStepResult Debugger::step()
{
    const auto pcBefore = cpu_.pc();

    if (cpu_.halted()) {
        return DebugStepResult{
            DebugStopReason::Halted,
            pcBefore,
            pcBefore,
            {},
        };
    }

    const auto instruction = cpu_.memory().readInstruction(pcBefore);
    cpu_.step();
    const auto pcAfter = cpu_.pc();

    if (tracingEnabled_) {
        trace_.push_back(TraceEntry{
            pcBefore,
            pcAfter,
            instruction,
        });
    }

    return DebugStepResult{
        cpu_.halted() ? DebugStopReason::Halted : DebugStopReason::StepComplete,
        pcBefore,
        pcAfter,
        instruction,
    };
}

DebugStepResult Debugger::continueExecution(std::size_t maxSteps)
{
    if (cpu_.halted()) {
        return step();
    }

    DebugStepResult lastResult{};
    std::size_t steps = 0;

    while (!cpu_.halted()) {
        const auto pc = cpu_.pc();
        if (hasBreakpoint(pc)) {
            return DebugStepResult{
                DebugStopReason::Breakpoint,
                pc,
                pc,
                cpu_.memory().readInstruction(pc),
            };
        }

        if (steps >= maxSteps) {
            return DebugStepResult{
                DebugStopReason::MaxStepsExceeded,
                pc,
                pc,
                {},
            };
        }

        lastResult = step();
        ++steps;

        if (lastResult.reason == DebugStopReason::Halted) {
            return lastResult;
        }
    }

    return lastResult;
}

void Debugger::addBreakpoint(std::uint32_t address)
{
    breakpoints_.insert(address);
}

void Debugger::removeBreakpoint(std::uint32_t address)
{
    breakpoints_.erase(address);
}

void Debugger::clearBreakpoints()
{
    breakpoints_.clear();
}

bool Debugger::hasBreakpoint(std::uint32_t address) const
{
    return breakpoints_.contains(address);
}

std::string Debugger::dumpRegisters() const
{
    std::ostringstream output;

    output << "halted = " << cpu_.halted() << '\n';
    for (auto index = 0; index < static_cast<int>(CPU::register_count); ++index) {
        const auto reg = static_cast<Register>(index);
        output << "R" << index << " = " << cpu_.reg(reg) << '\n';
    }
    output << "ZF = " << cpu_.zeroFlag() << '\n';
    output << "SF = " << cpu_.signFlag() << '\n';
    output << "PC = 0x" << std::hex << cpu_.pc() << '\n';
    output << "SP = 0x" << cpu_.sp() << std::dec << '\n';

    return output.str();
}

std::string Debugger::dumpMemory(std::uint32_t startAddress, std::size_t byteCount) const
{
    std::ostringstream output;

    for (std::size_t offset = 0; offset < byteCount; ++offset) {
        if (offset % 16 == 0) {
            if (offset != 0) {
                output << '\n';
            }
            output << "0x" << std::hex << (startAddress + offset) << ":";
        }

        const auto byte = cpu_.memory().readByte(startAddress + static_cast<std::uint32_t>(offset));
        output << ' ' << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte);
    }

    if (byteCount != 0) {
        output << '\n';
    }

    return output.str();
}

void Debugger::enableTracing(bool enabled)
{
    tracingEnabled_ = enabled;
}

void Debugger::clearTrace()
{
    trace_.clear();
}

const std::vector<TraceEntry>& Debugger::trace() const
{
    return trace_;
}

}

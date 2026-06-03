#include "debugger/Debugger.hpp"

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

    return DebugStepResult{
        cpu_.halted() ? DebugStopReason::Halted : DebugStopReason::StepComplete,
        pcBefore,
        cpu_.pc(),
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

}

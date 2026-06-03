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

}

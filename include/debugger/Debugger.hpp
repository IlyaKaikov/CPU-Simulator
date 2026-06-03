#pragma once

#include "cpu/CPU.hpp"
#include "isa/Instruction.hpp"

#include <cstdint>

namespace sim {

enum class DebugStopReason {
    StepComplete,
    Breakpoint,
    Halted,
    MaxStepsExceeded,
};

struct DebugStepResult {
    DebugStopReason reason{DebugStopReason::StepComplete};
    std::uint32_t pcBefore{};
    std::uint32_t pcAfter{};
    EncodedInstruction instruction{};
};

class Debugger {
public:
    explicit Debugger(CPU& cpu);

    [[nodiscard]] DebugStepResult step();

private:
    CPU& cpu_;
};

}

#pragma once

#include "cpu/CPU.hpp"
#include "isa/Instruction.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

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

struct TraceEntry {
    std::uint32_t pcBefore{};
    std::uint32_t pcAfter{};
    EncodedInstruction instruction{};
};

class Debugger {
public:
    explicit Debugger(CPU& cpu);

    [[nodiscard]] DebugStepResult step();
    [[nodiscard]] DebugStepResult continueExecution(std::size_t maxSteps = 10000);

    void addBreakpoint(std::uint32_t address);
    void removeBreakpoint(std::uint32_t address);
    void clearBreakpoints();
    [[nodiscard]] bool hasBreakpoint(std::uint32_t address) const;

    [[nodiscard]] std::string dumpRegisters() const;
    [[nodiscard]] std::string dumpMemory(std::uint32_t startAddress, std::size_t byteCount) const;

    void enableTracing(bool enabled);
    void clearTrace();
    [[nodiscard]] const std::vector<TraceEntry>& trace() const;

private:
    CPU& cpu_;
    std::unordered_set<std::uint32_t> breakpoints_;
    bool tracingEnabled_{false};
    std::vector<TraceEntry> trace_;
};

}

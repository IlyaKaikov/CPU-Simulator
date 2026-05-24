#pragma once

#include "isa/Instruction.hpp"
#include "memory/Memory.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace sim {

class CPU {
public:
    static constexpr std::uint32_t initial_pc = 0x0000;
    static constexpr std::uint32_t initial_sp = 0x10000;
    static constexpr std::size_t register_count = 8;

    CPU();

    void reset();
    void loadProgram(std::span<const EncodedInstruction> program, std::uint32_t startAddress = initial_pc);
    void step();
    void run(std::size_t maxSteps = 100000);

    [[nodiscard]] bool halted() const;
    [[nodiscard]] std::int32_t reg(Register reg) const;
    [[nodiscard]] std::uint32_t pc() const;
    [[nodiscard]] std::uint32_t sp() const;
    [[nodiscard]] bool zeroFlag() const;
    [[nodiscard]] bool signFlag() const;
    [[nodiscard]] const Memory& memory() const;

private:
    void execute(const EncodedInstruction& instruction);
    void updateFlags(std::int32_t result);
    [[nodiscard]] std::size_t registerIndex(Register reg) const;

    std::array<std::int32_t, register_count> registers_{};
    Memory memory_{};
    std::uint32_t pc_{initial_pc};
    std::uint32_t sp_{initial_sp};
    bool zero_flag_{false};
    bool sign_flag_{false};
    bool halted_{false};
};

}

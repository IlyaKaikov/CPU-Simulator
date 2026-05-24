#include "cpu/CPU.hpp"

#include <stdexcept>
#include <string>

namespace sim {

CPU::CPU()
{
    reset();
}

void CPU::reset()
{
    registers_.fill(0);
    memory_.reset();
    pc_ = initial_pc;
    sp_ = initial_sp;
    zero_flag_ = false;
    sign_flag_ = false;
    halted_ = false;
}

void CPU::loadProgram(std::span<const EncodedInstruction> program, std::uint32_t startAddress)
{
    auto address = startAddress;
    for (const auto& instruction : program) {
        memory_.writeInstruction(address, instruction);
        address += Memory::instruction_size;
    }

    pc_ = startAddress;
    halted_ = false;
}

void CPU::step()
{
    if (halted_) {
        return;
    }

    const auto instruction = memory_.readInstruction(pc_);
    pc_ += Memory::instruction_size;
    execute(instruction);
}

void CPU::run(std::size_t maxSteps)
{
    std::size_t steps = 0;
    while (!halted_) {
        if (steps >= maxSteps) {
            throw std::runtime_error("CPU execution exceeded max step count");
        }

        step();
        ++steps;
    }
}

bool CPU::halted() const
{
    return halted_;
}

std::int32_t CPU::reg(Register reg) const
{
    return registers_.at(registerIndex(reg));
}

std::uint32_t CPU::pc() const
{
    return pc_;
}

std::uint32_t CPU::sp() const
{
    return sp_;
}

const Memory& CPU::memory() const
{
    return memory_;
}

bool CPU::zeroFlag() const
{
    return zero_flag_;
}

bool CPU::signFlag() const
{
    return sign_flag_;
}

void CPU::execute(const EncodedInstruction& instruction)
{
    const auto opcode = static_cast<OpCode>(instruction.opcode);

    switch (opcode) {
    case OpCode::Nop:
        break;
    case OpCode::Mov:
        registers_.at(registerIndex(static_cast<Register>(instruction.a))) = instruction.b;
        break;
    case OpCode::Add: {
        auto& destination = registers_.at(registerIndex(static_cast<Register>(instruction.a)));
        const auto source = registers_.at(registerIndex(static_cast<Register>(instruction.b)));
        destination += source;
        updateFlags(destination);
        break;
    }
    case OpCode::Sub: {
        auto& destination = registers_.at(registerIndex(static_cast<Register>(instruction.a)));
        const auto source = registers_.at(registerIndex(static_cast<Register>(instruction.b)));
        destination -= source;
        updateFlags(destination);
        break;
    }
    case OpCode::Mul: {
        auto& destination = registers_.at(registerIndex(static_cast<Register>(instruction.a)));
        const auto source = registers_.at(registerIndex(static_cast<Register>(instruction.b)));
        destination *= source;
        updateFlags(destination);
        break;
    }
    case OpCode::Cmp: {
        const auto destination = registers_.at(registerIndex(static_cast<Register>(instruction.a)));
        const auto source = registers_.at(registerIndex(static_cast<Register>(instruction.b)));
        const auto result = destination - source;
        updateFlags(result);
        break;
    }
    case OpCode::Halt:
        halted_ = true;
        break;
    default:
        throw std::runtime_error("unknown opcode: " + std::to_string(instruction.opcode));
    }
}

void CPU::updateFlags(std::int32_t result)
{
    zero_flag_ = result == 0;
    sign_flag_ = result < 0;
}

std::size_t CPU::registerIndex(Register reg) const
{
    const auto index = static_cast<std::size_t>(reg);
    if (index >= register_count) {
        throw std::out_of_range("invalid register index: " + std::to_string(index));
    }

    return index;
}

}

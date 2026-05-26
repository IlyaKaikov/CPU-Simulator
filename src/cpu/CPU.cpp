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
        const auto left = registers_.at(registerIndex(static_cast<Register>(instruction.a)));
        const auto right = registers_.at(registerIndex(static_cast<Register>(instruction.b)));
        const auto result = left - right;
        updateFlags(result);
        break;
    }
    case OpCode::Jmp:
        pc_ = static_cast<std::uint16_t>(instruction.b);
        break;
    case OpCode::Je:
        if (zero_flag_) {
            pc_ = static_cast<std::uint16_t>(instruction.b);
        }
        break;
    case OpCode::Jne:
        if (!zero_flag_) {
            pc_ = static_cast<std::uint16_t>(instruction.b);
        }
        break;
    case OpCode::Jg:
        if (!zero_flag_ && !sign_flag_) {
            pc_ = static_cast<std::uint16_t>(instruction.b);
        }
        break;
    case OpCode::Jl:
        if (sign_flag_) {
            pc_ = static_cast<std::uint16_t>(instruction.b);
        }
        break;
    case OpCode::Load: {
        auto& destination = registers_.at(registerIndex(static_cast<Register>(instruction.a)));
        destination = memory_.readInt32(static_cast<std::uint16_t>(instruction.b));
        break;
    }
    case OpCode::Store: {
        const auto source = registers_.at(registerIndex(static_cast<Register>(instruction.a)));
        memory_.writeInt32(static_cast<std::uint16_t>(instruction.b), source);
        break;
    }
    case OpCode::Push: {
        const auto source = registers_.at(registerIndex(static_cast<Register>(instruction.a)));
        sp_ -= Memory::word_size;
        memory_.writeInt32(sp_, source);
        break;
    }
    case OpCode::Pop: {
        const auto value = memory_.readInt32(sp_);
        auto& destination = registers_.at(registerIndex(static_cast<Register>(instruction.a)));
        destination = value;
        sp_ += Memory::word_size;
        break;
    }
    case OpCode::Call: {
        sp_ -= Memory::word_size;
        memory_.writeInt32(sp_, pc_);
        pc_ = static_cast<std::uint16_t>(instruction.b);
        break;
    }
    case OpCode::Ret: {
        pc_ = memory_.readInt32(sp_);
        sp_ += Memory::word_size;
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

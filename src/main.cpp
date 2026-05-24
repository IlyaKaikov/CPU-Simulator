#include "cpu/CPU.hpp"
#include "isa/Instruction.hpp"
#include "memory/Memory.hpp"

#include <array>
#include <cstdint>
#include <iostream>

int main()
{
    constexpr auto loopAddress = static_cast<std::uint16_t>(3 * sim::Memory::instruction_size);
    const std::array program{
        sim::makeMov(sim::Register::R1, 3),
        sim::makeMov(sim::Register::R2, 0),
        sim::makeMov(sim::Register::R3, 1),
        sim::makeAdd(sim::Register::R2, sim::Register::R1),
        sim::makeSub(sim::Register::R1, sim::Register::R3),
        sim::makeCmp(sim::Register::R1, sim::Register::R0),
        sim::makeJg(loopAddress),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    std::cout << "Milestone 2 demo: sum 3 + 2 + 1\n";
    std::cout << "R1 = " << cpu.reg(sim::Register::R1) << '\n';
    std::cout << "R2 = " << cpu.reg(sim::Register::R2) << '\n';
    std::cout << "ZF = " << cpu.zeroFlag() << '\n';
    std::cout << "SF = " << cpu.signFlag() << '\n';
}

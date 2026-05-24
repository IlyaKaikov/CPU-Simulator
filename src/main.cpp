#include "cpu/CPU.hpp"
#include "isa/Instruction.hpp"

#include <array>
#include <iostream>

int main()
{
    const std::array program{
        sim::makeMov(sim::Register::R1, 10),
        sim::makeMov(sim::Register::R2, 3),
        sim::makeSub(sim::Register::R1, sim::Register::R2),
        sim::makeMul(sim::Register::R1, sim::Register::R2),
        sim::makeHalt(),
    };

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    std::cout << "R1 = " << cpu.reg(sim::Register::R1) << '\n';
}

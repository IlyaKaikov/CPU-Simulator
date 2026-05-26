#include "cpu/CPU.hpp"
#include "isa/Instruction.hpp"
#include "memory/Memory.hpp"

#include <array>
#include <cstdint>
#include <iostream>

int main()
{
    constexpr auto loopAddress = static_cast<std::uint16_t>(3 * sim::Memory::instruction_size);
    const std::array milestone2Program{
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
    cpu.loadProgram(milestone2Program);
    cpu.run();

    std::cout << "Milestone 2 demo: sum 3 + 2 + 1\n";
    std::cout << "R1 = " << cpu.reg(sim::Register::R1) << '\n';
    std::cout << "R2 = " << cpu.reg(sim::Register::R2) << '\n';
    std::cout << "ZF = " << cpu.zeroFlag() << '\n';
    std::cout << "SF = " << cpu.signFlag() << '\n';

    constexpr auto recursiveAddress = static_cast<std::uint16_t>(3 * sim::Memory::instruction_size);
    constexpr auto returnAddress = static_cast<std::uint16_t>(11 * sim::Memory::instruction_size);
    const std::array milestone3Program{
        sim::makeMov(sim::Register::R1, 3),
        sim::makeCall(recursiveAddress),
        sim::makeHalt(),
        sim::makeCmp(sim::Register::R1, sim::Register::R0),
        sim::makeJe(returnAddress),
        sim::makePush(sim::Register::R1),
        sim::makeMov(sim::Register::R2, 1),
        sim::makeSub(sim::Register::R1, sim::Register::R2),
        sim::makeCall(recursiveAddress),
        sim::makePop(sim::Register::R2),
        sim::makeAdd(sim::Register::R3, sim::Register::R2),
        sim::makeRet(),
    };

    cpu.reset();
    cpu.loadProgram(milestone3Program);
    cpu.run();

    std::cout << "\nMilestone 3 demo: recursive sum 3 + 2 + 1\n";
    std::cout << "R1 = " << cpu.reg(sim::Register::R1) << '\n';
    std::cout << "R2 = " << cpu.reg(sim::Register::R2) << '\n';
    std::cout << "R3 = " << cpu.reg(sim::Register::R3) << '\n';
    std::cout << "SP = 0x" << std::hex << cpu.sp() << std::dec << '\n';
}

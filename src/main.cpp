#include "assembler/Assembler.hpp"
#include "cpu/CPU.hpp"
#include "isa/Instruction.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string readTextFile(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("failed to open " + path.string());
    }

    std::ostringstream contents;
    contents << input.rdbuf();
    return contents.str();
}

std::vector<std::filesystem::path> examplePrograms()
{
    std::vector<std::filesystem::path> paths;
    const std::filesystem::path examplesDir{"examples"};

    for (const auto& entry : std::filesystem::directory_iterator(examplesDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".asm") {
            paths.push_back(entry.path());
        }
    }

    std::sort(paths.begin(), paths.end());
    return paths;
}

void printCpuState(const sim::CPU& cpu)
{
    std::cout << "  halted = " << cpu.halted() << '\n';
    for (auto index = 0; index < static_cast<int>(sim::CPU::register_count); ++index) {
        const auto reg = static_cast<sim::Register>(index);
        std::cout << "  R" << index << " = " << cpu.reg(reg) << '\n';
    }
    std::cout << "  ZF = " << cpu.zeroFlag() << '\n';
    std::cout << "  SF = " << cpu.signFlag() << '\n';
    std::cout << "  PC = 0x" << std::hex << cpu.pc() << '\n';
    std::cout << "  SP = 0x" << cpu.sp() << std::dec << '\n';
}

}

int main()
{
    const sim::Assembler assembler;
    const auto paths = examplePrograms();

    if (paths.empty()) {
        std::cout << "No .asm examples found.\n";
        return 0;
    }

    for (const auto& path : paths) {
        std::cout << path.string() << '\n';

        try {
            const auto source = readTextFile(path);
            const auto program = assembler.assemble(source);

            sim::CPU cpu;
            cpu.loadProgram(program);
            cpu.run();
            printCpuState(cpu);
        } catch (const std::exception& error) {
            std::cout << "  error: " << error.what() << '\n';
        }

        std::cout << '\n';
    }
}

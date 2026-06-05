#include "assembler/Assembler.hpp"
#include "binary/ProgramBinary.hpp"
#include "cpu/CPU.hpp"
#include "debugger/Debugger.hpp"
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

void runProgram(const std::vector<sim::EncodedInstruction>& program)
{
    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();
    printCpuState(cpu);
}

const char* stopReasonName(sim::DebugStopReason reason)
{
    switch (reason) {
    case sim::DebugStopReason::StepComplete:
        return "step-complete";
    case sim::DebugStopReason::Breakpoint:
        return "breakpoint";
    case sim::DebugStopReason::Halted:
        return "halted";
    case sim::DebugStopReason::MaxStepsExceeded:
        return "max-steps-exceeded";
    }

    return "unknown";
}

void printTrace(const std::vector<sim::TraceEntry>& trace)
{
    std::cout << "Trace:\n";
    for (const auto& entry : trace) {
        std::cout << "  0x" << std::hex << entry.pcBefore << " -> 0x" << entry.pcAfter << std::dec;
        std::cout << " opcode=0x" << std::hex << static_cast<int>(entry.instruction.opcode) << std::dec;
        std::cout << " a=" << static_cast<int>(entry.instruction.a);
        std::cout << " b=" << entry.instruction.b << '\n';
    }
}

void printUsage(const char* executable)
{
    std::cerr << "Usage:\n";
    std::cerr << "  " << executable << '\n';
    std::cerr << "  " << executable << " assemble <input.asm> <output.bin>\n";
    std::cerr << "  " << executable << " run <input.bin>\n";
    std::cerr << "  " << executable << " debug <input.bin>\n";
}

int runExamples()
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
            runProgram(program);
        } catch (const std::exception& error) {
            std::cout << "  error: " << error.what() << '\n';
        }

        std::cout << '\n';
    }

    return 0;
}

int assembleBinary(const std::filesystem::path& sourcePath, const std::filesystem::path& binaryPath)
{
    const sim::Assembler assembler;
    const auto source = readTextFile(sourcePath);
    const auto program = assembler.assemble(source);

    sim::writeProgramBinaryFile(binaryPath, program);
    std::cout << "Assembled " << sourcePath.string() << " -> " << binaryPath.string() << '\n';
    return 0;
}

int runBinary(const std::filesystem::path& binaryPath)
{
    const auto program = sim::readProgramBinaryFile(binaryPath);
    runProgram(program);
    return 0;
}

int debugBinary(const std::filesystem::path& binaryPath)
{
    const auto program = sim::readProgramBinaryFile(binaryPath);

    sim::CPU cpu;
    cpu.loadProgram(program);

    sim::Debugger debugger(cpu);
    debugger.enableTracing(true);
    const auto result = debugger.continueExecution();

    std::cout << "Stop reason: " << stopReasonName(result.reason) << '\n';
    std::cout << debugger.dumpRegisters();
    printTrace(debugger.trace());

    return result.reason == sim::DebugStopReason::MaxStepsExceeded ? 1 : 0;
}

}

int main(int argc, char* argv[])
{
    try {
        if (argc == 1) {
            return runExamples();
        }

        const std::string command = argv[1];
        if (command == "assemble" && argc == 4) {
            return assembleBinary(argv[2], argv[3]);
        }
        if (command == "run" && argc == 3) {
            return runBinary(argv[2]);
        }
        if (command == "debug" && argc == 3) {
            return debugBinary(argv[2]);
        }

        printUsage(argv[0]);
        return 2;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}

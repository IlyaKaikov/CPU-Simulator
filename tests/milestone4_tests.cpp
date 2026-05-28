#include "assembler/Assembler.hpp"
#include "cpu/CPU.hpp"
#include "isa/Instruction.hpp"
#include "memory/Memory.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

bool contains(const std::string& text, const std::string& expected)
{
    return text.find(expected) != std::string::npos;
}

bool sameInstruction(const sim::EncodedInstruction& left, const sim::EncodedInstruction& right)
{
    return left.opcode == right.opcode && left.a == right.a && left.b == right.b;
}

void assertProgramEquals(
    const std::vector<sim::EncodedInstruction>& actual,
    const std::vector<sim::EncodedInstruction>& expected)
{
    assert(actual.size() == expected.size());
    for (std::size_t index = 0; index < actual.size(); ++index) {
        assert(sameInstruction(actual.at(index), expected.at(index)));
    }
}

void assembles_empty_source()
{
    const sim::Assembler assembler;
    const auto program = assembler.assemble("");

    assert(program.empty());
}

void ignores_blank_and_comment_lines()
{
    const sim::Assembler assembler;
    const auto program = assembler.assemble(R"(
        ; milestone 4.1 skeleton

        ; comments do not emit instructions
    )");

    assert(program.empty());
}

void assembles_no_operand_instruction()
{
    const sim::Assembler assembler;
    const auto program = assembler.assemble("HALT");

    assertProgramEquals(program, {sim::makeHalt()});
}

void reports_scanned_line_number()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble(R"(
            ; comment-only line
            BOGUS
        )");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "line 3"));
        assert(contains(message, "BOGUS"));
    }

    assert(threw);
}

void tokenizes_whitespace_and_commas()
{
    const sim::Assembler assembler;
    const auto program = assembler.assemble("  MOV   R1,   5  ; initialize R1");

    assertProgramEquals(program, {sim::makeMov(sim::Register::R1, 5)});
}

void tokenizes_adjacent_comma_operands()
{
    const sim::Assembler assembler;
    const auto program = assembler.assemble("ADD R1,R2");

    assertProgramEquals(program, {sim::makeAdd(sim::Register::R1, sim::Register::R2)});
}

void parses_valid_registers_and_numbers()
{
    const sim::Assembler assembler;
    const auto program = assembler.assemble("MOV r7, -42");

    assertProgramEquals(program, {sim::makeMov(sim::Register::R7, -42)});
}

void parses_hex_uint16_addresses()
{
    const sim::Assembler assembler;
    const auto program = assembler.assemble("LOAD R2, 0x8000");

    assertProgramEquals(program, {sim::makeLoad(sim::Register::R2, 0x8000)});
}

void rejects_invalid_registers()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble("MOV R8, 1");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "line 1"));
        assert(contains(message, "invalid register 'R8'"));
    }

    assert(threw);
}

void rejects_invalid_numbers()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble("MOV R1, nope");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "line 1"));
        assert(contains(message, "invalid int16 'nope'"));
    }

    assert(threw);
}

void rejects_out_of_range_int16_values()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble("MOV R1, 40000");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "line 1"));
        assert(contains(message, "int16 out of range '40000'"));
    }

    assert(threw);
}

void rejects_out_of_range_uint16_values()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble("STORE R1, 0x10000");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "line 1"));
        assert(contains(message, "uint16 out of range '0x10000'"));
    }

    assert(threw);
}

void assembles_numeric_instruction_forms()
{
    const sim::Assembler assembler;
    const auto program = assembler.assemble(R"(
        NOP
        MOV R1, -5
        ADD R1, R2
        SUB R2, R3
        MUL R3, R4
        CMP R4, R5
        JMP 0x0010
        JE 0x0014
        JNE 0x0018
        JG 0x001c
        JL 0x0020
        LOAD R6, 0x8000
        STORE R6, 0x8004
        PUSH R7
        POP R0
        CALL 0x0024
        RET
        HALT
    )");

    assertProgramEquals(
        program,
        {
            sim::EncodedInstruction{static_cast<std::uint8_t>(sim::OpCode::Nop), 0, 0},
            sim::makeMov(sim::Register::R1, -5),
            sim::makeAdd(sim::Register::R1, sim::Register::R2),
            sim::makeSub(sim::Register::R2, sim::Register::R3),
            sim::makeMul(sim::Register::R3, sim::Register::R4),
            sim::makeCmp(sim::Register::R4, sim::Register::R5),
            sim::makeJmp(0x0010),
            sim::makeJe(0x0014),
            sim::makeJne(0x0018),
            sim::makeJg(0x001c),
            sim::makeJl(0x0020),
            sim::makeLoad(sim::Register::R6, 0x8000),
            sim::makeStore(sim::Register::R6, 0x8004),
            sim::makePush(sim::Register::R7),
            sim::makePop(sim::Register::R0),
            sim::makeCall(0x0024),
            sim::makeRet(),
            sim::makeHalt(),
        });
}

void runs_assembled_program()
{
    const sim::Assembler assembler;
    const auto program = assembler.assemble(R"(
        MOV R1, 5
        MOV R2, 7
        ADD R1, R2
        HALT
    )");

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.halted());
    assert(cpu.reg(sim::Register::R1) == 12);
    assert(cpu.reg(sim::Register::R2) == 7);
}

int main()
{
    assembles_empty_source();
    ignores_blank_and_comment_lines();
    assembles_no_operand_instruction();
    reports_scanned_line_number();
    tokenizes_whitespace_and_commas();
    tokenizes_adjacent_comma_operands();
    parses_valid_registers_and_numbers();
    parses_hex_uint16_addresses();
    rejects_invalid_registers();
    rejects_invalid_numbers();
    rejects_out_of_range_int16_values();
    rejects_out_of_range_uint16_values();
    assembles_numeric_instruction_forms();
    runs_assembled_program();
}

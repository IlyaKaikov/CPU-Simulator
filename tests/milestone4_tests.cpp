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

void reports_structured_assembly_error_line()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble(R"(
            MOV R1, 1
            BOGUS
        )");
        (void)program;
    } catch (const sim::AssemblyError& error) {
        threw = true;
        const std::string message = error.what();
        assert(error.line() == 3);
        assert(contains(message, "line 3"));
        assert(contains(message, "unknown instruction 'BOGUS'"));
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

void collects_labels_without_emitting_instructions()
{
    const sim::Assembler assembler;
    const auto program = assembler.assemble(R"(
        start:
        MOV R1, 5
        _loop_1:
        ADD R1, R1
        HALT
    )");

    assertProgramEquals(
        program,
        {
            sim::makeMov(sim::Register::R1, 5),
            sim::makeAdd(sim::Register::R1, sim::Register::R1),
            sim::makeHalt(),
        });
}

void rejects_duplicate_labels()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble(R"(
            loop:
            MOV R1, 1
            loop:
            HALT
        )");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "line 4"));
        assert(contains(message, "duplicate label 'loop'"));
    }

    assert(threw);
}

void rejects_malformed_labels()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble("1bad:");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "line 1"));
        assert(contains(message, "invalid label '1bad'"));
    }

    assert(threw);
}

void rejects_label_and_instruction_on_same_line()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble("loop: MOV R1, 1");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "line 1"));
        assert(contains(message, "labels must be on their own line"));
    }

    assert(threw);
}

void resolves_jump_labels_to_byte_addresses()
{
    const sim::Assembler assembler;
    const auto program = assembler.assemble(R"(
        start:
        JMP end
        MOV R1, 99
        end:
        HALT
    )");

    assertProgramEquals(
        program,
        {
            sim::makeJmp(2 * sim::Memory::instruction_size),
            sim::makeMov(sim::Register::R1, 99),
            sim::makeHalt(),
        });
}

void rejects_unknown_labels()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble("JMP missing");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "line 1"));
        assert(contains(message, "unknown label 'missing'"));
    }

    assert(threw);
}

void reports_unknown_label_as_assembly_error()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble(R"(
            JMP missing
        )");
        (void)program;
    } catch (const sim::AssemblyError& error) {
        threw = true;
        const std::string message = error.what();
        assert(error.line() == 2);
        assert(contains(message, "unknown label 'missing'"));
    }

    assert(threw);
}

void runs_assembled_loop_with_label()
{
    const sim::Assembler assembler;
    const auto program = assembler.assemble(R"(
        MOV R1, 3
        MOV R2, 0
        MOV R3, 1
        loop:
        ADD R2, R1
        SUB R1, R3
        CMP R1, R0
        JG loop
        HALT
    )");

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.halted());
    assert(cpu.reg(sim::Register::R1) == 0);
    assert(cpu.reg(sim::Register::R2) == 6);
    assert(cpu.zeroFlag());
    assert(!cpu.signFlag());
}

void runs_assembled_recursive_program_with_labels()
{
    const sim::Assembler assembler;
    const auto program = assembler.assemble(R"(
        MOV R1, 3
        CALL sum
        HALT

        sum:
        CMP R1, R0
        JE done
        PUSH R1
        MOV R2, 1
        SUB R1, R2
        CALL sum
        POP R2
        ADD R3, R2
        done:
        RET
    )");

    sim::CPU cpu;
    cpu.loadProgram(program);
    cpu.run();

    assert(cpu.halted());
    assert(cpu.reg(sim::Register::R1) == 0);
    assert(cpu.reg(sim::Register::R2) == 3);
    assert(cpu.reg(sim::Register::R3) == 6);
    assert(cpu.sp() == sim::CPU::initial_sp);
}

int main()
{
    assembles_empty_source();
    ignores_blank_and_comment_lines();
    assembles_no_operand_instruction();
    reports_scanned_line_number();
    reports_structured_assembly_error_line();
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
    collects_labels_without_emitting_instructions();
    rejects_duplicate_labels();
    rejects_malformed_labels();
    rejects_label_and_instruction_on_same_line();
    resolves_jump_labels_to_byte_addresses();
    rejects_unknown_labels();
    reports_unknown_label_as_assembly_error();
    runs_assembled_loop_with_label();
    runs_assembled_recursive_program_with_labels();
}

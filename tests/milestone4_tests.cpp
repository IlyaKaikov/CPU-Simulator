#include "assembler/Assembler.hpp"

#include <cassert>
#include <stdexcept>
#include <string>

bool contains(const std::string& text, const std::string& expected)
{
    return text.find(expected) != std::string::npos;
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

void rejects_instruction_text_until_parser_exists()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble("HALT");
        (void)program;
    } catch (const std::runtime_error&) {
        threw = true;
    }

    assert(threw);
}

void reports_scanned_line_number()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble(R"(
            ; comment-only line
            HALT
        )");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "line 3"));
        assert(contains(message, "HALT"));
    }

    assert(threw);
}

void tokenizes_whitespace_and_commas()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble("  MOV   R1,   5  ; initialize R1");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "MOV R1 5"));
    }

    assert(threw);
}

void tokenizes_adjacent_comma_operands()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble("ADD R1,R2");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "ADD R1 R2"));
    }

    assert(threw);
}

void parses_valid_registers_and_numbers()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble("MOV r7, -42");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "instruction parsing is not implemented yet"));
        assert(contains(message, "MOV r7 -42"));
    }

    assert(threw);
}

void parses_hex_uint16_addresses()
{
    const sim::Assembler assembler;
    bool threw = false;

    try {
        const auto program = assembler.assemble("LOAD R2, 0x8000");
        (void)program;
    } catch (const std::runtime_error& error) {
        threw = true;
        const std::string message = error.what();
        assert(contains(message, "instruction parsing is not implemented yet"));
        assert(contains(message, "LOAD R2 0x8000"));
    }

    assert(threw);
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

int main()
{
    assembles_empty_source();
    ignores_blank_and_comment_lines();
    rejects_instruction_text_until_parser_exists();
    reports_scanned_line_number();
    tokenizes_whitespace_and_commas();
    tokenizes_adjacent_comma_operands();
    parses_valid_registers_and_numbers();
    parses_hex_uint16_addresses();
    rejects_invalid_registers();
    rejects_invalid_numbers();
    rejects_out_of_range_int16_values();
    rejects_out_of_range_uint16_values();
}

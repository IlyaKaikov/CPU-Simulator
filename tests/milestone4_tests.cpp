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

int main()
{
    assembles_empty_source();
    ignores_blank_and_comment_lines();
    rejects_instruction_text_until_parser_exists();
    reports_scanned_line_number();
    tokenizes_whitespace_and_commas();
    tokenizes_adjacent_comma_operands();
}

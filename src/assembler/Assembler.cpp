#include "assembler/Assembler.hpp"
#include "memory/Memory.hpp"

#include <cctype>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace sim {

namespace {

struct ScannedLine {
    std::size_t lineNumber{};
    std::vector<std::string> tokens{};
};

struct AssemblyPlan {
    std::vector<ScannedLine> instructions{};
    std::unordered_map<std::string, std::uint16_t> labels{};
};

std::string_view stripComment(std::string_view line)
{
    const auto comment = line.find(';');
    if (comment == std::string_view::npos) {
        return line;
    }

    return line.substr(0, comment);
}

std::string_view trim(std::string_view value)
{
    const auto first = value.find_first_not_of(" \t\r");
    if (first == std::string_view::npos) {
        return {};
    }

    const auto last = value.find_last_not_of(" \t\r");
    return value.substr(first, last - first + 1);
}

std::vector<std::string> tokenize(std::string_view line)
{
    std::vector<std::string> tokens;
    std::size_t tokenStart = std::string_view::npos;

    for (std::size_t index = 0; index <= line.size(); ++index) {
        const auto atEnd = index == line.size();
        const auto separator = !atEnd && (line.at(index) == ',' || line.at(index) == ' ' || line.at(index) == '\t');

        if ((atEnd || separator) && tokenStart != std::string_view::npos) {
            tokens.emplace_back(line.substr(tokenStart, index - tokenStart));
            tokenStart = std::string_view::npos;
        } else if (!atEnd && !separator && tokenStart == std::string_view::npos) {
            tokenStart = index;
        }
    }

    return tokens;
}

std::string toUpper(std::string_view value)
{
    std::string result;
    result.reserve(value.size());
    for (const auto ch : value) {
        result += static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }

    return result;
}

[[noreturn]] void throwAtLine(std::size_t lineNumber, const std::string& message)
{
    throw std::runtime_error("line " + std::to_string(lineNumber) + ": " + message);
}

bool isIdentifierStart(char ch)
{
    const auto value = static_cast<unsigned char>(ch);
    return std::isalpha(value) || ch == '_';
}

bool isIdentifierPart(char ch)
{
    const auto value = static_cast<unsigned char>(ch);
    return std::isalnum(value) || ch == '_';
}

void validateLabelName(std::string_view label, std::size_t lineNumber)
{
    if (label.empty() || !isIdentifierStart(label.front())) {
        throwAtLine(lineNumber, "invalid label '" + std::string(label) + "'");
    }

    for (const auto ch : label.substr(1)) {
        if (!isIdentifierPart(ch)) {
            throwAtLine(lineNumber, "invalid label '" + std::string(label) + "'");
        }
    }
}

bool endsWithColon(std::string_view value)
{
    return !value.empty() && value.back() == ':';
}

bool containsColon(std::string_view value)
{
    return value.find(':') != std::string_view::npos;
}

Register parseRegister(std::string_view token, std::size_t lineNumber)
{
    const auto upper = toUpper(token);
    if (upper.size() != 2 || upper.at(0) != 'R' || upper.at(1) < '0' || upper.at(1) > '7') {
        throwAtLine(lineNumber, "invalid register '" + std::string(token) + "'");
    }

    return static_cast<Register>(upper.at(1) - '0');
}

int digitValue(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }

    return -1;
}

std::int64_t parseInteger(
    std::string_view token,
    std::int64_t minimum,
    std::int64_t maximum,
    std::size_t lineNumber,
    std::string_view description)
{
    if (token.empty()) {
        throwAtLine(lineNumber, "missing " + std::string(description));
    }

    std::size_t index = 0;
    bool negative = false;
    if (token.at(index) == '-') {
        negative = true;
        ++index;
    }

    auto base = 10;
    if (index + 2 <= token.size() && token.at(index) == '0' &&
        (token.at(index + 1) == 'x' || token.at(index + 1) == 'X')) {
        base = 16;
        index += 2;
    }

    if (index == token.size()) {
        throwAtLine(lineNumber, "invalid " + std::string(description) + " '" + std::string(token) + "'");
    }

    std::uint64_t value = 0;
    for (; index < token.size(); ++index) {
        const auto digit = digitValue(token.at(index));
        if (digit < 0 || digit >= base) {
            throwAtLine(lineNumber, "invalid " + std::string(description) + " '" + std::string(token) + "'");
        }

        value = value * static_cast<std::uint64_t>(base) + static_cast<std::uint64_t>(digit);
        if (value > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            throwAtLine(lineNumber, std::string(description) + " out of range '" + std::string(token) + "'");
        }
    }

    const auto signedValue = negative ? -static_cast<std::int64_t>(value) : static_cast<std::int64_t>(value);
    if (signedValue < minimum || signedValue > maximum) {
        throwAtLine(lineNumber, std::string(description) + " out of range '" + std::string(token) + "'");
    }

    return signedValue;
}

std::int16_t parseInt16(std::string_view token, std::size_t lineNumber)
{
    return static_cast<std::int16_t>(parseInteger(
        token,
        std::numeric_limits<std::int16_t>::min(),
        std::numeric_limits<std::int16_t>::max(),
        lineNumber,
        "int16"));
}

std::uint16_t parseUInt16(std::string_view token, std::size_t lineNumber)
{
    return static_cast<std::uint16_t>(parseInteger(
        token,
        0,
        std::numeric_limits<std::uint16_t>::max(),
        lineNumber,
        "uint16"));
}

bool looksNumeric(std::string_view token)
{
    return !token.empty() && (std::isdigit(static_cast<unsigned char>(token.front())) || token.front() == '-');
}

std::uint16_t parseTarget(
    std::string_view token,
    std::size_t lineNumber,
    const std::unordered_map<std::string, std::uint16_t>& labels)
{
    if (looksNumeric(token)) {
        return parseUInt16(token, lineNumber);
    }

    validateLabelName(token, lineNumber);
    const auto label = labels.find(std::string(token));
    if (label == labels.end()) {
        throwAtLine(lineNumber, "unknown label '" + std::string(token) + "'");
    }

    return label->second;
}

void requireOperandCount(const ScannedLine& line, std::size_t expected)
{
    if (line.tokens.size() != expected) {
        throwAtLine(
            line.lineNumber,
            "expected " + std::to_string(expected - 1) + " operand(s), got " +
                std::to_string(line.tokens.size() - 1));
    }
}

EncodedInstruction makeNoOperandInstruction(OpCode opcode)
{
    return EncodedInstruction{
        static_cast<std::uint8_t>(opcode),
        0,
        0,
    };
}

EncodedInstruction parseInstruction(const ScannedLine& line, const std::unordered_map<std::string, std::uint16_t>& labels)
{
    const auto opcode = toUpper(line.tokens.front());

    if (opcode == "NOP") {
        requireOperandCount(line, 1);
        return makeNoOperandInstruction(OpCode::Nop);
    }
    if (opcode == "RET") {
        requireOperandCount(line, 1);
        return makeRet();
    }
    if (opcode == "HALT") {
        requireOperandCount(line, 1);
        return makeHalt();
    }
    if (opcode == "MOV") {
        requireOperandCount(line, 3);
        return makeMov(parseRegister(line.tokens.at(1), line.lineNumber), parseInt16(line.tokens.at(2), line.lineNumber));
    }
    if (opcode == "ADD") {
        requireOperandCount(line, 3);
        return makeAdd(parseRegister(line.tokens.at(1), line.lineNumber), parseRegister(line.tokens.at(2), line.lineNumber));
    }
    if (opcode == "SUB") {
        requireOperandCount(line, 3);
        return makeSub(parseRegister(line.tokens.at(1), line.lineNumber), parseRegister(line.tokens.at(2), line.lineNumber));
    }
    if (opcode == "MUL") {
        requireOperandCount(line, 3);
        return makeMul(parseRegister(line.tokens.at(1), line.lineNumber), parseRegister(line.tokens.at(2), line.lineNumber));
    }
    if (opcode == "CMP") {
        requireOperandCount(line, 3);
        return makeCmp(parseRegister(line.tokens.at(1), line.lineNumber), parseRegister(line.tokens.at(2), line.lineNumber));
    }
    if (opcode == "JMP") {
        requireOperandCount(line, 2);
        return makeJmp(parseTarget(line.tokens.at(1), line.lineNumber, labels));
    }
    if (opcode == "JE") {
        requireOperandCount(line, 2);
        return makeJe(parseTarget(line.tokens.at(1), line.lineNumber, labels));
    }
    if (opcode == "JNE") {
        requireOperandCount(line, 2);
        return makeJne(parseTarget(line.tokens.at(1), line.lineNumber, labels));
    }
    if (opcode == "JG") {
        requireOperandCount(line, 2);
        return makeJg(parseTarget(line.tokens.at(1), line.lineNumber, labels));
    }
    if (opcode == "JL") {
        requireOperandCount(line, 2);
        return makeJl(parseTarget(line.tokens.at(1), line.lineNumber, labels));
    }
    if (opcode == "LOAD") {
        requireOperandCount(line, 3);
        return makeLoad(parseRegister(line.tokens.at(1), line.lineNumber), parseUInt16(line.tokens.at(2), line.lineNumber));
    }
    if (opcode == "STORE") {
        requireOperandCount(line, 3);
        return makeStore(parseRegister(line.tokens.at(1), line.lineNumber), parseUInt16(line.tokens.at(2), line.lineNumber));
    }
    if (opcode == "PUSH") {
        requireOperandCount(line, 2);
        return makePush(parseRegister(line.tokens.at(1), line.lineNumber));
    }
    if (opcode == "POP") {
        requireOperandCount(line, 2);
        return makePop(parseRegister(line.tokens.at(1), line.lineNumber));
    }
    if (opcode == "CALL") {
        requireOperandCount(line, 2);
        return makeCall(parseTarget(line.tokens.at(1), line.lineNumber, labels));
    }

    throwAtLine(line.lineNumber, "unknown instruction '" + line.tokens.front() + "'");
}

std::vector<ScannedLine> scanSource(std::string_view source)
{
    std::vector<ScannedLine> lines;
    std::size_t lineNumber = 1;
    std::size_t lineStart = 0;

    while (lineStart <= source.size()) {
        const auto lineEnd = source.find('\n', lineStart);
        const auto length = lineEnd == std::string_view::npos ? source.size() - lineStart : lineEnd - lineStart;
        const auto line = trim(stripComment(source.substr(lineStart, length)));

        if (!line.empty()) {
            lines.push_back(ScannedLine{
                lineNumber,
                tokenize(line),
            });
        }

        if (lineEnd == std::string_view::npos) {
            break;
        }

        ++lineNumber;
        lineStart = lineEnd + 1;
    }

    return lines;
}

AssemblyPlan collectLabels(const std::vector<ScannedLine>& lines)
{
    AssemblyPlan plan;
    std::uint32_t address = 0;

    for (const auto& line : lines) {
        if (line.tokens.size() == 1 && endsWithColon(line.tokens.front())) {
            const auto label = line.tokens.front().substr(0, line.tokens.front().size() - 1);
            validateLabelName(label, line.lineNumber);

            if (plan.labels.contains(label)) {
                throwAtLine(line.lineNumber, "duplicate label '" + label + "'");
            }
            if (address > std::numeric_limits<std::uint16_t>::max()) {
                throwAtLine(line.lineNumber, "label address out of range '" + label + "'");
            }

            plan.labels.emplace(label, static_cast<std::uint16_t>(address));
        } else {
            for (const auto& token : line.tokens) {
                if (containsColon(token)) {
                    throwAtLine(line.lineNumber, "labels must be on their own line");
                }
            }

            plan.instructions.push_back(line);
            address += Memory::instruction_size;
        }
    }

    return plan;
}

}

std::vector<EncodedInstruction> Assembler::assemble(std::string_view source) const
{
    const auto plan = collectLabels(scanSource(source));
    std::vector<EncodedInstruction> program;
    program.reserve(plan.instructions.size());

    for (const auto& line : plan.instructions) {
        program.push_back(parseInstruction(line, plan.labels));
    }

    return program;
}

}

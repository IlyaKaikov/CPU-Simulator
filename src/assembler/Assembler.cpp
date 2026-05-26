#include "assembler/Assembler.hpp"

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace sim {

namespace {

struct ScannedLine {
    std::size_t lineNumber{};
    std::vector<std::string> tokens{};
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

std::string joinTokens(const std::vector<std::string>& tokens)
{
    std::string result;
    for (const auto& token : tokens) {
        if (!result.empty()) {
            result += ' ';
        }
        result += token;
    }

    return result;
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

}

std::vector<EncodedInstruction> Assembler::assemble(std::string_view source) const
{
    const auto lines = scanSource(source);
    if (!lines.empty()) {
        throw std::runtime_error(
            "assembler instruction parsing is not implemented yet at line " +
            std::to_string(lines.front().lineNumber) + ": " + joinTokens(lines.front().tokens));
    }

    return {};
}

}

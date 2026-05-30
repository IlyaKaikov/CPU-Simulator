#pragma once

#include "isa/Instruction.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace sim {

class AssemblyError : public std::runtime_error {
public:
    AssemblyError(std::size_t line, const std::string& message);

    [[nodiscard]] std::size_t line() const noexcept;

private:
    std::size_t line_{};
};

class Assembler {
public:
    [[nodiscard]] std::vector<EncodedInstruction> assemble(std::string_view source) const;
};

}

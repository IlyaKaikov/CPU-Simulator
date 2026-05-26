#pragma once

#include "isa/Instruction.hpp"

#include <string_view>
#include <vector>

namespace sim {

class Assembler {
public:
    [[nodiscard]] std::vector<EncodedInstruction> assemble(std::string_view source) const;
};

}

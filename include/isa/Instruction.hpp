#pragma once

#include <cstdint>

namespace sim {

enum class OpCode : std::uint8_t {
    Nop = 0x00,
    Mov = 0x01,
    Add = 0x02,
    Sub = 0x03,
    Mul = 0x04,
    Halt = 0xff,
};

enum class Register : std::uint8_t {
    R0 = 0,
    R1 = 1,
    R2 = 2,
    R3 = 3,
    R4 = 4,
    R5 = 5,
    R6 = 6,
    R7 = 7,
};

struct EncodedInstruction {
    std::uint8_t opcode{};
    std::uint8_t a{};
    std::int16_t b{};
};

EncodedInstruction makeMov(Register destination, std::int16_t immediate);
EncodedInstruction makeAdd(Register destination, Register source);
EncodedInstruction makeSub(Register destination, Register source);
EncodedInstruction makeMul(Register destination, Register source);
EncodedInstruction makeHalt();

}

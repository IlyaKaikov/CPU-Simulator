#include "isa/Instruction.hpp"

namespace sim {

EncodedInstruction makeMov(Register destination, std::int16_t immediate)
{
    return EncodedInstruction{
        static_cast<std::uint8_t>(OpCode::Mov),
        static_cast<std::uint8_t>(destination),
        immediate,
    };
}

EncodedInstruction makeAdd(Register destination, Register source)
{
    return EncodedInstruction{
        static_cast<std::uint8_t>(OpCode::Add),
        static_cast<std::uint8_t>(destination),
        static_cast<std::int16_t>(source),
    };
}

EncodedInstruction makeHalt()
{
    return EncodedInstruction{
        static_cast<std::uint8_t>(OpCode::Halt),
        0,
        0,
    };
}

}

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

EncodedInstruction makeSub(Register destination, Register source)
{
    return EncodedInstruction{
        static_cast<std::uint8_t>(OpCode::Sub),
        static_cast<std::uint8_t>(destination),
        static_cast<std::int16_t>(source),
    };
}

EncodedInstruction makeMul(Register destination, Register source)
{
    return EncodedInstruction{
        static_cast<std::uint8_t>(OpCode::Mul),
        static_cast<std::uint8_t>(destination),
        static_cast<std::int16_t>(source),
    };
}

EncodedInstruction makeCmp(Register left, Register right)
{
    return EncodedInstruction{
        static_cast<std::uint8_t>(OpCode::Cmp),
        static_cast<std::uint8_t>(left),
        static_cast<std::int16_t>(right),
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

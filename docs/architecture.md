# Architecture Plan

## Main Modules
```text
cpu/
memory/
isa/
assembler/
debugger/
loader/
tests/
examples/
```

## CPU Execution Flow
```text
Fetch → Decode → Execute
```

## Recommended Core Types

### OpCode
Enum of all instructions.

### EncodedInstruction
```cpp
struct EncodedInstruction {
    uint8_t opcode;
    uint8_t a;
    int16_t b;
};
```

### CPU
Responsibilities:
- execute instructions
- maintain registers
- update PC/SP
- update flags
- manage execution state

### Memory
Responsibilities:
- read/write bytes
- read/write instructions
- stack operations

### Assembler
Responsibilities:
- parse assembly
- resolve labels
- encode instructions

### Debugger
Responsibilities:
- stepping
- breakpoints
- memory/register inspection
- execution tracing
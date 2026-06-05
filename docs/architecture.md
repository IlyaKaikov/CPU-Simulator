# Architecture Plan

## Main Modules
```text
cpu/
memory/
isa/
assembler/
binary/
debugger/
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

### Binary Program Format
Responsibilities:
- write assembled programs to versioned binary files
- validate binary headers
- load binary files back into `EncodedInstruction` programs

### Debugger
Responsibilities:
- stepping
- continuing until halt, breakpoint, or step budget
- breakpoints
- memory/register inspection
- execution tracing
- deterministic CLI debug output for binary programs

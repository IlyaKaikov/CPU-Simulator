# Milestone Roadmap

## Milestone 1 - CPU Skeleton - Complete

Implemented:
- registers
- memory
- instruction decoding
- MOV
- ADD
- HALT

Goal:
```asm
MOV R1, 5
MOV R2, 7
ADD R1, R2
HALT
```

Expected:
```text
R1 = 12
```

---

## Milestone 2 - Control Flow - Complete

Implemented:
- SUB
- MUL
- CMP
- flags
- JMP / JE / JNE / JG / JL

Goal:
- loops
- conditional branching

---

## Milestone 3 - Memory + Stack - Complete

Implemented:
- LOAD
- STORE
- PUSH
- POP
- CALL
- RET

Goal:
- function calls
- recursion support

---

## Milestone 4 - Assembler - Complete

Implemented:
- lexer/parser
- comments and blank lines
- decimal and hex numbers
- labels
- instruction generation as `EncodedInstruction` values
- useful parse errors with line numbers

Goal:
- assemble `.asm` text directly into programs the CPU can run
- express milestone 2 loops and milestone 3 recursive calls in assembly text

---

## Milestone 5 - Binary Format + Loader - Complete

Implemented:
- binary file format
- binary writer
- binary reader/loader
- magic/version/header validation
- CLI path for assembling source to binary

Goal:
- assemble a `.asm` file into a binary program file
- load that binary program into the CPU

---

## Milestone 6 - Debugger - Complete

Implemented:
- step
- continue
- breakpoints
- register dump
- memory dump
- execution tracing
- deterministic CLI debug command

---

## Milestone 7 - Polish - Complete

Implemented:
- final README pass
- documentation cleanup
- architecture diagrams
- example program cleanup
- final build/test/CLI validation

Deferred optional ideas:
- disassembler
- performance counters

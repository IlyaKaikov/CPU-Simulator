# Milestone Roadmap

## Milestone 1 - CPU Skeleton
Implement:
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

## Milestone 2 - Control Flow
Implement:
- SUB
- MUL
- CMP
- flags
- JMP / JE / JNE / JG / JL

Goal:
- loops
- conditional branching

---

## Milestone 3 - Memory + Stack
Implement:
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

## Milestone 4 - Assembler
Implement:
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

## Milestone 5 - Binary Format + Loader
Implement:
- binary file format
- binary writer
- binary reader/loader
- magic/version/header validation
- CLI path for assembling source to binary

Goal:
- assemble a `.asm` file into a binary program file
- load that binary program into the CPU

---

## Milestone 6 - Debugger
Implement:
- step
- continue
- breakpoints
- register dump
- memory dump
- execution tracing

---

## Milestone 7 - Polish
Implement:
- tests
- example programs
- README
- architecture diagrams
- disassembler (optional)
- performance counters (optional)

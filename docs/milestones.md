# Milestone Roadmap

## Milestone 1 — CPU Skeleton
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

## Milestone 2 — Control Flow
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

## Milestone 3 — Memory + Stack
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

## Milestone 4 — Assembler
Implement:
- lexer/parser
- labels
- instruction encoding
- binary generation

---

## Milestone 5 — Debugger
Implement:
- step
- continue
- breakpoints
- register dump
- memory dump
- execution tracing

---

## Milestone 6 — Polish
Implement:
- tests
- example programs
- README
- architecture diagrams
- disassembler (optional)
- performance counters (optional)
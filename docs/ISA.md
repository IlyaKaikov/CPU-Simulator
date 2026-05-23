# ISA Specification

## Registers
General-purpose:
- R0–R7

Special:
- PC (program counter)
- SP (stack pointer)

## Memory
- 64 KB RAM
- Byte-addressable
- Stack grows downward

Suggested layout:
- 0x0000–0x7FFF : code
- 0x8000–0xEFFF : data
- 0xF000–0xFFFF : stack

Initial state:
- PC = 0x0000
- SP = 0x10000

## Flags
- ZF (zero flag)
- SF (sign flag)

## Word Size
- 32-bit signed integers (`int32_t`)

## Instruction Size
- Fixed-width: 4 bytes

## Core Instructions

### Data Movement
- MOV
- LOAD
- STORE

### Arithmetic
- ADD
- SUB
- MUL

### Comparison / Branching
- CMP
- JMP
- JE
- JNE
- JG
- JL

### Stack / Functions
- PUSH
- POP
- CALL
- RET

### Control
- NOP
- HALT

## Example
```asm
MOV R1, 5
MOV R2, 7
ADD R1, R2
HALT
```
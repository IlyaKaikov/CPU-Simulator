# CPU Simulator

A small C++20 RISC-style CPU simulator with a simple assembler.

The simulator currently supports:
- 8 general-purpose registers: `R0`-`R7`
- 64 KB byte-addressable memory
- fixed-width 4-byte encoded instructions
- flags: zero flag (`ZF`) and sign flag (`SF`)
- memory words as signed 32-bit little-endian values
- downward-growing stack
- assembler input from `.asm` text

## Implemented ISA

```text
NOP
MOV
ADD
SUB
MUL
CMP
JMP
JE
JNE
JG
JL
LOAD
STORE
PUSH
POP
CALL
RET
HALT
```

The assembler supports comments, blank lines, decimal numbers, hex numbers, labels, and line-numbered `AssemblyError` diagnostics.

Example:

```asm
MOV R1, 3
MOV R2, 0
MOV R3, 1

loop:
ADD R2, R1
SUB R1, R3
CMP R1, R0
JG loop

HALT
```

## Build

Install a C++20 compiler and CMake.

The current local build uses MinGW:

```powershell
cmake --build build-mingw
ctest --test-dir build-mingw --output-on-failure
```

For a fresh build directory:

```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles"
cmake --build build-mingw
ctest --test-dir build-mingw --output-on-failure
```

The project also has a default CMake preset for other generators:

```powershell
cmake --preset default
cmake --build --preset default
ctest --preset default -C Debug
```

## Run Demo

The demo executable discovers every `.asm` file in `examples/`, assembles each file, runs it on a fresh CPU, and prints the final CPU state.

```powershell
.\build-mingw\cpu_sim.exe
```

Example output shape:

```text
examples\milestone1.asm
  halted = 1
  R0 = 0
  R1 = 12
  R2 = 7
  ...
  ZF = 0
  SF = 0
  PC = 0x10
  SP = 0x10000
```

## Tests

Tests are split by milestone:

```text
milestone2_tests
milestone3_tests
milestone4_tests
```

Run all tests:

```powershell
ctest --test-dir build-mingw --output-on-failure
```

## Roadmap

See [docs/milestones.md](docs/milestones.md).

Current state:
- Milestone 1: CPU skeleton - complete
- Milestone 2: control flow - complete
- Milestone 3: memory + stack - complete
- Milestone 4: assembler to `EncodedInstruction` programs - complete
- Milestone 5 next: binary format + loader

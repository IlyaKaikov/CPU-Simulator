# CPU Simulator

A C++20 RISC-style CPU simulator with an assembler, binary program format, loader, and debugger.

This project demonstrates:
- fixed-width instruction encoding
- register, memory, flag, stack, and call/return execution
- assembly parsing with labels and useful line-numbered errors
- binary program serialization and validation
- debugger control with stepping, continue, breakpoints, dumps, and tracing
- clean CMake-based project structure and milestone-focused tests

## Features

- 8 general-purpose registers: `R0`-`R7`
- 64 KB byte-addressable memory
- fixed-width 4-byte encoded instructions
- signed 32-bit memory words
- zero and sign flags
- downward-growing stack
- `.asm` source assembly
- versioned `CSIM` binary program files
- deterministic debugger CLI output

## ISA

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

See [docs/ISA.md](docs/ISA.md) for the ISA summary and [docs/binary_format.md](docs/binary_format.md) for the binary format.

## Architecture

Main modules:
- `cpu`: fetch/decode/execute loop and register/flag state
- `memory`: byte, word, and instruction storage
- `isa`: opcodes and instruction constructors
- `assembler`: source parsing and label resolution
- `binary`: binary writer/reader and header validation
- `debugger`: stepping, continue, breakpoints, dumps, and tracing

## Build And Test

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

## CLI

Run every `.asm` example in `examples/`:

```powershell
.\build-mingw\cpu_sim.exe
```

Assemble source into a binary program:

```powershell
.\build-mingw\cpu_sim.exe assemble examples\milestone6_debug_trace.asm build-mingw\milestone6_debug_trace.bin
```

Run a binary program:

```powershell
.\build-mingw\cpu_sim.exe run build-mingw\milestone6_debug_trace.bin
```

Debug a binary program and print the final register dump plus execution trace:

```powershell
.\build-mingw\cpu_sim.exe debug build-mingw\milestone6_debug_trace.bin
```

Run/debug output includes final CPU state such as registers, flags, `PC`, and `SP`. Debug output also includes trace rows with `pcBefore`, `pcAfter`, opcode, and operands.

## Examples

The `examples/` directory includes small assembly programs covering:
- milestone 1 arithmetic basics
- milestone 2 branching and comparisons
- milestone 3 memory and stack operations
- milestone 5 binary round trips
- milestone 6 debugger tracing

## Tests

Tests are split by milestone:

```text
milestone2_tests
milestone3_tests
milestone4_tests
milestone5_tests
milestone6_tests
```

Run all tests:

```powershell
ctest --test-dir build-mingw --output-on-failure
```

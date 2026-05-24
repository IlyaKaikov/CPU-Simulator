# CPU Simulator

A small C++20 RISC-style CPU simulator.

## Build

Install a C++20 compiler and CMake, then run:

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

On Windows, good options are:
- Visual Studio Build Tools with the C++ workload
- MSYS2/MinGW-w64 with CMake
- LLVM/Clang with CMake

## Run Demo

```powershell
.\build\Debug\cpu_sim.exe
```

Expected output:

```text
R1 = 21
```

# Example Programs

These assembly files are small fixtures for manual testing and assembler/binary-loader workflows.

Implemented examples:
- `milestone1.asm`: basic `MOV`, `ADD`, and `HALT`
- `milestone2_arithmetic.asm`: arithmetic operations
- `milestone2_cmp_equal.asm`: equality comparison and conditional branch
- `milestone2_cmp_less.asm`: less-than comparison and conditional branch
- `milestone2_jmp_skip.asm`: unconditional jump
- `milestone2_loop_sum.asm`: loop that sums 3 + 2 + 1
- `milestone2_countdown.asm`: countdown loop
- `milestone3_memory_preview.asm`: `STORE` and `LOAD`
- `milestone3_push_preview.asm`: repeated `PUSH` and `POP`
- `milestone3_stack_preview.asm`: stack order with two registers
- `milestone5_binary_roundtrip.asm`: binary writer/loader fixture
- `milestone6_debug_trace.asm`: debugger trace fixture

Run all assembly examples:

```powershell
.\build-mingw\cpu_sim.exe
```

Assemble and run a binary fixture:

```powershell
.\build-mingw\cpu_sim.exe assemble examples\milestone5_binary_roundtrip.asm build-mingw\milestone5_binary_roundtrip.bin
.\build-mingw\cpu_sim.exe run build-mingw\milestone5_binary_roundtrip.bin
```

Assemble and inspect a debugger trace fixture:

```powershell
.\build-mingw\cpu_sim.exe assemble examples\milestone6_debug_trace.asm build-mingw\milestone6_debug_trace.bin
.\build-mingw\cpu_sim.exe debug build-mingw\milestone6_debug_trace.bin
```

# Example Programs

These assembly files are small fixtures for manual testing and assembler/binary-loader workflows.

Implemented milestone examples:
- `milestone1.asm`
- `milestone2_arithmetic.asm`
- `milestone2_cmp_equal.asm`
- `milestone2_cmp_less.asm`
- `milestone2_jmp_skip.asm`
- `milestone2_loop_sum.asm`
- `milestone2_countdown.asm`
- `milestone3_memory_preview.asm`
- `milestone3_push_preview.asm`
- `milestone3_stack_preview.asm`
- `milestone5_binary_roundtrip.asm`

The milestone 5 fixture can be assembled and run as a binary program:

```powershell
.\build-mingw\cpu_sim.exe assemble examples\milestone5_binary_roundtrip.asm build-mingw\milestone5_binary_roundtrip.bin
.\build-mingw\cpu_sim.exe run build-mingw\milestone5_binary_roundtrip.bin
```

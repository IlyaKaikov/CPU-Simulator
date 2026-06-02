# Binary Program Format

Milestone 5 adds a versioned binary program file that stores assembled `EncodedInstruction` values.

All multi-byte fields are little-endian.

## Header

```text
offset  size  field
0       4     magic: "CSIM"
4       2     version: 1
6       2     header size: 16
8       4     instruction count
12      4     entry point: 0
```

The loader validates the magic, version, header size, entry point, and exact file size before decoding instructions.

## Body

The body contains `instruction count` fixed-width instructions. Each instruction is 4 bytes:

```text
offset  size  field
0       1     opcode
1       1     a
2       2     b
```

The instruction body uses the same byte layout as `Memory::writeInstruction`.

## CLI

Assemble source text into a binary program:

```powershell
.\build-mingw\cpu_sim.exe assemble examples\milestone5_binary_roundtrip.asm build-mingw\milestone5_binary_roundtrip.bin
```

Run a binary program:

```powershell
.\build-mingw\cpu_sim.exe run build-mingw\milestone5_binary_roundtrip.bin
```

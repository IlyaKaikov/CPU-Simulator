; Expected: R1 = 1, R2 = 7
MOV R1, 1
JMP target
MOV R1, 99
HALT
target:
MOV R2, 7
HALT

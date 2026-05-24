; Expected: R1 = 0, ZF = 1, SF = 0
MOV R1, 5
MOV R2, 1

loop:
SUB R1, R2
CMP R1, R0
JG loop

HALT

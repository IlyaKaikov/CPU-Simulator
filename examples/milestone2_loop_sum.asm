; Expected: R1 = 0, R2 = 6, ZF = 1, SF = 0
MOV R1, 3
MOV R2, 0
MOV R3, 1

loop:
ADD R2, R1
SUB R1, R3
CMP R1, R0
JG loop

HALT

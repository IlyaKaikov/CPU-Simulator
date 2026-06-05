; Demonstrates stack order with two registers.
; Expected: R1 = 7, R2 = 10
MOV R1, 10
MOV R2, 7
PUSH R1
PUSH R2
POP R1
POP R2
HALT

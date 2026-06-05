; Demonstrates repeated PUSH/POP.
; Expected: R1 = 10
MOV R1, 10
PUSH R1
PUSH R1
POP R1
HALT

; Future milestone 3 target.
; Expected after LOAD/STORE exist: R2 = 123
MOV R1, 123
STORE R1, 0x8000
LOAD R2, 0x8000
HALT

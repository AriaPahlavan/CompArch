        .ORIG x1200

        ADD R6, R6, #-2     ;0x1200
        STW R0, R6, #0      ;0x1202
        ADD R6, R6, #-2     ;0x1204
        STW R1, R6, #0      ;0x1206
        ADD R6, R6, #-2     ;0x1208
        STW R2, R6, #0      ;0x120A

        LEA R0, PTBR        ;0x120C
        LDW R0, R0, #0      ;0x120E            R0 = base

        LEA R1, PTLR        ;0x1210
        LDW R1, R1, #1      ;0x1212            R1 = length

LOOP    LDW R2, R0, #0      ;0x1214             ; clear bit 0 of R2 (PTE)
        AND R2, R2, #-2     ;0x1216             ; and store it back
        STW R2, R0, #0      ;0x1218             ; in R0 (PTE ADDR)
        ADD R0, R0, #2      ;0x121A
        ADD R1, R1, #-1     ;0x121C
        BRp LOOP            ;0x121E

        LDW R2, R6, #0      ;0x1220
        ADD R6, R6, #2      ;0x1222
        LDW R1, R6, #0      ;0x1224
        ADD R6, R6, #2      ;0x1226
        LDW R0, R6, #0      ;0x1228
        ADD R6, R6, #2      ;0x122A

        RTI                 ;0x122C

PTBR    .FILL x1000         ;0x122E
PTLR    .FILL #128          ;0x1230
	    .END

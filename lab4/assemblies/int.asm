        .ORIG x1200

        ADD R6, R6, #-2 ;0x1200
        STW R0, R6, #0  ;0x1202
        ADD R6, R6, #-2 ;0x1204
        STW R1, R6, #0  ;0x1206

        LEA R0, TARGET  ;0x1208
        LDW R0, R0, #0  ;0x120A
        LDW R1, R0, #0  ;0x120C           R1=MEM[x4000]
        ADD R1, R1, #1  ;0x120E
        STW R1, R0, #0  ;0x1210

        LDW R1, R6, #0  ;0x1212
        ADD R6, R6, #2  ;0x1214
        LDW R0, R6, #0  ;0x1216
        ADD R6, R6, #2  ;0x1218

        RTI             ;0x121A

TARGET  .FILL x4000     ;0x121C
	.END

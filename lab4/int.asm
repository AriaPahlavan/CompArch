        .ORIG x1200

        ADD R6, R6, #-2
        STW R0, R6, #0
        ADD R6, R6, #-2
        STW R1, R6, #0

        LEA R0, TARGET
        LDW R0, R0, #0
        LDW R1, R0, #0  ; R1=MEM[x4000]
        ADD R1, R1, #1
        STW R1, R0, #0

        LDW R1, R6, #0
        ADD R6, R6, #2
        LDW R0, R6, #0
        ADD R6, R6, #2

        RTI

TARGET  .FILL x4000
	.END

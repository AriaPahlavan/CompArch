	    .ORIG x3000
	    AND R0, R0, #0          ;0x3000
	    AND R1, R1, #0          ;0x3002
	    AND R2, R2, #0          ;0x3004
	    AND R3, R3, #0          ;0x3006

	    ADD R0, R0, #1          ;0x3008
	    LEA R1, INIT            ;0x300A
	    LDW R1, R1, #0          ;0x300C
	    STW R0, R1, #0          ;0x300E

	    LEA R0, COUNTER         ;0x3010
	    LDW R0, R0, #0; R0 = 20 ;0x3012
	    AND R2, R2, #0          ;0x3014

	    LEA R1, DATA            ;0x3016
	    LDW R1, R1, #0          ;0x3018
LOOP	LDB R3, R1, #0          ;0x301A
	    ADD R2, R2, R3          ;0x301C
	    ADD R1, R1, #1          ;0x301E
	    ADD R0, R0, #-1         ;0x3020
	    BRp LOOP                ;0x3022
	    LEA R0, STORE           ;0x3024
	    LDW R0, R0, #0          ;0x3026
	    STW R2, R0, #0          ;0x3028

	    HALT                    ;0x302A
STORE	.FILL x0000             ;0x302C
COUNTER	.FILL x0014             ;0x302E
DATA	.FILL xC000             ;0x3030
INIT	.FILL x4000             ;0x3032
	.END

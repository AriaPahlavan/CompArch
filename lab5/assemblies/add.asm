	    .ORIG x3000
	    AND R0, R0, #0          ;0x3000
	    AND R1, R1, #0          ;0x3002
	    AND R2, R2, #0          ;0x3004
	    AND R3, R3, #0          ;0x3006

	    LEA R0, COUNTER         ;0x3008
	    LDW R0, R0, #0          ;0x300A

	    LEA R1, DATA            ;0x300C
	    LDW R1, R1, #0          ;0x300E

LOOP	LDB R3, R1, #0          ;0x3010
	    ADD R2, R2, R3          ;0x3012
	    ADD R1, R1, #1          ;0x3014
	    ADD R0, R0, #-1         ;0x3016
	    BRp LOOP                ;0x3018

	    LEA R0, STORE           ;0x301A
	    LDW R0, R0, #0          ;0x301C
	    STW R2, R0, #0          ;0x301E

	    JMP R2                  ;0x3020
	    HALT                    ;0x3022

STORE	.FILL xC014             ;0x3024
COUNTER	.FILL x0014             ;0x3026
DATA	.FILL xC000             ;0x3028
	.END

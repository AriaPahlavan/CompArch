            .ORIG x3000             ;
	        AND R1, R1, #0          ;0    (0x3000
	        AND R2, R2, #0          ;1    (0x3002
	        AND R5, R5, #0          ;2    (0x3004  ;counter
                                    ;
	        LEA R0, TOTAL           ;3    (0x3006
	        LDW R3, R0, #0          ;4    (0x3008   ;R3=256
                                    ;
	        LEA R0, NUMSLOC         ;5    (0x300A
	        LDW R0, R0, #0          ;6    (0x300C    ;R0=x4000
	                                ;
FETCH	    LDB R1, R0, #0          ;7    (0x300E
	        LDB R2, R0, #1          ;8    (0x3010
	        ADD R0, R0, #2          ;9    (0x3012
	                                ;
	        AND R1, R1, #1          ;10   (0x3014
	        BRp ODD1                ;11   (0x3016
	                                ;
CHECK2	    AND R2, R2, #1          ;12   (0x3018
            BRp ODD2                ;13   (0x301A
            BRnzp EVEN2             ;14   (0x301C
                                    ;
                                    ;
ODD1        ADD R5, R5, #1    	    ;15   (0x301E
	        BRnzp CHECK2            ;16   (0x3020
	                                ;
ODD2            ADD R5, R5, #1      ;17   (0x3022
                                    ;
EVEN2          ADD R3, R3, #-2      ;18   (0x3024
	        BRp FETCH               ;19   (0x3026
	                                ;
	        LEA R0, RESLOC          ;20   (0x3028
	        LDW R0, R0, #0          ;21   (0x302A      ;R0 = x4100
	        STW R5, R0, #0          ;22   (0x302C
                                    ;
	        HALT                    ;23   (0x302E


NUMSLOC    .FILL x4000              ;24   (0x3030
RESLOC	    .FILL x4100             ;25   (0x3032
TOTAL	    .FILL #4                ;26   (0x3034
	        .END                    ;
            .ORIG x3000             ;
	        AND R1, R1, #0          ;0
	        AND R2, R2, #0          ;1
	        AND R5, R5, #0          ;2   ;counter
                                    ;
	        LEA R0, TOTAL           ;3
	        LDW R3, R0, #0          ;4    ;R3=256
                                    ;
	        LEA R0, NUMSLOC         ;5
	        LDW R0, R0, #0          ;6     ;R0=x4000
	                                ;
FETCH	        LDB R1, R0, #0      ;7
	        LDB R2, R0, #1          ;8
	        ADD R0, R0, #2          ;9
	                                ;
	        AND R1, R1, #1          ;10
	        BRp ODD1                ;11
	                                ;
CHECK2	        AND R2, R2, #1      ;12
                BRp ODD2            ;13
                BRnzp EVEN2         ;14
                                    ;
                                    ;
ODD1            ADD R5, R5, #1	    ;15
	        BRnzp CHECK2            ;16
	                                ;
ODD2            ADD R5, R5, #1      ;17
                                    ;
EVEN2          ADD R3, R3, #-2      ;18
	        BRp FETCH               ;19
	                                ;
	        LEA R0, RESLOC          ;20
	        LDW R0, R0, #0          ;21        ;R0 = x4100
	        STW R5, R0, #0          ;22
                                    ;
	        HALT                    ;23
                                    ;
                                    ;
NUMSLOC    .FILL x4000              ;24
RESLOC	    .FILL x4100             ;25
TOTAL	    .FILL #256              ;26
	        .END                    ;
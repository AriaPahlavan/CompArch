            .ORIG x3000
	        AND R1, R1, #0
	        AND R2, R2, #0
	        AND R5, R5, #0      ;counter

	        LEA R0, TOTAL
	        LDW R3, R0, #0      ;R3=256

	        LEA R0, NUMSLOC
	        LDW R0, R0, #0       ;R0=x4000
	        
FETCH	        LDB R1, R0, #0
	        LDB R2, R0, #1
	        ADD R0, R0, #2
	        
	        AND R1, R1, #1
	        BRp ODD1
	        
CHECK2	        AND R2, R2, #1
                BRp ODD2
                BRnzp EVEN2
                

ODD1            ADD R5, R5, #1	        
	        BRnzp CHECK2
	        
ODD2            ADD R5, R5, #1

EVEN2          ADD R3, R3, #-2
	        BRp FETCH
	        
	        LEA R0, RESLOC
	        LDW R0, R0, #0     ;R0 = x4100
	        STW R5, R0, #0

	        HALT


NUMSLOC    .FILL x4000
RESLOC	    .FILL x4100
TOTAL	    .FILL #256
	        .END
; this is a comment

;LBL    LEA R0, A

    .ORIG x3000

NLBL    ADD, R7, R1, #0    ;x3000
	BRz DUD                  ;x3002

	NOT R1, R7               ;x3004

DUD	JSR OK                  ;x3006

	LEA R2, VALUE            ;x3008
;	LDB R1, R2, #25

OK  TRAP x25                     ;x300A

VALUE   .FILL x0004          ;x300C
NEG .FILL x4000                ;x300E error
POS .FILL x350               ;x3010
	.END
;
XLBL	LEA R0, A
;

	BRn NLBL                ;x3004
	BRz NLBL                ;x3006
	BRp NLBL                ;x3008
	BRnz NLBL               ;x300A
	BRzp NLBL               ;x300C
	BRnp NLBL               ;x300E
	BRnzp NLBL              ;x3010
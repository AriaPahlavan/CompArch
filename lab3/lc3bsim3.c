/*
    Name 1: Aria Pahlavan
    UTEID 1: ap44342
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {
	IRD,
	COND1, COND0,
	J5, J4, J3, J2, J1, J0,
	LD_MAR,
	LD_MDR,
	LD_IR,
	LD_BEN,
	LD_REG,
	LD_CC,
	LD_PC,
	GATE_PC,
	GATE_MDR,
	GATE_ALU,
	GATE_MARMUX,
	GATE_SHF,
	PCMUX1, PCMUX0,
	DRMUX,
	SR1MUX,
	ADDR1MUX,
	ADDR2MUX1, ADDR2MUX0,
	MARMUX,
	ALUK1, ALUK0,
	MIO_EN,
	R_W,
	DATA_SIZE,
	LSHF1,
	CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
                                      (x[J3] << 3) + (x[J2] << 2) +
                                      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); }
int GetLSHF1(int *x)         { return(x[LSHF1]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
   There are two write enable signals, one for each byte. WE0 is used for
   the least significant byte of a word. WE1 is used for the most significant
   byte of a word. */

#define WORDS_IN_MEM    0x08000
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

	int PC,		/* program counter */
			MDR,	/* memory data register */
			MAR,	/* memory address register */
			IR,		/* instruction register */
			N,		/* n condition bit */
			Z,		/* z condition bit */
			P,		/* p condition bit */
			BEN;        /* ben register */

	int READY;	/* ready bit */
	/* The ready bit is also latched as you dont want the memory system to assert it
	   at a bad point in the cycle*/

	int REGS[LC_3b_REGS]; /* register file. */

	int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

	int STATE_NUMBER; /* Current State Number - Provided for debugging */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {
	printf("----------------LC-3bSIM Help-------------------------\n");
	printf("go               -  run program to completion       \n");
	printf("run n            -  execute program for n cycles    \n");
	printf("mdump low high   -  dump memory from low to high    \n");
	printf("rdump            -  dump the register & bus values  \n");
	printf("?                -  display this help menu          \n");
	printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

	eval_micro_sequencer();
	cycle_memory();
	eval_bus_drivers();
	drive_bus();
	latch_datapath_values();

	CURRENT_LATCHES = NEXT_LATCHES;

	CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
	int i;

	if (RUN_BIT == FALSE) {
		printf("Can't simulate, Simulator is halted\n\n");
		return;
	}

	printf("Simulating for %d cycles...\n\n", num_cycles);
	for (i = 0; i < num_cycles; i++) {
		if (CURRENT_LATCHES.PC == 0x0000) {
			RUN_BIT = FALSE;
			printf("Simulator halted\n\n");
			break;
		}
		cycle();
	}
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {
	if (RUN_BIT == FALSE) {
		printf("Can't simulate, Simulator is halted\n\n");
		return;
	}

	printf("Simulating...\n\n");
	while (CURRENT_LATCHES.PC != 0x0000)
		cycle();
	RUN_BIT = FALSE;
	printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
	int address; /* this is a byte address */

	printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
	printf("-------------------------------------\n");
	for (address = (start >> 1); address <= (stop >> 1); address++)
		printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
	printf("\n");

	/* dump the memory contents into the dumpsim file */
	fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
	fprintf(dumpsim_file, "-------------------------------------\n");
	for (address = (start >> 1); address <= (stop >> 1); address++)
		fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
	fprintf(dumpsim_file, "\n");
	fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
	int k;

	printf("\nCurrent register/bus values :\n");
	printf("-------------------------------------\n");
	printf("Cycle Count  : %d\n", CYCLE_COUNT);
	printf("PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
	printf("IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
	printf("STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
	printf("BUS          : 0x%.4x\n", BUS);
	printf("MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
	printf("MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
	printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
	printf("Registers:\n");
	for (k = 0; k < LC_3b_REGS; k++)
		printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
	printf("\n");

	/* dump the state information into the dumpsim file */
	fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
	fprintf(dumpsim_file, "-------------------------------------\n");
	fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
	fprintf(dumpsim_file, "PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
	fprintf(dumpsim_file, "IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
	fprintf(dumpsim_file, "STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
	fprintf(dumpsim_file, "BUS          : 0x%.4x\n", BUS);
	fprintf(dumpsim_file, "MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
	fprintf(dumpsim_file, "MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
	fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
	fprintf(dumpsim_file, "Registers:\n");
	for (k = 0; k < LC_3b_REGS; k++)
		fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
	fprintf(dumpsim_file, "\n");
	fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
	char buffer[20];
	int start, stop, cycles;

	printf("LC-3b-SIM> ");

	scanf("%s", buffer);
	printf("\n");

	switch(buffer[0]) {
		case 'G':
		case 'g':
			go();
			break;

		case 'M':
		case 'm':
			scanf("%i %i", &start, &stop);
			mdump(dumpsim_file, start, stop);
			break;

		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("Bye.\n");
			exit(0);

		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D')
				rdump(dumpsim_file);
			else {
				scanf("%d", &cycles);
				run(cycles);
			}
			break;

		default:
			printf("Invalid Command\n");
			break;
	}
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {
	FILE *ucode;
	int i, j, index;
	char line[200];

	printf("Loading Control Store from file: %s\n", ucode_filename);

	/* Open the micro-code file. */
	if ((ucode = fopen(ucode_filename, "r")) == NULL) {
		printf("Error: Can't open micro-code file %s\n", ucode_filename);
		exit(-1);
	}

	/* Read a line for each row in the control store. */
	for(i = 0; i < CONTROL_STORE_ROWS; i++) {
		if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
			printf("Error: Too few lines (%d) in micro-code file: %s\n",
			       i, ucode_filename);
			exit(-1);
		}

		/* Put in bits one at a time. */
		index = 0;

		for (j = 0; j < CONTROL_STORE_BITS; j++) {
			/* Needs to find enough bits in line. */
			if (line[index] == '\0') {
				printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
				       ucode_filename, i);
				exit(-1);
			}
			if (line[index] != '0' && line[index] != '1') {
				printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
				       ucode_filename, i, j);
				exit(-1);
			}

			/* Set the bit in the Control Store. */
			CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
			index++;
		}

		/* Warn about extra bits in line. */
		if (line[index] != '\0')
			printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
			       ucode_filename, i);
	}
	printf("\n");
}

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
void init_memory() {
	int i;

	for (i=0; i < WORDS_IN_MEM; i++) {
		MEMORY[i][0] = 0;
		MEMORY[i][1] = 0;
	}
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
	FILE * prog;
	int ii, word, program_base;

	/* Open program file. */
	prog = fopen(program_filename, "r");
	if (prog == NULL) {
		printf("Error: Can't open program file %s\n", program_filename);
		exit(-1);
	}

	/* Read in the program. */
	if (fscanf(prog, "%x\n", &word) != EOF)
		program_base = word >> 1;
	else {
		printf("Error: Program file is empty\n");
		exit(-1);
	}

	ii = 0;
	while (fscanf(prog, "%x\n", &word) != EOF) {
		/* Make sure it fits. */
		if (program_base + ii >= WORDS_IN_MEM) {
			printf("Error: Program file %s is too long to fit in memory. %x\n",
			       program_filename, ii);
			exit(-1);
		}

		/* Write the word to memory array. */
		MEMORY[program_base + ii][0] = word & 0x00FF;
		MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
		ii++;
	}

	if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

	printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) {
	int i;
	init_control_store(ucode_filename);

	init_memory();
	for ( i = 0; i < num_prog_files; i++ ) {
		load_program(program_filename);
		while(*program_filename++ != '\0');
	}
	CURRENT_LATCHES.Z = 1;
	CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
	memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

	NEXT_LATCHES = CURRENT_LATCHES;

	RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
	FILE * dumpsim_file;

	/* Error Checking */
	if (argc < 3) {
		printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
		       argv[0]);
		exit(1);
	}

	printf("LC-3b Simulator\n\n");

	initialize(argv[1], argv[2], argc - 2);

	if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
		printf("Error: Can't open dumpsim file\n");
		exit(-1);
	}

	while (1)
		get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/

#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>


/**----------------------------------- Definitions ------------------------------------*/
#define MANUAL_DEBUG        FALSE
#define EMPTY_VAL           0
#define LOG_LEVEL           NONE
#define MAX_LOG_DEF_ARGS    4
#define nibble              4
#define debug               EOL, __FUNCTION__, __LINE__, DEBUG
#define info                EOL, __FUNCTION__, __LINE__, INFO
#define warn                EOL, __FUNCTION__, __LINE__, WARN
#define error               EOL, __FUNCTION__, __LINE__, ERROR
#define memData(mem_index)  Low16bits(((MEMORY[mem_index][1] << 2*nibble) & 0xFF00) + (MEMORY[mem_index][0] & 0x00FF))
#define MEM(addr)           memData(addr/2)
#define bit(IR, b)          Low16bits((IR >> b) & 0x0001)
#define signExt(val, sbit)  Low16bits(bit(val, sbit) == 1 ? ((0xFFFF << (sbit + 1)) | val) \
                                                             : ((0xFFFF >> (16 - (sbit + 1))) & val))
#define SEXT(val, num_bits) signExt(val, num_bits-1)
#define lowByte(val)        Low16bits(val & 0x00FF)
#define highByte(val)       lowByte(val >> 2*nibble)
#define SEXT_VAL(val, n)    Low16bits((0xFFFF << (16 - n)) | val)
#define ZEXT_VAL(val, n)    Low16bits((0xFFFF >> n) & val)
#define LSHF(val)           Low16bits(val << 1)
#define LSHFN(val, n)       Low16bits(val << n)
#define RSHFN(val, n, b)    b == 0 ? ZEXT_VAL(val >> n, n) : SEXT_VAL(val >> n, n)
#define ZEXT8(val)           (val & 0x00FF)
#define DR_NUM(IR)          Low16bits((((IR & 0x0E00) >> 9) & 0x0007))
#define SR1_NUM(IR)         Low16bits((((IR & 0x01C0) >> 6) & 0x0007))
#define SR2_NUM(IR)         Low16bits((IR & 0x0007))
#define DR(IR)              Low16bits(CURRENT_LATCHES.REGS[DR_NUM(IR)])
#define SR1(IR)             Low16bits(CURRENT_LATCHES.REGS[SR1_NUM(IR)])
#define SR2(IR)             Low16bits(CURRENT_LATCHES.REGS[SR2_NUM(IR)])
#define amount4(IR)         (IR & 0x000F)
#define imm5(IR)            (IR & 0x001F)
#define boffset6(IR)        (IR & 0x003F)
#define offset6(IR)         SEXT( (IR & 0x003F), 6 )
#define PCoffset9(IR)       SEXT( (IR & 0x01FF), 9 )
#define PCoffset11(IR)      SEXT( (IR & 0x07FF), 11 )
/*---------------------------------------------------------------------------------*/
#define low3bits(BEN, R, IR11) (((BEN << 2) + (R << 1) + IR11) & 0x0007)
#define align(addr) (addr & 0xFFFE)
#define STB_MDR(val) Low16bits( (lowByte(val) << 2*nibble) + lowByte(val) )
#define IR15_12(IR) Low16bits( ((IR & 0xF000) >> 3*nibble) & 0x000F )
/* R.W */
    #define RD_         0
    #define WR_         1
/* DATA.SIZE */
    #define BYTE_       0
    #define WORD_       1
/* PCMUX */
    #define PCplus2_    0
    #define BUS_        1
    #define ADDER_PC_   2
/* DRMUX */
    #define B11_9_      0
    #define R7_         1
/* SR1MUX */
    #define B8_6_       1
/* SR2MUX */
    #define SR2_        0
    #define imm5_       1       /* SEXT[ IR[4:0]  ] */
/* ADDR1MUX */
    #define PC_         0
    #define baseR_      1
/* ADDR2MUX */
    #define ZERO_       0
    #define offset6_    1       /* SEXT[ IR[5:0]  ] */
    #define PCoffset9_  2       /* SEXT[ IR[8:0]  ] */
    #define PCoffset11_ 3       /* SEXT[ IR[10:0] ] */
/* MARMUX */
    #define B7_0_       0       /* LSHF(ZEXT[IR[7:0]],1) */
    #define ADDER_MAR_  1
/* ALUK */
    #define ADD_        0
    #define AND_        1
    #define XOR_        2
    #define PASSA_      3
/* SIGNALS */
    #define NO_         0
    #define YES_        1
#define ANRM  "\x1B[0m"
#define ARED  "\x1B[31m"
#define AGRN  "\x1B[32m"
#define AYEL  "\x1B[33m"
#define ABLU  "\x1B[34m"
#define AMAG  "\x1B[35m"
#define ACYN  "\x1B[36m"
#define AWHT  "\x1B[37m"


/**-------------------------------- Structures & Enums --------------------------------*/
typedef struct TRISTATE_GATES_STRUCTURE{
    int gate_MARMUX_input,
         gate_PC_input,
         gate_ALU_input,
         gate_SHF_input,
         gate_MDR_input;

    int16_t  ADDR;
} GATES_STRUCT;
enum DATA_TYPE {
    CUR_LATCH,
    MICROINST,
    ADDR,
    STAT,
    PTR,
    CHAR,
    INT_8,
    INT_16,
    INT_32,
    INT_64,
    UINT_8,
    UINT_16,
    UINT_32,
    UINT_64,
    STR,
    INT,
    LONG,
    LONG_INT,
    ULONG,
    ULONG_INT,
    S,
    I,
    Id,
    Ix,
    L,
    C,
    P,
    I8,
    I16,
    I32,
    I64,
    UI,
    UI8,
    UI16,
    UI32,
    UI64,
    UL,
    ULI,
    LI,
    N,
    ENDL,
    T,
    TAB,
    EOL
};
enum LOG_LEVELS {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    NONE
};
enum REGISTERS {
    r0, r1, r2, r3, r4, r5, r6, r7
};


/**--------------------------------- Global Variables ---------------------------------*/
GATES_STRUCT GATE_INPUTS;
const static char *enumStrings[] = {"DEBUG", "INFO", "WARN", "ERROR"};


/**-------------------------------- Function Declarations -----------------------------*/
void logging(int num, ...);

void loggingNoHeader(int num, ...);

void loggingMsg(const char *msg, enum DATA_TYPE t, const char *func, int line, enum LOG_LEVELS lvl);

void loggingMsgNoHeader(const char *msg, enum DATA_TYPE t, const char *func, int line, enum LOG_LEVELS lvl);

void print(int num, ...);

void println(int num, ...);

enum OPCODES fetch(int opcode, int pc);

int getReadyStatus(int cond, int R);

int getAddrModeStatus(int cond, int IR11);

int getBranchStatus(int cond, int BEN);

void cpyMicroInst(int dst[], int src[]);

void storeWordVal(uint16_t MAR, int16_t MDR);

void storeByteValue(uint16_t MAR, int16_t MDR);

void memOperation();

int16_t getMARMUXoutput(System_Latches curLatch);

int16_t getPCoutput(System_Latches curLatch);

int16_t getALUoutput(System_Latches curLatch);

int16_t getSHFoutput(System_Latches curLatch);

int16_t getMDRMUXoutput(System_Latches curLatch);

void updatePC(System_Latches curLatch);

void setCC();

void updateREG(System_Latches curLatch);

void updateMDR(System_Latches curLatch);

void updateBEN();

void updateIR();

void updateMAR();


/**-------------------------------- Function Definitions ------------------------------*/
/**
 * Evaluate the address of the next state according to the
 * micro sequencer logic. Latch the next microinstruction.
 */
void eval_micro_sequencer() {
    logging(CUR_LATCH, &CURRENT_LATCHES, debug);

    int     J =                 GetJ(CURRENT_LATCHES.MICROINSTRUCTION),
            COND =              GetCOND(CURRENT_LATCHES.MICROINSTRUCTION),
		    IRD =               GetIRD(CURRENT_LATCHES.MICROINSTRUCTION),
		    IR =                CURRENT_LATCHES.IR,
            readyStatus =       getReadyStatus(COND, CURRENT_LATCHES.READY),
            addrModeStatus =    getAddrModeStatus(COND, bit(CURRENT_LATCHES.IR, 11)),
            branchStatus =      getBranchStatus(COND, CURRENT_LATCHES.BEN),
            nextStateAddr =     (IRD == YES_ ? IR15_12(IR)
                                             : Low16bits(J | low3bits(branchStatus, readyStatus, addrModeStatus)));

    loggingNoHeader(S, "Next state address: ", I, nextStateAddr, debug);

    NEXT_LATCHES.STATE_NUMBER = nextStateAddr;
    cpyMicroInst(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[nextStateAddr]);

    loggingNoHeader(S, "Next state microinstruction: \n\t", MICROINST, NEXT_LATCHES.MICROINSTRUCTION, debug);

}

/**
  * This function emulates memory and the WE logic.
  * Keep track of which cycle of MEMEN we are dealing with.
  * If fourth, we need to latch Ready bit at the end of
  * cycle to prepare microsequencer for the fifth cycle.
  */
void cycle_memory() {
    int MIO, R;
    MIO = GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION);
    R = CURRENT_LATCHES.READY;

    if (R == (MEM_CYCLES - 1)) {
        memOperation();
        NEXT_LATCHES.READY = 0;
    }
    else if (MIO == YES_) NEXT_LATCHES.READY = R + 1;
}

/**
 * emulates memory operations (i.e. load or store)
 *
 * @param nextLatch
 * @param memSignlas
 */
void memOperation() {
    int*    microinst = CURRENT_LATCHES.MICROINSTRUCTION;
    int     MIO_EN = GetMIO_EN(microinst),
            R_W = GetR_W(microinst),
            MDR = CURRENT_LATCHES.MDR,
            MAR = CURRENT_LATCHES.MAR,
            DATA_SIZE = GetDATA_SIZE(microinst);


    if (MIO_EN == NO_) {
        loggingMsg("Memory operation terminated since MIO.EN is not enables.", error);
        return;
    }

    switch (R_W) {
        case RD_:
            if (DATA_SIZE == BYTE_) {
                CURRENT_LATCHES.MDR = MEM(align(MAR));
            }
            else if (DATA_SIZE == WORD_) {
                CURRENT_LATCHES.MDR = MEM(MAR);
            }
            break;
        case WR_:
            if (DATA_SIZE == BYTE_) {
                storeByteValue((uint16_t) MAR, MDR);
            }
            else if (DATA_SIZE == WORD_) {
                storeWordVal((uint16_t) MAR, MDR);
            }
            break;
        default:
            logging(S, "Invalid R.W detected: ", I, R_W, error);
            break;
    }
}

/**
  * Datapath routine emulating operations before driving the bus.
  * Evaluate the input of tristate drivers
  *      Gate_MARMUX,
  *		 Gate_PC,
  *		 Gate_ALU,
  *		 Gate_SHF,
  *		 Gate_MDR.
  */
void eval_bus_drivers() {
    GATE_INPUTS.gate_MARMUX_input =     getMARMUXoutput  (CURRENT_LATCHES);
    GATE_INPUTS.gate_PC_input     =     getPCoutput      (CURRENT_LATCHES);
    GATE_INPUTS.gate_ALU_input    =     getALUoutput     (CURRENT_LATCHES);
    GATE_INPUTS.gate_SHF_input    =     getSHFoutput     (CURRENT_LATCHES);
    GATE_INPUTS.gate_MDR_input    =     getMDRMUXoutput  (CURRENT_LATCHES);
}

/**
  * Datapath routine for driving the bus from one of the 5 possible
  * tristate drivers.
  */
void drive_bus() {
    int16_t result;
    int* microinst = CURRENT_LATCHES.MICROINSTRUCTION;

    if       (GetGATE_ALU(microinst))       result = GATE_INPUTS.gate_ALU_input;
    else if (GetGATE_MARMUX(microinst))     result = GATE_INPUTS.gate_MARMUX_input;
    else if (GetGATE_MDR(microinst))        result = GATE_INPUTS.gate_MDR_input;
    else if (GetGATE_PC(microinst))         result = GATE_INPUTS.gate_PC_input;
    else if (GetGATE_SHF(microinst))        result = GATE_INPUTS.gate_SHF_input;
    else                                    result = 0;

    BUS = Low16bits(result);
}

/**
  * Datapath routine for computing all functions that need to latch
  * values in the data path at the end of this cycle.  Some values
  * require sourcing the bus; therefore, this routine has to come
  * after drive_bus.
  */
void latch_datapath_values() {
	int *microinst = CURRENT_LATCHES.MICROINSTRUCTION;

	if (GetLD_PC(microinst)) updatePC(CURRENT_LATCHES);
	if (GetLD_IR(microinst)) updateIR();
	if (GetLD_MDR(microinst)) updateMDR(CURRENT_LATCHES);
	if (GetLD_MAR(microinst)) updateMAR();
	if (GetLD_BEN(microinst)) updateBEN();
	if (GetLD_REG(microinst)) updateREG(CURRENT_LATCHES);
	if (GetLD_CC(microinst)) setCC();
}

void updateMAR() {
	logging(S, "Updating MAR to: ", I, Low16bits(BUS), debug);

	NEXT_LATCHES.MAR = Low16bits(BUS);
}

void updateIR() {
	logging(S, "Updating IR to: ", I, Low16bits(BUS), debug);

	NEXT_LATCHES.IR = Low16bits(BUS);
}

void updateBEN() {
	int     P = CURRENT_LATCHES.P,
			Z = CURRENT_LATCHES.Z,
			N = CURRENT_LATCHES.N,
			IR = CURRENT_LATCHES.IR;

	NEXT_LATCHES.BEN = (( ( (bit(IR, 11) == 1) && (N == 1) )
	                   || ( (bit(IR, 10) == 1) && (Z == 1) )
	                   || ( (bit(IR, 9)  == 1) && (P == 1) )) ? 1 : 0);

	logging(S, "Updating BEN to: ", Id, NEXT_LATCHES.BEN,
	        S, "\nCurrent instruction: ", Ix, NEXT_LATCHES.IR, info);
}

void updateMDR(System_Latches curLatch) {
	int16_t DATA_SIZE, result;
    int* microinst = curLatch.MICROINSTRUCTION;
    DATA_SIZE = GetDATA_SIZE(microinst);

    if (GetMIO_EN(microinst) == YES_)   result = curLatch.MDR;
    else if (DATA_SIZE == BYTE_)        result = STB_MDR(BUS);
    else if (DATA_SIZE == WORD_)        result = Low16bits(BUS);
    else                                result = CURRENT_LATCHES.MDR;

	NEXT_LATCHES.MDR = Low16bits(result);

	logging(S, "Updating MDR to: ", I, result, debug);
}

void updateREG(System_Latches curLatch) {
	logging(S, "Updating REG to: ", I, Low16bits(BUS), debug);

	int DRMUX = GetDRMUX(curLatch.MICROINSTRUCTION);
    int16_t IR = CURRENT_LATCHES.IR;

    if (DRMUX == B11_9_)    NEXT_LATCHES.REGS[DR_NUM(IR)] = Low16bits(BUS);
    else                    NEXT_LATCHES.REGS[r7] = Low16bits(BUS);
}

void setCC() {
	logging(S, "Setting CC to N=", Id, Low16bits(BUS) < 0,
	        S, " Z=", Id, Low16bits(BUS) == 0,
	        S, " P=", Id, Low16bits(BUS) > 0, debug);

	NEXT_LATCHES.N = (  (int16_t) BUS <  0 ? 1 : 0 );
	NEXT_LATCHES.Z = (  (int16_t) BUS == 0 ? 1 : 0 );
	NEXT_LATCHES.P = (  (int16_t) BUS >  0 ? 1 : 0 );
}

void updatePC(System_Latches curLatch){
	int16_t result;
    int* microinst = curLatch.MICROINSTRUCTION;

    switch (GetPCMUX(microinst)){
        case PCplus2_:      result = curLatch.PC + 2;       break;
        case BUS_:          result = Low16bits(BUS);        break;
        case ADDER_PC_:     result = GATE_INPUTS.ADDR;      break;
	    default:            result = CURRENT_LATCHES.PC;    break;
    }

	NEXT_LATCHES.PC = result;

	logging(S, "Updating PC to: ", I, result, debug);
}

void cpyMicroInst(int dst[], int src[]) {
    int i;

    for (i = 0; i < CONTROL_STORE_BITS; ++i) {
        dst[i] = src[i];
    }

}

int getBranchStatus(int cond, int BEN) {
	if (CURRENT_LATCHES.STATE_NUMBER == 0)
		logging(S, "Branch status checked to be: ",
	          I, cond == 2 && BEN, info
		);
    return cond == 2 && BEN;
}

int getAddrModeStatus(int cond, int IR11) {
    return cond == 3 && IR11;
}

int getReadyStatus(int cond, int R) {
    return cond == 1 && R == (MEM_CYCLES - 1);
}

void storeWordVal(uint16_t MAR, int16_t MDR) {
    loggingNoHeader(S, "<SW> Storing ", I, MDR, S, " into address ", I, MAR, info);

    int *mem = MEMORY[MAR / 2];

    mem[0] = Low16bits(MDR & 0x000000FF);
    mem[1] = Low16bits(MDR & 0x0000FF00) >> 2*nibble;
}

void storeByteValue(uint16_t MAR, int16_t MDR) {
    loggingNoHeader(S, "<SB> Storing ", I, MDR, S, " into address ", I, MAR, info);

    int *mem = MEMORY[MAR/2];

    if      (MAR%2 == 0) mem[MAR%2] = Low16bits(lowByte(MDR));
    else if (MAR%2 == 1) mem[MAR%2] = Low16bits(highByte(MDR));
}

int16_t getMDRMUXoutput(System_Latches curLatch) {
    int16_t result;
    int* microinst = curLatch.MICROINSTRUCTION;

    if (bit(curLatch.MAR, 0) == 0)	    result = lowByte(curLatch.MDR);        /* MDR[7:0]  */
    else                                result = highByte(curLatch.MDR);       /* MDR[15:8] */

    switch (GetDATA_SIZE(microinst)) {
        case BYTE_:
            if (result & 0x0080)    result = result + 0xFF00;              /* sig extend */
            break;
        case WORD_:
            result = curLatch.MDR;
            break;
    }

	logging(S, "Updating MDRMUX output to: ", I, Low16bits(result), debug);

	return Low16bits(result);
}

int16_t getSHFoutput(System_Latches curLatch) {
    int16_t IR, off6, result, SR1;
    IR = curLatch.IR;
    off6 = boffset6(IR);
    SR1 = (GetSR1MUX(curLatch.MICROINSTRUCTION) == B11_9_ ? DR(IR) : SR1(IR));

    if (bit(off6, 4) == 0)       result = LSHFN(SR1, amount4(IR));
    else if (bit(off6, 5) == 0)  result = RSHFN(SR1, amount4(IR), 0);
    else                         result = RSHFN(SR1, amount4(IR), bit(SR1, 15));

	logging(S, "Updating SHF output to: ", I, Low16bits(result), debug);

	return Low16bits(result);
}

int16_t getALUoutput(System_Latches curLatch) {
    int16_t IR, ALUK, result, SR1;
    int* microinst = curLatch.MICROINSTRUCTION;
    IR = curLatch.IR;
    ALUK = GetALUK(microinst);
    SR1 = (GetSR1MUX(microinst) == B11_9_ ? DR(IR) : SR1(IR));

    switch (ALUK) {
        case ADD_:
            if (bit(IR, 5) == 0)    result = SR1 + SR2(IR);
            else                    result = SR1 + SEXT(imm5(IR), 5);
            break;
        case AND_:
            if (bit(IR, 5) == 0)    result = SR1 & SR2(IR);
            else                    result = SR1 & SEXT(imm5(IR), 5);
            break;
        case XOR_:
            if (bit(IR, 5) == 0)    result = SR1 ^ SR2(IR);
            else                    result = SR1 ^ SEXT(imm5(IR), 5);
            break;
        case PASSA_:                result = SR1;
            break;
	    default:
		                            result = 0;
		    logging(S, "Ivalid ALUK MUX signal", error);
		    break;
    }

	logging(S, "Updating ALU output to: ", I, Low16bits(result), debug);

	return Low16bits(result);
}

int16_t getPCoutput(System_Latches curLatch) {
	logging(S, "Updating PC output to: ", I, Low16bits(curLatch.PC), debug);

	return curLatch.PC;
}

int16_t getMARMUXoutput(System_Latches curLatch) {
    int16_t IR, result, ADDER1MUX, ADDER2MUX, ADDER;
    int* microinst = curLatch.MICROINSTRUCTION;
    IR = curLatch.IR;

    ADDER1MUX = (GetADDR1MUX(microinst) == PC_ ? curLatch.PC : SR1(IR));

    switch (GetADDR2MUX(microinst)) {
        case ZERO_:         ADDER2MUX = 0;              break;
        case offset6_:      ADDER2MUX = offset6(IR);    break;
        case PCoffset9_:    ADDER2MUX = PCoffset9(IR);  break;
        case PCoffset11_:   ADDER2MUX = PCoffset11(IR); break;
        default:            ADDER2MUX = 0;              break;
    }

    ADDER2MUX = (GetLSHF1(microinst) == YES_ ? LSHF(ADDER2MUX) : ADDER2MUX);
    ADDER = GATE_INPUTS.ADDR =  ADDER1MUX + ADDER2MUX;
    result = (GetMARMUX(microinst) == B7_0_ ? LSHF(ZEXT8(IR)) : ADDER);

	logging(S, "Updating MARMUX output to: ", I, Low16bits(result), debug);

	return Low16bits(result);
}


/**-------------------------------- Logging Library ---------------------------------*/
void printList(int num, va_list valist) {
    int i;
    num *= 2;

    enum DATA_TYPE T;
    void *V;

    /* access all the arguments assigned to valist */
    for (i = 0; i < num;) {
        T = va_arg(valist, enum DATA_TYPE);

        if (T == N || T == ENDL) {
            V = EMPTY_VAL;
            T == ENDL ? i += 2 : 0;
        } else {
            V = va_arg(valist, void *);
            i += 2;
        }

        output(V, T);
    }
}

void print(int num, ...) {
    va_list valist;

    /* initialize valist for num number of arguments */
    va_start(valist, num);

    printList(num, valist);

    /* clean memory reserved for valist */
    va_end(valist);
}

void println(int num, ...) {
    va_list valist;

    /* initialize valist for num number of arguments */
    va_start(valist, num);

    printList(num, valist);

    /* clean memory reserved for valist */
    va_end(valist);

    output(EMPTY_VAL, ENDL);
}

void outputDouble(double value) {
    printf("%f", value);
}

void output(void *V, enum DATA_TYPE Type) {
    switch (Type) {
        case CUR_LATCH:
            if (TRUE){
                int k;
                printf("N=%d, Z=%d, P=%d\nSTATE NUM=%d, PC=%d, BEN=%d, IR=%d\nMAR=%d, MDR=%d, READY=%d\n",
                       CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P,
                       CURRENT_LATCHES.STATE_NUMBER, CURRENT_LATCHES.PC, CURRENT_LATCHES.BEN, CURRENT_LATCHES.IR,
                       CURRENT_LATCHES.MAR, CURRENT_LATCHES.MDR, CURRENT_LATCHES.READY
                );
                output(CURRENT_LATCHES.MICROINSTRUCTION, MICROINST);
                for (k = 0; k < LC_3b_REGS; k++) {
                    printf("R%d: 0x%.4X  ", k, CURRENT_LATCHES.REGS[k]);
                    if (k == 3) printf("\n");
                }
            }
            break;
        case MICROINST:
            if (TRUE) {
                int i;

                for (i = 0; i < CONTROL_STORE_BITS; ++i) {
                    printf("%d", ((int *) V)[i]);
                }
                printf("\n");
            }
            break;
        case ADDR:
            printf("0x%X", MEM((int) V));
            break;
        case STAT:
            if (TRUE) {
                int k;
                printf("PC: 0x%.4X\n", CURRENT_LATCHES.PC);
                printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
                for (k = 0; k < LC_3b_REGS; k++) {
                    printf("R%d: 0x%.4X  ", k, CURRENT_LATCHES.REGS[k]);
                    if (k == 3) printf("\n");
                }
            }
            break;
        case P:
        case PTR:
            printf("%p", (int *) V);
            break;
        case C:
        case CHAR:
            printf("%c", (char) V);
            break;
        case S:
        case STR:
            printf("%s", (char *) V);
            break;
        case I8:
        case INT_8:
            if(MANUAL_DEBUG) printf("%d %s(0x%X)%s", (int8_t) V, AWHT, (int8_t) V, ANRM);
            else printf("%d (0x%X)", (int8_t) V, (int8_t) V);
            break;
        case I16:
        case INT_16:
            if(MANUAL_DEBUG) printf("%d %s(0x%X)%s", (int16_t) V, AWHT, (int16_t) V, ANRM);
            else printf("%d (0x%X)", (int16_t) V,  (int16_t) V);
            break;
        case I32:
        case INT_32:
            if(MANUAL_DEBUG) printf("%d %s(0x%X)%s", (int32_t) V, AWHT, (int32_t) V, ANRM);
            else printf("%d (0x%X)", (int32_t) V,  (int32_t) V);
            break;
        case I64:
        case INT_64:
            if(MANUAL_DEBUG) printf("%ld %s(0x%lX)%s", (int64_t) V, AWHT, (int64_t) V, ANRM);
            else printf("%ld (0x%lX)", (int64_t) V, (int64_t) V);
            break;
        case UI8:
        case UINT_8:
            if(MANUAL_DEBUG) printf("%u %s(0x%X)%s", (uint8_t) V, AWHT, (uint8_t) V, ANRM);
            else printf("%u (0x%X)", (uint8_t) V, (uint8_t) V);
            break;
        case UI16:
        case UINT_16:
            if(MANUAL_DEBUG) printf("%u %s(0x%X)%s", (uint16_t) V, AWHT, (uint16_t) V, ANRM);
            else printf("%u (0x%X)", (uint16_t) V,  (uint16_t) V);
            break;
        case UI32:
        case UINT_32:
            if(MANUAL_DEBUG) printf("%u %s(0x%X)%s", (uint32_t) V, AWHT, (uint32_t) V, ANRM);
            else printf("%u (0x%X)", (uint32_t) V, (uint32_t) V);
            break;
        case UI64:
        case UINT_64:
            if(MANUAL_DEBUG) printf("%llu %s(0x%llX)%s", (uint64_t) V, AWHT, (uint64_t) V, ANRM);
            else printf("%llu (0x%llX)", (uint64_t) V, (uint64_t) V);
            break;
        case I:
        case INT:
            if(MANUAL_DEBUG) printf("%d %s(0x%X)%s", (int) V, AWHT, (int) V, ANRM);
            else printf("%d (0x%X)", (int) V, (int) V);
            break;
        case Id:
            printf("%d", (int) V);
            break;
        case Ix:
            printf("0x%X", (int) V);
            break;
        case L:
        case LONG:
            if(MANUAL_DEBUG) printf("%ld %s(0x%lX)%s", (long) V, AWHT, (long) V, ANRM);
            else printf("%ld (0x%lX)", (long) V, (long) V);
            break;
        case LI:
        case LONG_INT:
            if(MANUAL_DEBUG) printf("%ld %s(0x%lX)%s", (long int) V, AWHT, (long int) V, ANRM);
            else printf("%ld (0x%lX)", (long int) V, (long int) V);
            break;
        case UL:
        case ULONG:
            if(MANUAL_DEBUG) printf("%lu %s(0x%lX)%s", (unsigned long) V, AWHT, (unsigned long) V, ANRM);
            else printf("%lu (0x%lX)", (unsigned long) V, (unsigned long) V);
            break;
        case ULI:
        case ULONG_INT:
            if(MANUAL_DEBUG) printf("%lu %s(0x%lX)%s", (unsigned long int) V, AWHT, (unsigned long int) V, ANRM);
            else printf("%lu (0x%lX)", (unsigned long int) V, (unsigned long int) V);
            break;
        case N:
        case ENDL:
            printf("\n");
            break;
        case T:
        case TAB:
            printf("\t");
            break;
        default:
            printf("INVALID ARGUMENTS\n");
    }
}

void setColorPrint(enum LOG_LEVELS lvl) {
    switch (lvl) {
        case DEBUG:
            printf(ACYN "");
            break;
        case INFO:
            printf(ABLU "");
            break;
        case WARN:
            printf(AYEL "");
            break;
        case ERROR:
            printf(ARED "");
            break;
    }
}

void colorPrint(enum LOG_LEVELS lvl, const char *txt) {
    switch (lvl) {
        case DEBUG:
            printf(ACYN "%s" ANRM, txt);
            break;
        case INFO:
            printf(ABLU "%s" ANRM, txt);
            break;
        case WARN:
            printf(AYEL "%s" ANRM, txt);
            break;
        case ERROR:
            printf(ARED "%s" ANRM, txt);
            break;
    }

}

int getLogNum() {
    static int logNum = 0;
    return logNum++;
}

void logging(int num, ...) {
    va_list valist;
    char *func;
    int line, cur_indx = 0, i;
    enum LOG_LEVELS lvl;
    void *values[100];
    enum DATA_TYPE tags[100];
    enum DATA_TYPE tag;

    /* initialize valist for num number of arguments */
    va_start(valist, MAX_LOG_DEF_ARGS);
    if (num == EOL) return;

    tags[cur_indx] = num;
    values[cur_indx] = va_arg(valist, void *);
    cur_indx++;

    while ((tag = va_arg(valist, enum DATA_TYPE)) != EOL) {
        tags[cur_indx] = tag;
        if (tag == ENDL || tag == N || tag == TAB || tag == T) {
            cur_indx++;
            continue;
        }
        values[cur_indx] = va_arg(valist, void *);
        cur_indx++;
    }
    func = va_arg(valist, char *);
    line = va_arg(valist, int);
    lvl = va_arg(valist, enum LOG_LEVELS);

    printLog(func, line, lvl, cur_indx, values, tags, true);

    /* clean memory reserved for valist */
    va_end(valist);

}

void loggingMsg(const char *msg, enum DATA_TYPE t, const char *func, int line, enum LOG_LEVELS lvl) {
    if (lvl >= LOG_LEVEL) {
        int curLogNum = getLogNum();
        setColorPrint(lvl);
        printf("[%s]: func:'%s' → line:%d (%d)%s\n%s", enumStrings[lvl], func, line, curLogNum, ANRM, msg);
        setColorPrint(lvl);
        printf(" (%d)%s\n", curLogNum, ANRM);
    }
}

void printLog(const char *func, int line, enum LOG_LEVELS level, int length, void *const *values,
              const enum DATA_TYPE *tags, bool header) {
    int i;
    enum DATA_TYPE tag;
    void *val;
    if (level >= LOG_LEVEL) {
        int curLogNum = getLogNum();

        if (header) {
            if(MANUAL_DEBUG) {
                setColorPrint(level);
                printf("[%s]: func:'%s' → line:%d (%d)\n" ANRM, enumStrings[level], func, line, curLogNum);

            } else{
                if (line < 10) printf("\n------------------c(%d) -> l(%d)----------------\n", (CYCLE_COUNT+1), line);
                else if (line < 100) printf("\n------------------c(%d) -> l(%d)---------------\n", (CYCLE_COUNT+1), line);
                else if (line < 1000) printf("\n-----------------c(%d) -> l(%d)---------------\n", (CYCLE_COUNT+1), line);
                else printf("\n-----------------c(%d) -> l(%d)--------------\n", (CYCLE_COUNT+1), line);
            }

        }

        /* access all the arguments assigned to valist */
        for (i = 0; i < length; i++) {
            tag = tags[i];

            if (tag == N || tag == ENDL || tag == TAB || tag == T)
                val = EMPTY_VAL;
            else
                val = values[i];

            output(val, tag);
        }

        if(MANUAL_DEBUG) {
            setColorPrint(level);
            printf(" (%d)\n" ANRM, curLogNum);

        } else{
            printf("\n");
        }
    }
}

void loggingMsgNoHeader(const char *msg, enum DATA_TYPE t, const char *func, int line, enum LOG_LEVELS lvl) {
    if (lvl >= LOG_LEVEL) {
        int curLogNum = getLogNum();
        printf("%s", msg);
        setColorPrint(lvl);
        printf(" (%d)%s\n", curLogNum, ANRM);
    }
}

void loggingNoHeader(int num, ...) {
    va_list valist;
    char *func;
    int line, cur_indx = 0, i;
    enum LOG_LEVELS lvl;
    void *values[100];
    enum DATA_TYPE tags[100];
    enum DATA_TYPE tag;

    /* initialize valist for num number of arguments */
    va_start(valist, MAX_LOG_DEF_ARGS);
    if (num == EOL) return;

    tags[cur_indx] = num;
    values[cur_indx] = va_arg(valist, void *);
    cur_indx++;

    while ((tag = va_arg(valist, enum DATA_TYPE)) != EOL) {
        tags[cur_indx] = tag;
        if (tag == ENDL || tag == N || tag == TAB || tag == T) {
            cur_indx++;
            continue;
        }
        values[cur_indx] = va_arg(valist, void *);
        cur_indx++;
    }
    func = va_arg(valist, char *);
    line = va_arg(valist, int);
    lvl = va_arg(valist, enum LOG_LEVELS);


    printLog(func, line, lvl, cur_indx, values, tags, false);

    /* clean memory reserved for valist */
    va_end(valist);

}

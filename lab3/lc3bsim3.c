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
#define MANUAL_DEBUG FALSE
#define EMPTY_VAL 0
#define LOG_LEVEL NONE
#define MAX_LOG_DEF_ARGS 4
#define MAX_NUM_OPCODES 29
#define nibble 4
#define debug EOL, __FUNCTION__, __LINE__, DEBUG
#define info EOL, __FUNCTION__, __LINE__, INFO
#define warn EOL, __FUNCTION__, __LINE__, WARN
#define error EOL, __FUNCTION__, __LINE__, ERROR
#define incrmntdPC(pc) (pc+2)
#define getMemData(mem_index) Low16bits(((MEMORY[mem_index][1] << 2*nibble) & 0xFF00) + (MEMORY[mem_index][0] & 0x00FF))
#define MEM(addr) getMemData(addr/2)
#define bit(pc, b) Low16bits((MEM(pc) >> b) & 0x0001)
#define bitVal(val, b) Low16bits(((val >> b) & 0x0001))
#define getSignExtend(val, sign_bit) Low16bits(bitVal(val, sign_bit) == 1 ? ((0xFFFF << (sign_bit + 1)) | val) \
                                                                : ((0xFFFF >> (16 - (sign_bit + 1))) & val))
#define SEXT(val, num_bits) getSignExtend(val, num_bits-1)
#define mPtr(pc) MEMORY[pc/2]
#define topByte(pc) Low16bits((mPtr(pc)[1] << 2*nibble1))
#define btmByte(pc) Low16bits((mPtr(pc)[0]))
#define nibble1(pc) ((mPtr(pc)[0]) & 0x000F)
#define nibble2(pc) ((mPtr(pc)[0]) & 0x00F0)
#define nibble3(pc) ((mPtr(pc)[1] << 2*nibble) & 0x0F00)

#define nibble4(pc) ((mPtr(pc)[1] << 2*nibble) & 0xF000)
#define arg1(pc) (MEM(pc) & 0x0E00)
#define adjArgOne(val) Low16bits(val << 2*nibble+1)
#define SEXT_VAL(val, n) Low16bits((0xFFFF << (16 - n)) | val)
#define ZEXT_VAL(val, n) Low16bits((0xFFFF >> n) & val)
#define LSHF(val) Low16bits(val << 1)
#define LSHFN(val, n) Low16bits(val << n)
#define RSHFN(val, n, b) b == 0 ? ZEXT_VAL(val >> n, n) : SEXT_VAL(val >> n, n)
#define ZEXT8(pc) (MEM(pc) & 0x00FF)
#define DR_NUM(pc) Low16bits((((MEM(pc) & 0x0E00) >> 9) & 0x0007))
#define SR1_NUM(pc) Low16bits((((MEM(pc) & 0x01C0) >> 6) & 0x0007))
#define SR2_NUM(pc) Low16bits((MEM(pc) & 0x0007))
#define DR(pc) Low16bits(CURRENT_LATCHES.REGS[DR_NUM(pc)])
#define SR1(pc) Low16bits(CURRENT_LATCHES.REGS[SR1_NUM(pc)])
#define SR2(pc) Low16bits(CURRENT_LATCHES.REGS[SR2_NUM(pc)])
#define amount4(pc) (MEM(pc) & 0x000F)
#define imm5(pc) (MEM(pc) & 0x001F)
#define boffset6(pc) (MEM(pc) & 0x003F)
#define offset6(pc) (MEM(pc) & 0x003F)
#define PCoffset9(pc) (MEM(pc) & 0x01FF)
#define PCoffset11(pc) (MEM(pc) & 0x07FF)
#define getPCoffset9(pc) Low16bits((incrmntdPC(pc) + LSHF(SEXT(PCoffset9(pc), 9))))
/*---------------------------------------------------------------------------------*/
#define low3bits(BEN, R, IR11) (((BEN << 2) + (R << 1) + IR11) & 0x0007)
#define ANRM  "\x1B[0m"
#define ARED  "\x1B[31m"
#define AGRN  "\x1B[32m"
#define AYEL  "\x1B[33m"
#define ABLU  "\x1B[34m"
#define AMAG  "\x1B[35m"
#define ACYN  "\x1B[36m"
#define AWHT  "\x1B[37m"


/**-------------------------------- Structures & Enums --------------------------------*/
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
enum OPCODES {
    add, and, br, brn, brp, brnp, brz, brnz, brzp, brnzp, jmp, ret, jsr, jsrr, ldb,
    ldw, lea, not, rti, lshf, rshfl, rshfa, stb, stw, trap, xor, halt, nop, fill
};
enum REGISTERS {
    r0, r1, r2, r3, r4, r5, r6, r7
};


/**--------------------------------- Global Variables ---------------------------------*/
const static char *enumStrings[] = {"DEBUG", "INFO", "WARN", "ERROR"};

const static char *lowerOpcodes[] = {"add", "and", "br", "brn", "brp", "brnp", "brz", "brnz", "brzp",
                                     "brnzp", "jmp", "ret", "jsr", "jsrr", "ldb", "ldw", "lea", "not",
                                     "rti", "lshf", "rshfl", "rshfa", "stb", "stw", "trap", "xor", "halt", "nop",
                                     ".fill"};

const static int opcodes[] = {1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 12, 12, 4, 4, 2,
                              6, 14, 9, 8, 13, 13, 13, 3, 7, 15, 9, 15, 0, 10};

const enum OPCODES opcodeEnum[] = {add, and, br, brn, brp, brnp, brz, brnz, brzp, brnzp,
                                   jmp, ret, jsr, jsrr, ldb, ldw, lea, not, rti, lshf,
                                   rshfl, rshfa, stb, stw, trap, xor, halt, nop, fill};


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

/*void processNop();

void setRegWithCC(int16_t reg, int16_t val);

void storeByteValue(uint16_t addr, int16_t val);

void storeWordVal(uint16_t addr, int16_t val);

int decodeAndExecute(int pc, enum OPCODES opcode);

int16_t MEM_BYTE(uint16_t addr);

void setupNextLatch();*/


bool isMemAccessState(int s);

/**-------------------------------- Function Definitions ------------------------------*/
/**
 * Evaluate the address of the next state according to the
 * micro sequencer logic. Latch the next microinstruction.
 */
void eval_micro_sequencer() {
    logging(CUR_LATCH, &CURRENT_LATCHES, debug);
    int COND, readyStatus, addrModeStatus, branchStatus, nextStateAddr, J;

    J = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
    COND = GetCOND(CURRENT_LATCHES.MICROINSTRUCTION);
    readyStatus = getReadyStatus(COND, CURRENT_LATCHES.READY);
    addrModeStatus = getAddrModeStatus(COND, bitVal(CURRENT_LATCHES.IR, 11));
    branchStatus = getBranchStatus(COND, CURRENT_LATCHES.BEN);

    nextStateAddr = Low16bits(J | low3bits(branchStatus, readyStatus, addrModeStatus));
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
    int curState, curReady, curMAR;

    curState = CURRENT_LATCHES.STATE_NUMBER;
    curReady = CURRENT_LATCHES.READY;
    curMAR = CURRENT_LATCHES.MAR;

    if (curReady == (MEM_CYCLES - 1)) {
        NEXT_LATCHES.MDR = MEM(curMAR);
        NEXT_LATCHES.READY = 0;
    }
    else if (isMemAccessState(curState)) NEXT_LATCHES.READY = curReady + 1;

}

/**
  * Datapath routine emulating operations before driving the bus.
  * Evaluate the input of tristate drivers
  *         Gate_MARMUX,
  *		 Gate_PC,
  *		 Gate_ALU,
  *		 Gate_SHF,
  *		 Gate_MDR.
  */
void eval_bus_drivers() {
}

/**
  * Datapath routine for driving the bus from one of the 5 possible
  * tristate drivers.
  */
void drive_bus() {

}

/**
  * Datapath routine for computing all functions that need to latch
  * values in the data path at the end of this cycle.  Some values
  * require sourcing the bus; therefore, this routine has to come
  * after drive_bus.
  */
void latch_datapath_values() {

}

void cpyMicroInst(int dst[], int src[]) {
    int i;

    for (i = 0; i < CONTROL_STORE_BITS; ++i) {
        dst[i] = src[i];
    }

}

int getBranchStatus(int cond, int BEN) {
    return cond == 2 && BEN;
}

int getAddrModeStatus(int cond, int IR11) {
    return cond == 3 && IR11;
}

int getReadyStatus(int cond, int R) {
    return cond == 1 && R == (MEM_CYCLES - 1);
}

bool isMemAccessState(int s) {
    return s == 33 || s == 28 || s == 29 ||
           s == 25 || s == 16 || s == 17;
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


/*
void process_instruction() {

    */
/*    uint16_t i = 0xE15F;
    uint16_t j = 0x215F;
    logging(S, "value is: ", I, RSHFN(i, 0x0003, bitVal(i, 15)), info);
    logging(S, "value is: ", I, RSHFN(j, 5, bitVal(j, 15)), info);
    logging(S, "value is: ", I, LSHFN(j, 5), info);
    logging(S, "value is: ", I, LSHFN(i, 0x0003), info);*//*

    */
/*  function: process_instruction
     *
     *    Process one instruction at a time
     *       -Fetch one instruction
     *       -Decode
     *       -Execute
     *       -Update NEXT_LATCHES
     *//*

    static int instruction_num = 0;
    logging(S, "Simulating instruction #", Id, instruction_num++, info);

    int pc = CURRENT_LATCHES.PC, next_pc;
    setupNextLatch();
    loggingNoHeader(STAT, &CURRENT_LATCHES, N,
                    S, "current instruction: ", ADDR, pc, debug);

    enum OPCODES opcode = fetch(nibble4(pc), pc);


    next_pc = decodeAndExecute(pc, opcode);


    NEXT_LATCHES.PC = next_pc;

    */
/*loggingMsgNoHeader(AGRN"PASS: "ANRM"simulation is done.", info);*//*

}


void setupNextLatch() {
    int i;
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
    NEXT_LATCHES.P = CURRENT_LATCHES.P;
    NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
    NEXT_LATCHES.N = CURRENT_LATCHES.N;
    for(i=0; i < LC_3b_REGS; i++)
        NEXT_LATCHES.REGS[i]=CURRENT_LATCHES.REGS[i];
}


int decodeAndExecute(int pc, enum OPCODES opcode) {
    int next_pc = pc + 2;
    int16_t val;

    switch (opcode) {
        case add:
            if (bit(pc, 5) == 0)
                setRegWithCC(DR_NUM(pc), SR1(pc) + SR2(pc));
            else
                setRegWithCC(DR_NUM(pc), SR1(pc) + SEXT(imm5(pc), 5));
            break;
        case and:
            if (bit(pc, 5) == 0)
                setRegWithCC(DR_NUM(pc), SR1(pc) & SR2(pc));
            else
                setRegWithCC(DR_NUM(pc), SR1(pc) & SEXT(imm5(pc), 5));
            break;
        case ldb:
            val = SR1(pc) + SEXT(boffset6(pc), 6);
            */
/**loggingNoHeader(S, AMAG"<LB> "ANRM"Loading ",
                            I, SEXT(MEM_BYTE(val), 9),
                            S, " into register R", I, DR_NUM(pc),
                            N, S, "offset: ", I, boffset6(pc),
                            N, S, "SEXT: ", I, SEXT(boffset6(pc), 6),
                            N, S, "SR1: ", I, SR1(pc),
                            N, S, "MEM_BYTE: ", I, MEM_BYTE(val),
                            N, S, "SR + offset: ", I, SR1(pc) + SEXT(boffset6(pc), 6),
                            N, S, "arg: ", I, val, info);*//*

            setRegWithCC(DR_NUM(pc), SEXT(MEM_BYTE(val), 9));
            break;
        case ldw:
            val = SR1(pc) + LSHF(SEXT(offset6(pc), 6));
            */
/**loggingNoHeader(S, AMAG"<LW> "ANRM"Loading ",
                            I, MEM(val),
                            S, " into register R", I, DR_NUM(pc),
                            N, S, "offset: ", I, offset6(pc),
                            N, S, "SEXT: ", I, SEXT(offset6(pc), 6),
                            N, S, "LSHF: ", I, LSHF(SEXT(offset6(pc), 6)),
                            N, S, "arg: ", I, val,
                            N, S, "SR1: ", I, SR1(pc), info);*//*

            setRegWithCC(DR_NUM(pc), MEM(val));
            break;
        case lshf:
        case rshfl:
        case rshfa:
            if (bit(pc, 4) == 0)
                setRegWithCC(DR_NUM(pc), LSHFN(SR1(pc), amount4(pc)));
            else if (bit(pc, 5) == 0)
                setRegWithCC(DR_NUM(pc), RSHFN(SR1(pc), amount4(pc), 0));
            else
                setRegWithCC(DR_NUM(pc), RSHFN(SR1(pc), amount4(pc), bitVal(SR1(pc), 15)));
            break;
        case stb:
            storeByteValue((uint16_t) (SR1(pc) + SEXT(boffset6(pc), 6)), Low16bits((DR(pc) & 0x00FF)));
            break;
        case stw:
            storeWordVal((uint16_t) (SR1(pc) + LSHF(SEXT(offset6(pc), 6))), Low16bits(DR(pc)));
            break;
        case not:
        case xor:
            if (bit(pc, 5) == 0)
                setRegWithCC(DR_NUM(pc), (SR1(pc) ^ SR2(pc)));
            else
                setRegWithCC(DR_NUM(pc), (SR1(pc) ^ SEXT(imm5(pc), 5)));
            break;
        case brn:
            if (CURRENT_LATCHES.N == TRUE)
                next_pc = getPCoffset9(pc);
            break;
        case brp:
            if (CURRENT_LATCHES.P == TRUE)
                next_pc = getPCoffset9(pc);
            break;
        case brnp:
            if (CURRENT_LATCHES.N == TRUE || CURRENT_LATCHES.P == TRUE)
                next_pc = getPCoffset9(pc);
            break;
        case brz:
            if (CURRENT_LATCHES.Z == TRUE)
                next_pc = getPCoffset9(pc);
            break;
        case brnz:
            if (CURRENT_LATCHES.N == TRUE || CURRENT_LATCHES.Z == TRUE)
                next_pc = getPCoffset9(pc);
            break;
        case brzp:
            if (CURRENT_LATCHES.Z == TRUE || CURRENT_LATCHES.P == TRUE)
                next_pc = getPCoffset9(pc);
            break;
        case br:
        case brnzp:
            next_pc = getPCoffset9(pc);
            break;
        case rti:
            */
/* Don't care! *//*

            break;
        case ret:
        case jmp:
            next_pc = Low16bits(SR1(pc));
            break;
        case jsrr:
        case jsr:
            if (TRUE) {
                int16_t temp = Low16bits(incrmntdPC(pc));
                if (bit(pc, 11) == 0) {
                    next_pc = Low16bits(SR1(pc));

                } else {
                    next_pc = Low16bits(incrmntdPC(pc) + LSHF(SEXT(PCoffset11(pc), 11)));
                }
                NEXT_LATCHES.REGS[r7] = temp;
            }
            break;
        case lea:
            NEXT_LATCHES.REGS[DR_NUM(pc)] =
                    Low16bits(incrmntdPC(pc) + LSHF(SEXT(PCoffset9(pc), 9)));
            break;
        case trap:
        case halt:
            NEXT_LATCHES.REGS[r7] = Low16bits(incrmntdPC(pc));
            next_pc = MEM(LSHF(ZEXT8(pc)));
            break;
        case nop:
            processNop();
            break;
        default:
            */
/* Don't care!*//*

            break;
    }
    return next_pc;
}


int16_t MEM_BYTE(uint16_t addr) {
    int16_t result;

    if (addr % 2 == 0) {
        result = (0x00FF & MEMORY[addr/2][0]);

    } else {
        addr -= 1;
        result = (0x00FF & MEMORY[addr/2][1]);
    }

    if (result & 0x0080)
        result = result + 0xFF00;

    return Low16bits(result);
}


void storeWordVal(uint16_t addr, int16_t val) {
    loggingNoHeader(S, "<SW> Storing ", I, val, S, " into address ", I, addr, info);
    int *mem = MEMORY[addr / 2];
    mem[0] = Low16bits(val & 0x000000FF);
    mem[1] = Low16bits(val & 0x0000FF00) >> 2*nibble;
}


void storeByteValue(uint16_t addr, int16_t val) {
    loggingNoHeader(S, "<SB> Storing ", I, val, S, " into address ", I, addr, info);
    int *mem = MEMORY[addr / 2];
    if (addr % 2 == 0) {
        mem[0] = Low16bits(val & 0x00FF);
    } else {
        mem[1] = Low16bits(val & 0x00FF);
    }
}


void setRegWithCC(int16_t reg, int16_t val) {
    if (val > 0) NEXT_LATCHES.P = TRUE;
    else NEXT_LATCHES.P = FALSE;

    if (val == 0) NEXT_LATCHES.Z = TRUE;
    else NEXT_LATCHES.Z = FALSE;

    if (val < 0) NEXT_LATCHES.N = TRUE;
    else NEXT_LATCHES.N = FALSE;

    NEXT_LATCHES.REGS[reg] = Low16bits(val);
}


void processNop() {
    int k;
    NEXT_LATCHES.N = CURRENT_LATCHES.N;
    NEXT_LATCHES.Z = CURRENT_LATCHES.Z;
    NEXT_LATCHES.P = CURRENT_LATCHES.P;
    for (k = 0; k < LC_3b_REGS; k++)
        NEXT_LATCHES.REGS[k] = CURRENT_LATCHES.REGS[k];
}


enum OPCODES fetch(int opcode, int pc) {
    loggingNoHeader(S, "finding opcode for ", Ix, opcode, debug);

    int j;
    enum OPCODES result = fill;

    for (j = 0; j < MAX_NUM_OPCODES; ++j)
        if (opcode == opcodes[j] << 3 * nibble)
            result = opcodeEnum[j];
    if (opcode == 0) {
        if (adjArgOne(4) == arg1(pc)) result = brn;
        else if (adjArgOne(1) == arg1(pc)) result = brp;
        else if (adjArgOne(5) == arg1(pc)) result = brnp;
        else if (adjArgOne(2) == arg1(pc)) result = brz;
        else if (adjArgOne(6) == arg1(pc)) result = brnz;
        else if (adjArgOne(3) == arg1(pc)) result = brzp;
        else if (adjArgOne(7) == arg1(pc)) result = br;
        else if (MEM(pc) == opcodes[nop]) result = nop;
        else {
            loggingMsg("Invalid instruction.", error);
            exit(4);
        }
    }

    loggingNoHeader(S, "Value ", Ix, opcode, S, " is for opcode ",
                    S, lowerOpcodes[result], info);

    return result;
}*/

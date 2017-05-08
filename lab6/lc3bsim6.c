/*
    Name 1: Aria Pahlavan
    UTEID 1: ap44342
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator - Lab 6                                   */
/*                                                             */
/*   EE 460N -- Spring 2013                                    */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/
void FETCH_stage();
void DE_stage();
void AGEX_stage();
void MEM_stage();
void SR_stage();
/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
int dependencyWhere(int *de_cs, int sr1, int sr2);

int canLoadPc();

#define TRUE  1
#define FALSE 0

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
/* control signals from the control store */
enum CS_BITS {
	SR1_NEEDED,
	SR2_NEEDED,
	DRMUX,

	ADDR1MUX,
	ADDR2MUX1, ADDR2MUX0,
	LSHF1,
	ADDRESSMUX,
	SR2MUX,
	ALUK1, ALUK0,
	ALU_RESULTMUX,

	BR_OP,
	UNCOND_OP,
	TRAP_OP,
	BR_STALL,

	DCACHE_EN,
	DCACHE_RW,
	DATA_SIZE,

	DR_VALUEMUX1, DR_VALUEMUX0,
	LD_REG,
	LD_CC,
	NUM_CONTROL_STORE_BITS

} CS_BITS;


enum AGEX_CS_BITS {

	AGEX_ADDR1MUX,
	AGEX_ADDR2MUX1, AGEX_ADDR2MUX0,
	AGEX_LSHF1,
	AGEX_ADDRESSMUX,
	AGEX_SR2MUX,
	AGEX_ALUK1, AGEX_ALUK0,
	AGEX_ALU_RESULTMUX,

	AGEX_BR_OP,
	AGEX_UNCOND_OP,
	AGEX_TRAP_OP,
	AGEX_BR_STALL,
	AGEX_DCACHE_EN,
	AGEX_DCACHE_RW,
	AGEX_DATA_SIZE,

	AGEX_DR_VALUEMUX1, AGEX_DR_VALUEMUX0,
	AGEX_LD_REG,
	AGEX_LD_CC,
	NUM_AGEX_CS_BITS
} AGEX_CS_BITS;

enum MEM_CS_BITS {
	MEM_BR_OP,
	MEM_UNCOND_OP,
	MEM_TRAP_OP,
	MEM_BR_STALL,
	MEM_DCACHE_EN,
	MEM_DCACHE_RW,
	MEM_DATA_SIZE,

	MEM_DR_VALUEMUX1, MEM_DR_VALUEMUX0,
	MEM_LD_REG,
	MEM_LD_CC,
	NUM_MEM_CS_BITS
} MEM_CS_BITS;

enum SR_CS_BITS {
	SR_DR_VALUEMUX1, SR_DR_VALUEMUX0,
	SR_LD_REG,
	SR_LD_CC,
	NUM_SR_CS_BITS
} SR_CS_BITS;


/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int Get_SR1_NEEDED(int *x)     { return (x[SR1_NEEDED]); }
int Get_SR2_NEEDED(int *x)     { return (x[SR2_NEEDED]); }
int Get_DRMUX(int *x)          { return (x[DRMUX]);}
int Get_DE_BR_OP(int *x)       { return (x[BR_OP]); }
int Get_ADDR1MUX(int *x)       { return (x[AGEX_ADDR1MUX]); }
int Get_ADDR2MUX(int *x)       { return ((x[AGEX_ADDR2MUX1] << 1) + x[AGEX_ADDR2MUX0]); }
int Get_LSHF1(int *x)          { return (x[AGEX_LSHF1]); }
int Get_ADDRESSMUX(int *x)     { return (x[AGEX_ADDRESSMUX]); }
int Get_SR2MUX(int *x)          { return (x[AGEX_SR2MUX]); }
int Get_ALUK(int *x)           { return ((x[AGEX_ALUK1] << 1) + x[AGEX_ALUK0]); }
int Get_ALU_RESULTMUX(int *x)  { return (x[AGEX_ALU_RESULTMUX]); }
int Get_BR_OP(int *x)          { return (x[MEM_BR_OP]); }
int Get_UNCOND_OP(int *x)      { return (x[MEM_UNCOND_OP]); }
int Get_TRAP_OP(int *x)        { return (x[MEM_TRAP_OP]); }
int Get_DCACHE_EN(int *x)      { return (x[MEM_DCACHE_EN]); }
int Get_DCACHE_RW(int *x)      { return (x[MEM_DCACHE_RW]); }
int Get_DATA_SIZE(int *x)      { return (x[MEM_DATA_SIZE]); }
int Get_DR_VALUEMUX1(int *x)   { return ((x[SR_DR_VALUEMUX1] << 1 ) + x[SR_DR_VALUEMUX0]); }
int Get_AGEX_LD_REG(int *x)    { return (x[AGEX_LD_REG]); }
int Get_AGEX_LD_CC(int *x)     { return (x[AGEX_LD_CC]); }
int Get_MEM_LD_REG(int *x)     { return (x[MEM_LD_REG]); }
int Get_MEM_LD_CC(int *x)      { return (x[MEM_LD_CC]); }
int Get_SR_LD_REG(int *x)      { return (x[SR_LD_REG]); }
int Get_SR_LD_CC(int *x)       { return (x[SR_LD_CC]); }
int Get_DE_BR_STALL(int *x)    { return (x[BR_STALL]); }
int Get_AGEX_BR_STALL(int *x)  { return (x[AGEX_BR_STALL]); }
int Get_MEM_BR_STALL(int *x)   { return (x[MEM_BR_STALL]); }



/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][NUM_CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
   There are two write enable signals, one for each byte. WE0 is used for
   the least significant byte of a word. WE1 is used for the most significant
   byte of a word. */

#define WORDS_IN_MEM    0x08000
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/
/* The LC-3b register file.                                      */
/***************************************************************/
#define LC3b_REGS 8
int REGS[LC3b_REGS];
/***************************************************************/
/* architectural state */
/***************************************************************/
int  PC,  	/* program counter */
		N,		/* n condition bit */
		Z = 1,	/* z condition bit */
		P;		/* p condition bit */
/***************************************************************/
/* LC-3b State info.                                             */
/***************************************************************/

typedef struct PipeState_Entry_Struct{

	/* DE latches */
	int DE_NPC,
			DE_IR,
			DE_V,
	/* AGEX lateches */
			AGEX_NPC,
			AGEX_SR1,
			AGEX_SR2,
			AGEX_CC,
			AGEX_IR,
			AGEX_DRID,
			AGEX_V,
			AGEX_CS[NUM_AGEX_CS_BITS],
	/* MEM latches */
			MEM_NPC,
			MEM_ALU_RESULT,
			MEM_ADDRESS,
			MEM_CC,
			MEM_IR,
			MEM_DRID,
			MEM_V,
			MEM_CS[NUM_MEM_CS_BITS],
	/* SR latches */
			SR_NPC,
			SR_DATA,
			SR_ALU_RESULT,
			SR_ADDRESS,
			SR_IR,
			SR_DRID,
			SR_V,
			SR_CS[NUM_SR_CS_BITS];

} PipeState_Entry;

/* data structure for latch */
PipeState_Entry PS, NEW_PS;

/* simulator signal */
int RUN_BIT;

/* Internal stall signals */
int   dep_stall,
		v_de_br_stall,
		v_agex_br_stall,
		v_mem_br_stall,
		mem_stall,
		icache_r;

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
	printf("rdump            -  dump the architectural state    \n");
	printf("idump            -  dump the internal state         \n");
	printf("?                -  display this help menu          \n");
	printf("quit             -  exit the program                \n\n");
}

void print_CS(int *CS, int num)
{
	int ii ;
	for ( ii = 0 ; ii < num; ii++) {
		printf("%d",CS[ii]);
	}
	printf("\n");
}
/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {
	NEW_PS = PS;
	SR_stage();
	MEM_stage();
	AGEX_stage();
	DE_stage();
	FETCH_stage();
	PS = NEW_PS;
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
		if (PC == 0x0000) {
			cycle();
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
	if ((RUN_BIT == FALSE) || (PC == 0x0000)) {
		printf("Can't simulate, Simulator is halted\n\n");
		return;
	}
	printf("Simulating...\n\n");
	/* initialization */
	while (PC != 0x0000)
		cycle();
	cycle();
	RUN_BIT = FALSE;
	printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a region of memory to the output file.     */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
	int address; /* this is a byte address */

	printf("\nMemory content [0x%04x..0x%04x] :\n", start, stop);
	printf("-------------------------------------\n");
	for (address = (start >> 1); address <= (stop >> 1); address++)
		printf("  0x%04x (%d) : 0x%02x%02x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
	printf("\n");

	/* dump the memory contents into the dumpsim file */
	fprintf(dumpsim_file, "\nMemory content [0x%04x..0x%04x] :\n", start, stop);
	fprintf(dumpsim_file, "-------------------------------------\n");
	for (address = (start >> 1); address <= (stop >> 1); address++)
		fprintf(dumpsim_file, " 0x%04x (%d) : 0x%02x%02x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
	fprintf(dumpsim_file, "\n");
	fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current architectural state  to the       */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
	int k;

	printf("\nCurrent architectural state :\n");
	printf("-------------------------------------\n");
	printf("Cycle Count : %d\n", CYCLE_COUNT);
	printf("PC          : 0x%04x\n", PC);
	printf("CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
	printf("Registers:\n");
	for (k = 0; k < LC3b_REGS; k++)
		printf("%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
	printf("\n");

	/* dump the state information into the dumpsim file */
	fprintf(dumpsim_file, "\nCurrent architectural state :\n");
	fprintf(dumpsim_file, "-------------------------------------\n");
	fprintf(dumpsim_file, "Cycle Count : %d\n", CYCLE_COUNT);
	fprintf(dumpsim_file, "PC          : 0x%04x\n", PC);
	fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
	fprintf(dumpsim_file, "Registers:\n");
	for (k = 0; k < LC3b_REGS; k++)
		fprintf(dumpsim_file, "%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
	fprintf(dumpsim_file, "\n");
	fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : idump                                           */
/*                                                             */
/* Purpose   : Dump current internal state to the              */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void idump(FILE * dumpsim_file) {
	int k;

	printf("\nCurrent architectural state :\n");
	printf("-------------------------------------\n");
	printf("Cycle Count     : %d\n", CYCLE_COUNT);
	printf("PC              : 0x%04x\n", PC);
	printf("CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
	printf("Registers:\n");
	for (k = 0; k < LC3b_REGS; k++)
		printf("%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
	printf("\n");

	printf("------------- Stall Signals -------------\n");
	printf("ICACHE_R        :  %d\n", icache_r);
	printf("DEP_STALL       :  %d\n", dep_stall);
	printf("V_DE_BR_STALL   :  %d\n", v_de_br_stall);
	printf("V_AGEX_BR_STALL :  %d\n", v_agex_br_stall);
	printf("MEM_STALL       :  %d\n", mem_stall);
	printf("V_MEM_BR_STALL  :  %d\n", v_mem_br_stall);
	printf("\n");

	printf("------------- DE   Latches --------------\n");
	printf("DE_NPC          :  0x%04x\n", PS.DE_NPC );
	printf("DE_IR           :  0x%04x\n", PS.DE_IR );
	printf("DE_V            :  %d\n", PS.DE_V);
	printf("\n");

	printf("------------- AGEX Latches --------------\n");
	printf("AGEX_NPC        :  0x%04x\n", PS.AGEX_NPC );
	printf("AGEX_SR1        :  0x%04x\n", PS.AGEX_SR1 );
	printf("AGEX_SR2        :  0x%04x\n", PS.AGEX_SR2 );
	printf("AGEX_CC         :  %d\n", PS.AGEX_CC );
	printf("AGEX_IR         :  0x%04x\n", PS.AGEX_IR );
	printf("AGEX_DRID       :  %d\n", PS.AGEX_DRID);
	printf("AGEX_CS         :  ");
	for ( k = 0 ; k < NUM_AGEX_CS_BITS; k++) {
		printf("%d",PS.AGEX_CS[k]);
	}
	printf("\n");
	printf("AGEX_V          :  %d\n", PS.AGEX_V);
	printf("\n");

	printf("------------- MEM  Latches --------------\n");
	printf("MEM_NPC         :  0x%04x\n", PS.MEM_NPC );
	printf("MEM_ALU_RESULT  :  0x%04x\n", PS.MEM_ALU_RESULT );
	printf("MEM_ADDRESS     :  0x%04x\n", PS.MEM_ADDRESS );
	printf("MEM_CC          :  %d\n", PS.MEM_CC );
	printf("MEM_IR          :  0x%04x\n", PS.MEM_IR );
	printf("MEM_DRID        :  %d\n", PS.MEM_DRID);
	printf("MEM_CS          :  ");
	for ( k = 0 ; k < NUM_MEM_CS_BITS; k++) {
		printf("%d",PS.MEM_CS[k]);
	}
	printf("\n");
	printf("MEM_V           :  %d\n", PS.MEM_V);
	printf("\n");

	printf("------------- SR   Latches --------------\n");
	printf("SR_NPC          :  0x%04x\n", PS.SR_NPC );
	printf("SR_DATA         :  0x%04x\n", PS.SR_DATA );
	printf("SR_ALU_RESULT   :  0x%04x\n", PS.SR_ALU_RESULT );
	printf("SR_ADDRESS      :  0x%04x\n", PS.SR_ADDRESS );
	printf("SR_IR           :  0x%04x\n", PS.SR_IR );
	printf("SR_DRID         :  %d\n", PS.SR_DRID);
	printf("SR_CS           :  ");
	for ( k = 0 ; k < NUM_SR_CS_BITS; k++) {
		printf("%d",PS.SR_CS[k]);
	}
	printf("\n");
	printf("SR_V            :  %d\n", PS.SR_V);

	printf("\n");

	/* dump the state information into the dumpsim file */
	fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
	fprintf(dumpsim_file,"\nCurrent architectural state :\n");
	fprintf(dumpsim_file,"-------------------------------------\n");
	fprintf(dumpsim_file,"Cycle Count     : %d\n", CYCLE_COUNT);
	fprintf(dumpsim_file,"PC              : 0x%04x\n", PC);
	fprintf(dumpsim_file,"CCs: N = %d  Z = %d  P = %d\n", N, Z, P);
	fprintf(dumpsim_file,"Registers:\n");
	for (k = 0; k < LC3b_REGS; k++)
		fprintf(dumpsim_file,"%d: 0x%04x\n", k, (REGS[k] & 0xFFFF));
	fprintf(dumpsim_file,"\n");

	fprintf(dumpsim_file,"------------- Stall Signals -------------\n");
	fprintf(dumpsim_file,"ICACHE_R        :  %d\n", icache_r);
	fprintf(dumpsim_file,"DEP_STALL       :  %d\n", dep_stall);
	fprintf(dumpsim_file,"V_DE_BR_STALL   :  %d\n", v_de_br_stall);
	fprintf(dumpsim_file,"V_AGEX_BR_STALL :  %d\n", v_agex_br_stall);
	fprintf(dumpsim_file,"MEM_STALL       :  %d\n", mem_stall);
	fprintf(dumpsim_file,"V_MEM_BR_STALL  :  %d\n", v_mem_br_stall);
	fprintf(dumpsim_file,"\n");

	fprintf(dumpsim_file,"------------- DE   Latches --------------\n");
	fprintf(dumpsim_file,"DE_NPC          :  0x%04x\n", PS.DE_NPC );
	fprintf(dumpsim_file,"DE_IR           :  0x%04x\n", PS.DE_IR );
	fprintf(dumpsim_file,"DE_V            :  %d\n", PS.DE_V);
	fprintf(dumpsim_file,"\n");

	fprintf(dumpsim_file,"------------- AGEX Latches --------------\n");
	fprintf(dumpsim_file,"AGEX_NPC        :  0x%04x\n", PS.AGEX_NPC );
	fprintf(dumpsim_file,"AGEX_SR1        :  0x%04x\n", PS.AGEX_SR1 );
	fprintf(dumpsim_file,"AGEX_SR2        :  0x%04x\n", PS.AGEX_SR2 );
	fprintf(dumpsim_file,"AGEX_CC         :  %d\n", PS.AGEX_CC );
	fprintf(dumpsim_file,"AGEX_IR         :  0x%04x\n", PS.AGEX_IR );
	fprintf(dumpsim_file,"AGEX_DRID       :  %d\n", PS.AGEX_DRID);
	fprintf(dumpsim_file,"AGEX_CS         :  ");
	for ( k = 0 ; k < NUM_AGEX_CS_BITS; k++) {
		fprintf(dumpsim_file,"%d",PS.AGEX_CS[k]);
	}
	fprintf(dumpsim_file,"\n");
	fprintf(dumpsim_file,"AGEX_V          :  %d\n", PS.AGEX_V);
	fprintf(dumpsim_file,"\n");

	fprintf(dumpsim_file,"------------- MEM  Latches --------------\n");
	fprintf(dumpsim_file,"MEM_NPC         :  0x%04x\n", PS.MEM_NPC );
	fprintf(dumpsim_file,"MEM_ALU_RESULT  :  0x%04x\n", PS.MEM_ALU_RESULT );
	fprintf(dumpsim_file,"MEM_ADDRESS     :  0x%04x\n", PS.MEM_ADDRESS );
	fprintf(dumpsim_file,"MEM_CC          :  %d\n", PS.MEM_CC );
	fprintf(dumpsim_file,"MEM_IR          :  0x%04x\n", PS.MEM_IR );
	fprintf(dumpsim_file,"MEM_DRID        :  %d\n", PS.MEM_DRID);
	fprintf(dumpsim_file,"MEM_CS          :  ");
	for ( k = 0 ; k < NUM_MEM_CS_BITS; k++) {
		fprintf(dumpsim_file,"%d",PS.MEM_CS[k]);
	}
	fprintf(dumpsim_file,"\n");
	fprintf(dumpsim_file,"MEM_V           :  %d\n", PS.MEM_V);
	fprintf(dumpsim_file,"\n");

	fprintf(dumpsim_file,"------------- SR   Latches --------------\n");
	fprintf(dumpsim_file,"SR_NPC          :  0x%04x\n", PS.SR_NPC );
	fprintf(dumpsim_file,"SR_DATA         :  0x%04x\n",PS.SR_DATA );
	fprintf(dumpsim_file,"SR_ALU_RESULT   :  0x%04x\n", PS.SR_ALU_RESULT );
	fprintf(dumpsim_file,"SR_ADDRESS      :  0x%04x\n", PS.SR_ADDRESS );
	fprintf(dumpsim_file,"SR_IR           :  0x%04x\n", PS.SR_IR );
	fprintf(dumpsim_file,"SR_DRID         :  %d\n", PS.SR_DRID);
	fprintf(dumpsim_file,"SR_CS           :  ");
	for ( k = 0 ; k < NUM_SR_CS_BITS; k++) {
		fprintf(dumpsim_file, "%d",PS.SR_CS[k]);
	}
	fprintf(dumpsim_file,"\n");
	fprintf(dumpsim_file,"SR_V            :  %d\n", PS.SR_V);

	fprintf(dumpsim_file,"\n");
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

		case 'I':
		case 'i':
			idump(dumpsim_file);
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

		for (j = 0; j < NUM_CONTROL_STORE_BITS; j++) {
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

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
	int i;

	for (i=0; i < WORDS_IN_MEM; i++) {
		MEMORY[i][0] = 0;
		MEMORY[i][1] = 0;
	}
}


/***************************************************************/
/*                                                             */
/* Procedure : init_state                                      */
/*                                                             */
/* Purpose   : Zero out all latches and registers              */
/*                                                             */
/***************************************************************/
void init_state() {

	memset(&PS, 0 ,sizeof(PipeState_Entry));
	memset(&NEW_PS, 0 , sizeof(PipeState_Entry));

	dep_stall       = 0;
	v_de_br_stall   = 0;
	v_agex_br_stall = 0;
	v_mem_br_stall  = 0;
	mem_stall       = 0;
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
		program_base = word >> 1 ;
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

	if (PC == 0) PC  = program_base << 1 ;
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
	init_state();

	RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* dcache_access                                               */
/*                                                             */
/***************************************************************/
void dcache_access(int dcache_addr, int *read_word, int write_word, int *dcache_r,
                   int mem_w0, int mem_w1) {

	int addr = dcache_addr >> 1 ;
	int random = CYCLE_COUNT % 9;

	if (!random) {
		*dcache_r = 0;
		*read_word = 0xfeed ;
	}
	else {
		*dcache_r = 1;

		*read_word = (MEMORY[addr][1] << 8) | (MEMORY[addr][0] & 0x00FF);
		if(mem_w0) MEMORY[addr][0] = write_word & 0x00FF;
		if(mem_w1) MEMORY[addr][1] = (write_word & 0xFF00) >> 8;
	}
}
/***************************************************************/
/*                                                             */
/* icache_access                                               */
/*                                                             */
/***************************************************************/
void icache_access(int icache_addr, int *read_word, int *icache_r) {

	int addr = icache_addr >> 1 ;
	int random = CYCLE_COUNT % 13;

	if (!random) {
		*icache_r = 0;
		*read_word = 0xfeed;
	}
	else {
		*icache_r = 1;
		*read_word = MEMORY[addr][1] << 8 | MEMORY[addr][0];
	}
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

   RUN_BIT
   REGS
   MEMORY

   PC
   N
   Z
   P

   dep_stall
   v_de_br_stall
   v_agex_br_stall
   v_mem_br_stall
   mem_stall
   icache_r

   PS
   NEW_PS


   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.


   Begin your code here 	  			       */
/***************************************************************/

#include <stdbool.h>
#include <stdint.h>


/**----------------------------------- Definitions ------------------------------------*/
#define MANUAL_DEBUG        FALSE
#define EMPTY_VAL           0
#define LOG_LEVEL           DEBUG
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
#define align(addr)         (addr & 0xFFFE)
#define STB_MDR(val)        Low16bits( (lowByte(val) << 2*nibble) + lowByte(val) )
#define IR15_12(IR)         Low16bits( ((IR & 0xF000) >> 3*nibble) & 0x000F )

#define BEN(IR, CC)         (( ( (bit(IR, 11) == 1) && (bit(CC, 2) == 1) ) \
							|| ( (bit(IR, 10) == 1) && (bit(CC, 1) == 1) ) \
							|| ( (bit(IR, 9)  == 1) && (bit(CC, 0) == 1) )) ? 1 : 0)
#define CS_ADDR(IR)         Low16bits( (((IR & 0xF800)>>10)  + (bit(IR, 5))) )
#define ccfy(n,z,p)         Low16bits( (n<<2) + (z<<1) + p )
#define reg(num)            Low16bits( REGS[num] )


/* R.W */
#define RD_         0
#define WR_         1
/* DATA.SIZE */
#define BYTE_       0
#define WORD_       1
/* PCMUX */
#define PCplus2_    0
#define TARGET_PC_  1
#define TRAP_PC_    2
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
#define AWHT  "\x1B[37m"

/*--------------------LAB 6------------------*/
#define COPY_AGEX_CS_START 3
#define COPY_MEM_CS_START 9
#define COPY_SR_CS_START  7


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
/*GATES_STRUCT GATE_INPUTS;*/
const static char *enumStrings[] = {"DEBUG", "INFO", "WARN", "ERROR"};



/**-------------------------------- Function Definitions ------------------------------*/


/* Signals generated by SR stage and needed by previous stages in the
   pipeline are declared below. */
int sr_reg_data,
		sr_n, sr_z, sr_p,
		v_sr_ld_cc,
		v_sr_ld_reg,
		sr_reg_id;


/************************* SR_stage() *************************/
void SR_stage() {

	/* You are given the code for SR_stage to get you started. Look at
	   the figure for SR stage to see how this code is implemented. */

	switch (Get_DR_VALUEMUX1(PS.SR_CS))
	{
		case 0:
			sr_reg_data = PS.SR_ADDRESS ;
			break;
		case 1:
			sr_reg_data = PS.SR_DATA ;
			break;
		case 2:
			sr_reg_data = PS.SR_NPC ;
			break;
		case 3:
			sr_reg_data = PS.SR_ALU_RESULT ;
			break;
	}

	sr_reg_id = PS.SR_DRID;
	v_sr_ld_reg = Get_SR_LD_REG(PS.SR_CS) & PS.SR_V;
	v_sr_ld_cc = Get_SR_LD_CC(PS.SR_CS) & PS.SR_V ;

	/* CC LOGIC  */
	sr_n = ((sr_reg_data & 0x8000) ? 1 : 0);
	sr_z = ((sr_reg_data & 0xFFFF) ? 0 : 1);
	sr_p = 0;
	if ((!sr_n) && (!sr_z))
		sr_p = 1;

}


int mem_dr_id,
	mem_pc_mux,
	target_pc,
	trap_pc,
	v_mem_ld_cc,
	v_mem_ld_reg;
/************************* MEM_stage() *************************/
void MEM_stage() {

	int ii,jj = 0;

	/* your code for MEM stage goes here */
	mem_dr_id     = PS.MEM_DRID;
	int mem_ir    = PS.MEM_IR,
		mem_cc    = PS.MEM_CC,
		mem_v     = PS.MEM_V,
		br_op     = Get_BR_OP(PS.MEM_CS),
		uncond_op =	Get_UNCOND_OP(PS.MEM_CS),
		trap_op   = Get_TRAP_OP(PS.MEM_CS);

	mem_pc_mux = mem_v==0     ? 0:
                 br_op==1     ? BEN(mem_ir, mem_cc):
                 uncond_op==1 ? 1:
                 trap_op==1   ? 2:
                                0;

	int dcache_en   = Get_DCACHE_EN(PS.MEM_CS),
		dcache_rw   = Get_DCACHE_RW(PS.MEM_CS),
		data_size   = Get_DATA_SIZE(PS.MEM_CS),
		mem_alu_res = PS.MEM_ALU_RESULT,
		mem_address = PS.MEM_ADDRESS,
		mem_addr_0  = bit(mem_address, 0);



	int v_dcache_en = dcache_en & mem_v,
		dcache_addr  = mem_address,
		read_word    = 0,
		dcache_r     = 0,

		write_word   = data_size==1  ? mem_alu_res :
		               mem_addr_0==1 ? Low16bits( (mem_alu_res<<(2*nibble)) | (mem_alu_res & 0x00FF) ):
		                               mem_alu_res,

		mem_w0       = dcache_rw==1 && (data_size==1 || mem_addr_0==0),
		mem_w1       = dcache_rw==1 && (data_size==1 || mem_addr_0==1);


	if(dcache_en)
		dcache_access(dcache_addr,
		              &read_word,
		              write_word,
		              &dcache_r,
		              mem_w0, mem_w1);
	else
		dcache_r=1;


	mem_stall = (!dcache_r) & v_dcache_en;

	target_pc = mem_address;

	int mem_ld_cc    = Get_MEM_LD_CC(PS.MEM_CS),
		mem_ld_reg   = Get_MEM_LD_REG(PS.MEM_CS),
		mem_br_stall = Get_MEM_BR_STALL(PS.MEM_CS);

	v_mem_br_stall = mem_br_stall & mem_v;
	v_mem_ld_cc    = mem_ld_cc & mem_v;
	v_mem_ld_reg   = mem_ld_reg & mem_v;


	trap_pc = v_dcache_en==0 ? 0 :
              data_size==1   ? read_word :
              mem_addr_0==0  ? lowByte(read_word) :
                               SEXT(lowByte((read_word>>(2*nibble))), 8);



	/* The code below propagates the control signals from MEM.CS latch
	   to SR.CS latch. You still need to latch other values into the
	   other SR latches. */
	for (ii=COPY_SR_CS_START; ii < NUM_MEM_CS_BITS; ii++) {
		NEW_PS.SR_CS [jj++] = PS.MEM_CS [ii];
	}

	NEW_PS.SR_V = (!mem_stall) && mem_v;

	NEW_PS.SR_ALU_RESULT = mem_alu_res;
	NEW_PS.SR_ADDRESS    = mem_address;
	NEW_PS.SR_DRID       = PS.MEM_DRID;
	NEW_PS.SR_DATA       = trap_pc;
	NEW_PS.SR_NPC        = PS.MEM_NPC;
	NEW_PS.SR_IR         = mem_ir;
}



int agex_dr_id,
	v_agex_ld_cc,
	v_agex_ld_reg;
/************************* AGEX_stage() *************************/
void AGEX_stage() {

	int ii, jj = 0;
	int LD_MEM; /* You need to write code to compute the value of LD.MEM
		 signal */

	/* your code for AGEX stage goes here */
	agex_dr_id = PS.AGEX_DRID;

	int *agex_cs   = PS.AGEX_CS,
		agex_npc   = PS.AGEX_NPC,
		agex_ir    = PS.AGEX_IR,
		agex_sr1   = PS.AGEX_SR1,
		agex_sr2   = PS.AGEX_SR2,
		agex_cc    = PS.AGEX_CC,
		agex_v     = PS.AGEX_V;

	int addr1_res,
		addr2_res,
		mem_addr_res,
		addr1mux   = Get_ADDR1MUX(agex_cs),
		addr2mux   = Get_ADDR2MUX(agex_cs),
		lshf1      = Get_LSHF1(agex_cs),
		addressmux = Get_ADDRESSMUX(agex_cs),
		aluk       = Get_ALUK(agex_cs),
		sr2mux     = Get_SR2MUX(agex_cs),
		alu_resmux = Get_ALU_RESULTMUX(agex_cs);

	int mem_alu_res=0,
		shf_res,
		off6 = boffset6(agex_ir),
		a = agex_sr1,
		b = sr2mux==SR2_ ? agex_sr2 :
                           imm5(agex_ir);



	switch (aluk) {
		case ADD_:   mem_alu_res = Low16bits(a+b); break;
		case AND_:   mem_alu_res = Low16bits(a&b); break;
		case XOR_:   mem_alu_res = Low16bits(a^b); break;
		case PASSA_: mem_alu_res = Low16bits(b);   break;
	}

	if (bit(off6, 4) == 0)       shf_res = LSHFN(agex_sr1, amount4(agex_ir));
	else if (bit(off6, 5) == 0)  shf_res = RSHFN(agex_sr1, amount4(agex_ir), 0);
	else                         shf_res = RSHFN(agex_sr1, amount4(agex_ir), bit(agex_sr1, 15));

	mem_alu_res = alu_resmux==0 ? shf_res
	                        : mem_alu_res;


	addr1_res = (addr1mux==PC_ ? agex_npc : agex_sr1);

	switch (addr2mux) {
		case ZERO_:       addr2_res = 0;                   break;
		case offset6_:    addr2_res = offset6(agex_ir);    break;
		case PCoffset9_:  addr2_res = PCoffset9(agex_ir);  break;
		case PCoffset11_: addr2_res = PCoffset11(agex_ir); break;
		default:          addr2_res = 0;                   break;
	}

	addr2_res = (lshf1== YES_ ? LSHF(addr2_res)
	                          : addr2_res);
	mem_addr_res = (addressmux == B7_0_ ? LSHF(ZEXT8(agex_ir))
	                                    : addr1_res + addr2_res);

	int agex_ld_cc    = Get_AGEX_LD_CC(PS.AGEX_CS),
		agex_ld_reg   = Get_AGEX_LD_REG(PS.AGEX_CS),
		agex_br_stall = Get_AGEX_BR_STALL(PS.AGEX_CS);

	v_agex_br_stall = agex_br_stall & agex_v;
	v_agex_ld_cc    = agex_ld_cc & agex_v;
	v_agex_ld_reg   = agex_ld_reg & agex_v;


	LD_MEM = mem_stall==1 ? 0 : 1;


	if (LD_MEM) {
		/* Your code for latching into MEM latches goes here */
		NEW_PS.MEM_ALU_RESULT = mem_alu_res;
		NEW_PS.MEM_ADDRESS    = mem_addr_res;
		NEW_PS.MEM_DRID       = PS.AGEX_DRID;
		NEW_PS.MEM_NPC        = agex_npc;
		NEW_PS.MEM_CC         = agex_cc;
		NEW_PS.MEM_IR         = agex_ir;
		NEW_PS.MEM_V          = agex_v;


		/* The code below propagates the control signals from AGEX.CS latch
		   to MEM.CS latch. */
		for (ii = COPY_MEM_CS_START; ii < NUM_AGEX_CS_BITS; ii++) {
			NEW_PS.MEM_CS [jj++] = PS.AGEX_CS [ii];
		}
	}
}



/************************* DE_stage() *************************/
void DE_stage() {

	int CONTROL_STORE_ADDRESS;  /* You need to implement the logic to
			         set the value of this variable. Look
			         at the figure for DE stage */
	int ii, jj = 0;
	int LD_AGEX; /* You need to write code to compute the value of
		  LD.AGEX signal */

	/* your code for DE stage goes here */
	int de_agex_dr_id,
		de_npc = PS.DE_NPC,
		de_ir  = PS.DE_IR,
		de_v   = PS.DE_V;

	CONTROL_STORE_ADDRESS = CS_ADDR(de_ir);
	int *de_cs = CONTROL_STORE[CONTROL_STORE_ADDRESS];

	int agex_cc,
		agex_sr1,
		agex_sr2,
		agex_v;

	v_de_br_stall = de_v && de_cs[BR_STALL];



	if (v_sr_ld_cc==1)
	{
		N = sr_n;
		Z = sr_z;
		P = sr_p;
	}

	int sr1      = SR1_NUM(de_ir),
		sr2_11_9 = DR_NUM(de_ir),
		sr2_2_0  = SR2_NUM(de_ir),
		sr2      = bit(de_ir, 13)==0 ? sr2_2_0
		                             : sr2_11_9;

	agex_sr1 = reg(sr1);
	agex_sr2 = reg(sr2);

	if(v_sr_ld_reg==1)
		REGS[sr_reg_id] = Low16bits( sr_reg_data );

	agex_cc = ccfy(N,Z,P);

	LD_AGEX = mem_stall==1 ? 0
	                       : 1;


	de_agex_dr_id = de_cs[DRMUX]==0 ? DR_NUM(de_ir)
	                                : 7;

	dep_stall = dependencyWhere(de_cs, sr1, sr2);
	agex_v = (!dep_stall) && de_v;


	if (LD_AGEX) {
		/* Your code for latching into AGEX latches goes here */

		NEW_PS.AGEX_DRID = de_agex_dr_id;
		NEW_PS.AGEX_SR1  = agex_sr1;
		NEW_PS.AGEX_SR2  = agex_sr2;
		NEW_PS.AGEX_NPC  = de_npc;
		NEW_PS.AGEX_IR   = de_ir;
		NEW_PS.AGEX_CC   = agex_cc;
		NEW_PS.AGEX_V    = agex_v;

		/* The code below propagates the control signals from the CONTROL
		   STORE to the AGEX.CS latch. */
		for (ii = COPY_AGEX_CS_START; ii< NUM_CONTROL_STORE_BITS; ii++) {
			NEW_PS.AGEX_CS[jj++] = CONTROL_STORE[CONTROL_STORE_ADDRESS][ii];
		}
	}

}

int dependencyWhere(int *de_cs, int sr1, int sr2) {
	int sr1_needed = Get_SR1_NEEDED(de_cs),
		sr2_needed = Get_SR2_NEEDED(de_cs),
		de_br_op   = Get_DE_BR_OP(de_cs);

	if (PS.DE_V==0) return 0;

	if(   (sr1_needed && v_agex_ld_reg && (sr1==agex_dr_id))
	   || (sr1_needed && v_mem_ld_reg  && (sr1==mem_dr_id))
	   || (sr1_needed && v_sr_ld_reg   && (sr1==sr_reg_id))
	   || (sr2_needed && v_agex_ld_reg && (sr2==agex_dr_id))
	   || (sr2_needed && v_mem_ld_reg  && (sr2==mem_dr_id))
	   || (sr2_needed && v_sr_ld_reg   && (sr2==sr_reg_id))
	   || (de_br_op && ( v_agex_ld_cc || v_mem_ld_cc || v_sr_ld_cc )))
		return 1;

	else return 0;
}


/************************* FETCH_stage() *************************/
void FETCH_stage() {

	/* your code for FETCH stage goes here */
	int LD_DE,
		ld_pc,
		de_npc = Low16bits(PC+2),
		de_ir = 0,
		de_v;

	icache_access(PC, &de_ir, &icache_r);

	ld_pc = canLoadPc();

	if(ld_pc) {
		PC = mem_pc_mux == PCplus2_   ? Low16bits(PC+2) :
		     mem_pc_mux == TARGET_PC_ ? Low16bits(target_pc) :
							     	    Low16bits(trap_pc);
	}

	de_v = (ld_pc==0)||v_mem_br_stall ? 0
	                                  : 1;

	LD_DE = mem_stall||dep_stall ? 0
	                             : 1;

	if(LD_DE) {
		NEW_PS.DE_NPC = de_npc;
		NEW_PS.DE_IR  = de_ir;
		NEW_PS.DE_V   = de_v;
	}
}

int canLoadPc() {
	if ((icache_r && !dep_stall && !mem_stall && !v_de_br_stall && !v_agex_br_stall && !v_mem_br_stall)
	    || ( v_mem_br_stall && (mem_pc_mux!=0) ))
		return 1;
	else
		return 0;
}


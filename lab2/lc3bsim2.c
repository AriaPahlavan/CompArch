/*
    Remove all unnecessary lines (including this one) 
    in this comment.
    REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

    Name 1: Aria Pahlavan
    UTEID 1: ap44342
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

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
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;    /* run bit */


typedef struct System_Latches_Struct {

    int PC,           /* program counter */
            N,        /* n condition bit */
            Z,        /* z condition bit */
            P;        /* p condition bit */
    int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3b ISIM Help-----------------------\n");
    printf("go               -  run program to completion         \n");
    printf("run n            -  execute program for n instructions\n");
    printf("mdump low high   -  dump memory from low to high      \n");
    printf("rdump            -  dump the register & bus values    \n");
    printf("?                -  display this help menu            \n");
    printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

    process_instruction();
    CURRENT_LATCHES = NEXT_LATCHES;
    INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
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
/* Purpose   : Simulate the LC-3b until HALTed                 */
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
void mdump(FILE *dumpsim_file, int start, int stop) {
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
        fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1],
                MEMORY[address][0]);
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
void rdump(FILE *dumpsim_file) {
    int k;

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
    printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
    fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
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
void get_command(FILE *dumpsim_file) {
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch (buffer[0]) {
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
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
    int i;

    for (i = 0; i < WORDS_IN_MEM; i++) {
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
    FILE *prog;
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

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) {
    int i;

    init_memory();
    for (i = 0; i < num_prog_files; i++) {
        load_program(program_filename);
        while (*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    FILE *dumpsim_file;

    /* Error Checking */
    if (argc < 2) {
        printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
               argv[0]);
        exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argc - 1);

    if ((dumpsim_file = fopen("dumpsim", "w")) == NULL) {
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

   MEMORY

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
#define EMPTY_VAL 0
#define LOG_LEVEL DEBUG
#define MAX_LOG_DEF_ARGS 4
#define MAX_NUM_OPCODES 29
#define nibble 4
#define debug EOL, __FUNCTION__, __LINE__, DEBUG
#define info EOL, __FUNCTION__, __LINE__, INFO
#define warn EOL, __FUNCTION__, __LINE__, WARN
#define error EOL, __FUNCTION__, __LINE__, ERROR
#define incrmntdPC(pc) (pc+2)
#define MEM(pc) Low16bits(((MEMORY[pc][1] << 2*nibble) + MEMORY[pc][0]))
#define topByte(pc) (MEMORY[pc][1] << 2*nibble)
#define btmByte(pc) (MEMORY[pc][0])
#define nibble1(pc) ((MEMORY[pc][0]) & 0x000F)
#define nibble2(pc) ((MEMORY[pc][0]) & 0x00F0)
#define nibble3(pc) ((MEMORY[pc][1] << 2*nibble) & 0x0F00)
#define nibble4(pc) ((MEMORY[pc][1] << 2*nibble) & 0xF000)
#define arg1(pc) (MEM(pc) & 0x0E00)
#define adjArgOne(val) val << 2*nibble+1
#define LSHF(val) val << 1
#define ZEXT8(pc) (MEM(pc) & 0x00FF)
#define bit(pc, b) ((MEM(pc) >> b) & 0x0001)
#define bitVal(val, b) ((val >> b) & 0x0001)
#define DR_NUM(pc) ((MEM(pc) & 0x0E00) >> 9)
#define SR1_NUM(pc) ((MEM(pc) & 0x01C0) >> 6)
#define SR2_NUM(pc) ((MEM(pc) & 0x0007))
#define DR(pc) CURRENT_LATCHES.REGS[DR_NUM(pc)]
#define SR1(pc) CURRENT_LATCHES.REGS[SR1_NUM(pc)]
#define SR2(pc) CURRENT_LATCHES.REGS[SR2_NUM(pc)]
#define imm5(pc) (MEM(pc) & 0x001F)
#define PCoffset9(pc) (MEM(pc) & 0x01FF)
#define PCoffset11(pc) (MEM(pc) & 0x07FF)
#define getPCoffset9(pc) (incrmntdPC(pc) + LSHF(signExt(PCoffset9(pc), 9)))
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

void processNop();

int16_t signExt(int16_t val, int sign_bit);

void setReg(int16_t reg, int16_t val);


/**-------------------------------- Function Definitions ------------------------------*/
void process_instruction() {

    loggingNoHeader(S, "Sign ", UI16, signExt(0x21EC, 4), info);

    /*  function: process_instruction
     *
     *    Process one instruction at a time
     *       -Fetch one instruction
     *       -Decode
     *       -Execute
     *       -Update NEXT_LATCHES
     */
    static int instruction_num = 0;
    int pc = CURRENT_LATCHES.PC / 2, next_pc = CURRENT_LATCHES.PC + 2;
    logging(S, "Simulating instruction #", Id, instruction_num++, info);
    loggingNoHeader(STAT, &CURRENT_LATCHES, N,
                    S, "current instruction: ", ADDR, pc, debug);

    enum OPCODES opcode = fetch(nibble4(pc), pc);


    switch (opcode) {
        case add:
            if (bit(pc, 5) == 0)
                setReg(DR_NUM(pc), SR1(pc) + SR2(pc));
            else
                setReg(DR_NUM(pc), SR1(pc) + signExt(imm5(pc), 5));

        case and:
            if (bit(pc, 5) == 0)
                setReg(DR_NUM(pc), SR1(pc) & SR2(pc));
            else
                setReg(DR_NUM(pc), SR1(pc) & signExt(imm5(pc), 5));

        case ldb:

        case ldw:
        case not:
        case lshf:
        case rshfl:
        case rshfa:
        case stb:
        case stw:
        case xor:
/*            result = reg = validateRegister(arg, true);
            if (argNum == 1) result = adjArgOne(result);
            else result = adjArgTwo(result);
            deprecatedLoggingFunc(deprecated_debug, 7, S, arg, S, " >> R", I, reg, S, ": ", I, result, S, " as arg #",
                                  I, argNum);*/
            break;
            /*if ((n AND N) OR (z AND Z) OR (p AND P))
            PC = PC + LSHF(SEXT(PCoffset9), 1);*/
        case brn:
            if (CURRENT_LATCHES.N == TRUE)
                next_pc = getPCoffset9(pc);
            break;
        case brp:
            if (CURRENT_LATCHES.P == TRUE)
                next_pc = getPCoffset9(pc);
            break;
        case brnp:
            if (CURRENT_LATCHES.N == TRUE && CURRENT_LATCHES.P == TRUE)
                next_pc = getPCoffset9(pc);
            break;
        case brz:
            if (CURRENT_LATCHES.Z == TRUE)
                next_pc = getPCoffset9(pc);
            break;
        case brnz:
            if (CURRENT_LATCHES.N == TRUE && CURRENT_LATCHES.Z == TRUE)
                next_pc = getPCoffset9(pc);
            break;
        case brzp:
            if (CURRENT_LATCHES.Z == TRUE && CURRENT_LATCHES.P == TRUE)
                next_pc = getPCoffset9(pc);
            break;
        case br:
        case brnzp:
            next_pc = getPCoffset9(pc);
            break;
        case rti:
/*            if (argNum == 1) writeToFile(outfile, "0x8000\n");*/
            break;
        case ret:
            next_pc = CURRENT_LATCHES.REGS[r7];
            break;
        case jmp:
            next_pc = SR1(pc);
            break;
        case jsrr:
        case jsr:
            if (bit(pc, 11) == 0)
                next_pc = SR1(pc);
            else
                next_pc = incrmntdPC(pc) + LSHF(signExt(PCoffset11(pc), 11));

            NEXT_LATCHES.REGS[r7] = incrmntdPC(pc);
            break;
        case lea:
/*            if (argNum == 1) {
                result = reg = validateRegister(arg, true);
                result = adjArgOne(result);
            }*/
            break;
        case trap:
        case halt:
            NEXT_LATCHES.REGS[r7] = pc;         /* R7 = PC; */
            next_pc = MEM(LSHF(ZEXT8(pc)));     /* PC = MEM[LSHF(ZEXT(trapvect8), 1)]; */
            break;
        case nop:
            processNop();
            break;
        case fill:
/*            if (argNum == 1) {
                uint16_t val = toNum(i->arg1);

                if (opcode == fill) result = outputFill(i->arg1);
            }
            deprecatedLoggingFunc(deprecated_debug, 2, S, "Fill current address with ", I, toNum(i->arg1));*/
            break;
    }


    NEXT_LATCHES.PC = next_pc;

    /*loggingMsgNoHeader(AGRN"PASS: "ANRM"simulation is done.", info);*/
}

void setReg(int16_t reg, int16_t val) {
    if (val > 0) NEXT_LATCHES.P = TRUE;
    else NEXT_LATCHES.P = FALSE;

    if (val == 0) NEXT_LATCHES.Z = TRUE;
    else NEXT_LATCHES.Z = FALSE;

    if (val < 0) NEXT_LATCHES.N = TRUE;
    else NEXT_LATCHES.N = FALSE;

    NEXT_LATCHES.REGS[reg] = val;
}

int16_t signExt(int16_t val, int sign_bit) {
    sign_bit--;
    loggingNoHeader(S, "Sign extenting: ", I, val, S, ", bit: ", Id, sign_bit, info);

    int16_t sign = bitVal(val, sign_bit);
    return sign == 1 ? ((0xFFFF << (sign_bit + 1)) | val)
                     : ((0xFFFF >> (16 - (sign_bit + 1))) & val);
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
            printf("%d %s(0x%X)%s", (int8_t) V, AWHT, (int8_t) V, ANRM);
            break;
        case I16:
        case INT_16:
            printf("%d %s(0x%X)%s", (int16_t) V, AWHT, (int16_t) V, ANRM);
            break;
        case I32:
        case INT_32:
            printf("%d %s(0x%X)%s", (int32_t) V, AWHT, (int32_t) V, ANRM);
            break;
        case I64:
        case INT_64:
            printf("%ld %s(0x%lX)%s", (int64_t) V, AWHT, (int64_t) V, ANRM);
            break;
        case UI8:
        case UINT_8:
            printf("%u %s(0x%X)%s", (uint8_t) V, AWHT, (uint8_t) V, ANRM);
            break;
        case UI16:
        case UINT_16:
            printf("%u %s(0x%X)%s", (uint16_t) V, AWHT, (uint16_t) V, ANRM);
            break;
        case UI32:
        case UINT_32:
            printf("%u %s(0x%X)%s", (uint32_t) V, AWHT, (uint32_t) V, ANRM);
            break;
        case UI64:
        case UINT_64:
            printf("%llu %s(0x%llX)%s", (uint64_t) V, AWHT, (uint64_t) V, ANRM);
            break;
        case I:
        case INT:
            printf("%d %s(0x%X)%s", (int) V, AWHT, (int) V, ANRM);
            break;
        case Id:
            printf("%d", (int) V);
            break;
        case Ix:
            printf("0x%X", (int) V);
            break;
        case L:
        case LONG:
            printf("%ld %s(0x%lX)%s", (long) V, AWHT, (long) V, ANRM);
            break;
        case LI:
        case LONG_INT:
            printf("%ld %s(0x%lX)%s", (long int) V, AWHT, (long int) V, ANRM);
            break;
        case UL:
        case ULONG:
            printf("%lu %s(0x%lX)%s", (unsigned long) V, AWHT, (unsigned long) V, ANRM);
            break;
        case ULI:
        case ULONG_INT:
            printf("%lu %s(0x%lX)%s", (unsigned long int) V, AWHT, (unsigned long int) V, ANRM);
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
            setColorPrint(level);
            printf("[%s]: func:'%s' → line:%d (%d)\n" ANRM, enumStrings[level], func, line, curLogNum);
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

        setColorPrint(level);
        printf(" (%d)\n" ANRM, curLogNum);
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

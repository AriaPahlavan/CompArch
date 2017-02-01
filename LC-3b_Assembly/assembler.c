#include <stdio.h>      /* standard input/output library */
#include <stdlib.h>     /* Standard C Library */
#include <string.h>     /* String operations library */
#include <ctype.h>      /* Library for useful character operations */
#include <limits.h>     /* Library for definitions of common variable type characteristics */
#include <stdarg.h>
#include <stdint.h>


/**----------------------------------- Definitions ------------------------------------*/
#define MAX_NUM_OPCODES 27
#define MAX_LINE_LENGTH 255
#define MAX_LABELS 255
#define EMPTY_VAL 0
#define LOG_LEVEL DEBUG
#define caller(arg) __FUNCTION__, __LINE__, arg
#define debug __FUNCTION__, __LINE__, DEBUG
#define info __FUNCTION__, __LINE__, INFO
#define warn __FUNCTION__, __LINE__, WARN
#define error __FUNCTION__, __LINE__, ERROR
#define ANRM  "\x1B[0m"
#define ARED  "\x1B[31m"
#define AGRN  "\x1B[32m"
#define AYEL  "\x1B[33m"
#define ABLU  "\x1B[34m"
#define AMAG  "\x1B[35m"
#define ACYN  "\x1B[36m"
#define AWHT  "\x1B[37m"


/**-------------------------------- Structures & Enums --------------------------------*/
/**
 * A symbol table element object
 *
 * @param address
 *          Address of the memory location where the label is located
 *
 * @param label
 *          Symbolic label for a specific address
 */
typedef struct SYMBOL_TABLE_ELEMENT {
    int address;
    char *label;
} SymbolTableElement;
typedef struct INSTRUCTION {
    char *label;
    char *opcode;
    char *arg1;
    char *arg2;
    char *arg3;
    char *arg4;

    int8_t hasLabel;
    int8_t isPseudoOpe;
    int8_t isValidOpcode;
} Instruction;

enum {
    DONE, OK, EMPTY_LINE
};
enum DATA_TYPE {
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
    TAB
};
enum LOG_LEVELS {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    NONE
};


/**--------------------------------- Global Variables ---------------------------------*/
SymbolTableElement symbolTable[MAX_LABELS];
static int logNum = 0;

const static char *enumStrings[] = {"DEBUG", "INFO", "WARN", "ERROR"};
const static char *pseudoOps[] = {".ORIG", ".END", ".FILL"};
/*const static char *opcodes[] = {"ADD", "AND", "BR", "BRn", "BRp", "BRnp", "BRz", "BRnz", "BRzp",
                                "BRnzp", "JMP", "JSR", "JSRR", "LDB", "LDW", "LEA", "NOT", "RET",
                                "RTI", "LSHF", "RSHFL", "RSHFA", "STB", "STW", "TRAP", "XOR", "HALT"};*/

const static char *lowerOpcodes[] = {"add", "and", "br", "brn", "brp", "brnp", "brz", "brnz", "brzp",
                                     "brnzp", "jmp", "jsr", "jsrr", "jdb", "ldw", "lea", "not", "ret",
                                     "rit", "lshf", "rshfl", "rshfa", "stb", "stw", "trap", "xor", "halt"};


/**-------------------------------- Function Declarations -----------------------------*/
int isOpcode(char *potential_opcode);

void parsCommandLine(int argc, char *argv[]);

void testIsOpcode();

void testLimits();

void loggingMsg(const char *func, int line, enum LOG_LEVELS lvl, const char *msg);

void logging(const char *func, int line, enum LOG_LEVELS lvl, int num, ...);

void print(int num, ...);

void println(int num, ...);

void printList(int num, va_list valist);

void outputDouble(double value);

void output(void *V, enum DATA_TYPE Type);

void colorPrint(enum LOG_LEVELS lvl, const char *txt);

int getLogNum();

void testOpenCloseFile(int argc, char *argv[], FILE *infile, FILE *outfile);

void closeFiles(FILE *file);

FILE *openFile(char *filename, char *permission);

void testToNum();

void writeToFile(FILE *file, const char *text);

void testWriteToFile(const FILE *infile);

Instruction *newInstruction();

void freeInstruction(Instruction *instruction);

int parseFile(const FILE *infile, Instruction *i);

int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode,
                 char **pArg1, char **pArg2, char **pArg3, char **pArg4);


/**-------------------------------- Function Definitions ------------------------------*/
int main(int argc, char *argv[]) {
    loggingMsg(info, "Initializing...");

    /* open input/output files*/
    FILE *infile = openFile(argv[1], "r");
    FILE *outfile = openFile(argv[2], "w");

    Instruction i;

    parseFile(infile, &i);

    /* close input/output files*/
    closeFiles(infile);
    closeFiles(outfile);

    return 0;
}

int parseFile(const FILE *infile, Instruction *i) {
    char lLine[MAX_LINE_LENGTH + 1];
    int lRet, instructionCounter = 0;

    do {
        lRet = readAndParse(infile, lLine, &i->label,
                            &i->opcode, &i->arg1, &i->arg2, &i->arg3, &i->arg4);
        if (lRet != DONE && lRet != EMPTY_LINE) {
            instructionCounter++;

            logging(debug, 12,
                    S, "Label: ", S, i->label, N,
                    S, "Opcode: ", S, i->opcode, N,
                    S, "Arg1: ", S, i->arg1, N,
                    S, "Arg2: ", S, i->arg2, N,
                    S, "Arg3: ", S, i->arg3, N,
                    S, "Arg4: ", S, i->arg4
            );

            //TODO If had label, add to symbol table


        }
    } while (lRet != DONE);

    logging(debug, 2, S, "Number of instruction in the file=", I, instructionCounter);

    return instructionCounter;
}

void writeToFile(FILE *file, const char *text) {
    loggingMsg(debug, "Writing to file...");
    int lInstr;
    fprintf(file, text, lInstr);

    logging(debug, 2,
            S, "Success: wrote to file: ", S, text
    );
}

FILE *openFile(char *filename, char *permission) {
    logging(debug, 2,
            S, "Openning file: ", S, filename);

    /* open the source file */
    FILE *file = fopen(filename, permission);

    if (!file) {
        logging(warn, 2,
                S, ARED "Failed" ANRM " to open file ", S, filename
        );
        exit(4);

    } else {
        logging(debug, 2,
                S, "Successfully opened file: ",
                S, filename
        );
    }

    return file;
}

void closeFiles(FILE *file) {
    logging(debug, 2,
            S, "Closing file descriptor: ",
            I, file
    );

    if (fclose(file) != 0) {
        logging(error, 2,
                S, "Error wile attempting to close file descriptor: ",
                S, file->_fileno
        );
    } else {
        loggingMsg(debug, "Successfully closed file");
    }
}

int isOpcode(char *potential_opcode) {
    int result = -1;
    int i;

    for (i = 0; i < MAX_NUM_OPCODES; ++i) {
        int curCompResult = strcmp(potential_opcode, lowerOpcodes[i]);

        if (curCompResult == 0) {
            result = 1;
            break;
        }
    }

    return result;
}

int toNum(char *pStr) {
    char *t_ptr;
    char *orig_pStr;
    int t_length, k;
    int lNum, lNeg = 0;
    long int lNumLong;

    orig_pStr = pStr;
    if (*pStr == '#')                /* decimal */
    {
        pStr++;
        if (*pStr == '-')                /* dec is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for (k = 0; k < t_length; k++) {
            if (!isdigit(*t_ptr)) {
                logging(error, 2,
                        S, "invalid decimal operand, ", S, orig_pStr
                );
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg)
            lNum = -lNum;

        return lNum;
    } else if (*pStr == 'x')    /* hex     */
    {
        pStr++;
        if (*pStr == '-')                /* hex is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for (k = 0; k < t_length; k++) {
            if (!isxdigit(*t_ptr)) {
                logging(error, 2,
                        S, "invalid hex operand, ", S, orig_pStr
                );
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
        lNum = (lNumLong > INT_MAX) ? INT_MAX : lNumLong;
        if (lNeg)
            lNum = -lNum;
        return lNum;
    } else {
        logging(error, 2,
                S, "Invalid operand, ", S, orig_pStr
        );
        exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}

/* Note: MAX_LINE_LENGTH, OK, EMPTY_LINE, and DONE are defined values */
int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode,
                 char **pArg1, char **pArg2, char **pArg3, char **pArg4) {
    char *lRet, *lPtr;
    int i;
    if (!fgets(pLine, MAX_LINE_LENGTH, pInfile))
        return (DONE);
    for (i = 0; i < strlen(pLine); i++)
        pLine[i] = tolower(pLine[i]);

    /* convert entire line to lowercase */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

    /* ignore the comments */
    lPtr = pLine;

    while (*lPtr != ';' && *lPtr != '\0' &&
           *lPtr != '\n')
        lPtr++;

    *lPtr = '\0';
    if (!(lPtr = strtok(pLine, "\t\n ,")))
        return (EMPTY_LINE);

    if (isOpcode(lPtr) == -1 && lPtr[0] != '.') /* found a label */
    {
        *pLabel = lPtr;
        if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);
    }

    *pOpcode = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);

    *pArg1 = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);

    *pArg2 = lPtr;
    if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);

    *pArg3 = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);

    *pArg4 = lPtr;

    return (OK);
}

void parsCommandLine(int argc, char **argv) {
    char *prgName = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;

    prgName = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];

    logging(debug, 6,
            S, "program name = ", S, prgName, N,
            S, "input file name = ", S, iFileName, N,
            S, "output file name = ", S, oFileName
    );
}

/**------------------------------------- Tests ---------------------------------------*/
void testOpenCloseFile(int argc, char *argv[], FILE *infile, FILE *outfile) {
    /* open the source file */
    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");

    if (!infile) {
        logging(warn, 2,
                S, "Error: Cannot open file ", S, argv[1]);
        exit(4);
    }
    if (!outfile) {
        logging(warn, 2,
                S, "Error: Cannot open file ", S, argv[2]);
        exit(4);
    } else {
        logging(debug, 1,
                S, "Successfully opened both files!"
        );
    }

    /* Do stuff with files */

    closeFiles(infile);
    closeFiles(outfile);
}

void testWriteToFile(const FILE *infile) {
    writeToFile(infile, "\t.ORIG x3000\n");
    writeToFile(infile, "A\tAND R0, R0, #0\n");
    writeToFile(infile, "\tLEA R0, A\n");
    writeToFile(infile, "\tHALT\n");
    writeToFile(infile, "\t.END\n");
}

void testIsOpcode() {
    char *myOpcode = "AND";

    logging(debug, 2,
            S, "The reselt is: ",
            I, isOpcode(myOpcode)
    );
}

void testToNum() {
    /*=====================================Test 1========================================*/
    loggingMsg(debug, "Converting string to num for valid value 'x3000'");

    int result = toNum("X3000");

    if (result != 0x3000)
        logging(error, 2,
                S, "Failed: expected result=0x3000 but actual result=", I, result
        );
    else
        logging(debug, 2,
                S, "Success: expected result=0x3000 and actual result=", I, result
        );

    /*=====================================Test 2========================================*/
    loggingMsg(debug, "Converting string to num for valid value '#45'");

    result = toNum("#-45");

    if (result != -45)
        logging(error, 2,
                S, "Failed: expected result=-45 but actual result=", I, result
        );
    else
        logging(debug, 2,
                S, "Success: expected result=-45 and actual result=", I, result
        );

    /*=====================================Test 3========================================*/
    loggingMsg(debug, "Converting string to num for invalid value '0'. Will exit(4)!");
    result = toNum("0");
}

void testLimits() {
    logging(debug,
            26, /*(51-36)*2 = 31; subtracting line numbers, to get total number of outputings*/
            S, "The number of bits in a byte = ", I8, CHAR_BIT, N,
            S, "The minimum value of SIGNED CHAR = ", I8, SCHAR_MIN, N,
            S, "The maximum value of SIGNED CHAR = ", I8, SCHAR_MAX, N,
            S, "The maximum value of UNSIGNED CHAR = ", UI8, UCHAR_MAX, N,
            S, "The minimum value of SHORT INT = ", I16, SHRT_MIN, N,
            S, "The maximum value of SHORT INT = ", I16, SHRT_MAX, N,
            S, "The minimum value of INT = ", I, INT_MIN, N,
            S, "The maximum value of INT = ", I, INT_MAX, N,
            S, "The minimum value of CHAR = ", I8, CHAR_MIN, N,
            S, "The maximum value of CHAR = ", I8, CHAR_MAX, N,
            S, "The minimum value of LONG = ", L, LONG_MIN, N,
            S, "The maximum value of LONG = ", L, LONG_MAX, N,
            S, "The maximum value of UNSIGNED LONG = ", UL, ULONG_MAX
    );
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

void loggingMsg(const char *func, int line, enum LOG_LEVELS lvl, const char *msg) {
    if (lvl >= LOG_LEVEL) {
        int curLogNum = getLogNum();
        setColorPrint(lvl);
        printf("[%s]: func:'%s' → line:%d (%d)%s\n%s", enumStrings[lvl], func, line, curLogNum, ANRM, msg);
        setColorPrint(lvl);
        printf(" (%d)%s\n", curLogNum, ANRM);
    }
}

void logging(const char *func, int line, enum LOG_LEVELS lvl, int num, ...) {
    va_list valist;

    /* initialize valist for num number of arguments */
    va_start(valist, num);

    if (lvl >= LOG_LEVEL) {
        int curLogNum = getLogNum();
        setColorPrint(lvl);
        printf("[%s]: func:'%s' → line:%d (%d)\n" ANRM, enumStrings[lvl], func, line, curLogNum);
        printList(num, valist);
        setColorPrint(lvl);
        printf(" (%d)\n" ANRM, curLogNum);
    }

    /* clean memory reserved for valist */
    va_end(valist);

}

int getLogNum() {
    int curNum = logNum;
    logNum += 1;
    return curNum;
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

Instruction *newInstruction() {
    Instruction *instruction = malloc(sizeof(Instruction *));
}

void freeInstruction(Instruction *instruction) {
    free(instruction);
}
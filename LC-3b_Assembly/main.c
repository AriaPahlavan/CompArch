#include <stdio.h>      /* standard input/output library */
#include <stdlib.h>     /* Standard C Library */
#include <string.h>     /* String operations library */
#include <ctype.h>      /* Library for useful character operations */
#include <limits.h>     /* Library for definitions of common variable type characteristics */
#include <stdarg.h>

/**----------------------------------- Definitions ------------------------------------*/
#define MY_LOG_LEVEL DEBUG
#define MAX_NUM_OPCODES 20
#define MAX_LINE_LENGTH 255
#define EMPTY_VAL 0
#define LOG_LEVEL DEBUG
#define ANRM  "\x1B[0m"
#define ARED  "\x1B[31m"
#define AGRN  "\x1B[32m"
#define AYEL  "\x1B[33m"
#define ABLU  "\x1B[34m"

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
SymbolTableElement symbol_table[1000];

const static char *enumStrings[] = {"DEBUG", "INFO", "WARN", "ERROR"};

const static char *opcodes[] = {"ADD", "AND", "BR", "JMP", "JSR", "JSRR", "LDB", "LDW",
                                "LEA", "NOT", "RET", "RTI", "LSHF", "RSHFL", "RSHFA",
                                "STB", "STW", "TRAP", "XOR", "HALT"};


/**-------------------------------- Function Declarations -----------------------------*/
int isOpcode(char *potential_opcode);

void parsCommandLine(int argc, char *argv[]);

void testIsOpcode();

void testLimits();

void loggingMsg(enum LOG_LEVELS lvl, char *msg);

void logging(enum LOG_LEVELS lvl, int num, ...);

void print(int num, ...);

void println(int num, ...);

void printList(int num, va_list valist);

void printLog(int num, va_list valist);

void printLogLabel(enum LOG_LEVELS lvl);

void outputDouble(double value);

void output(void *V, enum DATA_TYPE Type);

void colorPrint(enum LOG_LEVELS lvl, const char *txt);

/**-------------------------------- Function Definitions ------------------------------*/
int main(int argc, char *argv[]) {
    testIsOpcode();                     /* Testing isOpcode func */
    parsCommandLine(argc, argv);        /* Testing command line parsing */


    return 0;
}

int isOpcode(char *potential_opcode) {
    int result = -1;
    int i;

    for (i = 0; i < MAX_NUM_OPCODES; ++i) {
        int curCompResult = strcmp(potential_opcode, opcodes[i]);

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
                printf("Error: invalid decimal operand, %s\n", orig_pStr);
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
                printf("Error: invalid hex operand, %s\n", orig_pStr);
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
        printf("Error: invalid operand, %s\n", orig_pStr);
        exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}

void testIsOpcode() {
    char *myOpcode = "AND";

    logging(DEBUG,
            2,
            S, "The reselt is: ",
            I, isOpcode(myOpcode)
    );
}

void parsCommandLine(int argc, char **argv) {
    char *prgName = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;

    prgName = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];

    logging(INFO,
            6,
            S, "program name = ", S, prgName, N,
            S, "input file name = ", S, iFileName, N,
            S, "output file name = ", S, oFileName
    );
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

void testLimits() {
    logging(MY_LOG_LEVEL,
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

void printLog(int num, va_list valist) {
    int i;
    num *= 2;

    enum DATA_TYPE T;
    void *V;

    for (i = 0; i < num;) {
        T = va_arg(valist, enum DATA_TYPE);

        if (T == N || T == ENDL) {
            V = EMPTY_VAL;
            T == ENDL ? i += 2 : 0;
            output(V, T);
            printf("\t\t ");
        } else {
            V = va_arg(valist, void *);
            i += 2;
            output(V, T);
        }
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
            printf("%d", (int8_t) V);
            break;
        case I16:
        case INT_16:
            printf("%d", (int16_t) V);
            break;
        case I32:
        case INT_32:
            printf("%d", (int32_t) V);
            break;
        case I64:
        case INT_64:
            printf("%d", (int64_t) V);
            break;
        case UI8:
        case UINT_8:
            printf("%u", (__uint8_t) V);
            break;
        case UI16:
        case UINT_16:
            printf("%u", (__uint16_t) V);
            break;
        case UI32:
        case UINT_32:
            printf("%u", (__uint32_t) V);
            break;
        case UI64:
        case UINT_64:
            printf("%u", (__uint64_t) V);
            break;
        case I:
        case INT:
            printf("%d", (int) V);
            break;
        case L:
        case LONG:
            printf("%ld", (long) V);
            break;
        case LI:
        case LONG_INT:
            printf("%ld", (long int) V);
            break;
        case UL:
        case ULONG:
            printf("%lu", (unsigned long) V);
            break;
        case ULI:
        case ULONG_INT:
            printf("%lu", (unsigned long int) V);
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

void loggingMsg(enum LOG_LEVELS lvl, char *msg) {
    printLogLabel(lvl);
    printf("%s", msg);
    colorPrint(lvl, "\n---------------------------\n");
}

void logging(enum LOG_LEVELS lvl, int num, ...) {
    va_list valist;

    /* initialize valist for num number of arguments */
    va_start(valist, num);

    if (lvl >= LOG_LEVEL) {
        printLogLabel(lvl);
        if (lvl == INFO || lvl == WARN) printf(" ");
        printLog(num, valist);
        colorPrint(lvl, "\n---------------------------\n");
    }

    /* clean memory reserved for valist */
    va_end(valist);

}

void printLogLabel(enum LOG_LEVELS lvl) {
    printf("[");
    colorPrint(lvl, enumStrings[lvl]);
    printf("]: ");
}

void colorPrint(enum LOG_LEVELS lvl, const char *txt) {
    switch (lvl) {
        case DEBUG:
            printf(AGRN "%s%s", txt, ANRM);
            break;
        case INFO:
            printf(ABLU "%s%s", txt, ANRM);
            break;
        case WARN:
            printf(AYEL "%s%s", txt, ANRM);
            break;
        case ERROR:
            printf(ARED "%s%s", txt, ANRM);
            break;
    }
}

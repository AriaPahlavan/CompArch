/**
 * Created by aria on 1/28/17.
 */

#ifndef LC_3B_ASSEMBLY_PRINTTOOLS_H
#define LC_3B_ASSEMBLY_PRINTTOOLS_H

#include <stdbool.h>


enum DATA_TYPE{
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

enum LOG_LEVELS{
    DEBUG,
    INFO,
    WARN,
    ERROR,
    NONE
};

const static char * enumStrings[] = {"DEBUG", "INFO", "WARN", "ERROR"};

void loggingMsg(enum LOG_LEVELS lvl, char * msg);

void logging(enum LOG_LEVELS lvl, int num, ...);

void printList(int num, va_list valist);

void printLog(int num, va_list valist);

void print(int num, ...);

void println(int num, ...);

void outputDouble(double value);

void output(void *V, enum DATA_TYPE Type);


#endif /*LC_3B_ASSEMBLY_PRINTTOOLS_H*/

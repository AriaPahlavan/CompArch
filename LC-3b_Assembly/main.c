#include <stdio.h>      /* standard input/output library */
#include <stdlib.h>     /* Standard C Library */
#include <string.h>     /* String operations library */
#include <ctype.h>      /* Library for useful character operations */
#include <limits.h>     /* Library for definitions of common variable type characteristics */
#include "loglib.h"
#include "symbolTable.h"


#define MY_LOG_LEVEL DEBUG



int main() {
    return 0;
}

void limits() {

    /*printf("The number of bits in a byte %d\n", CHAR_BIT);

    printf("The minimum value of SIGNED CHAR = %d\n", SCHAR_MIN);
    printf("The maximum value of SIGNED CHAR = %d\n", SCHAR_MAX);
    printf("The maximum value of UNSIGNED CHAR = %d\n", UCHAR_MAX);

    printf("The minimum value of SHORT INT = %d\n", SHRT_MIN);
    printf("The maximum value of SHORT INT = %d\n", SHRT_MAX);

    printf("The minimum value of INT = %d\n", INT_MIN);
    printf("The maximum value of INT = %d\n", INT_MAX);

    printf("The minimum value of CHAR = %d\n", CHAR_MIN);
    printf("The maximum value of CHAR = %d\n", CHAR_MAX);

    printf("The minimum value of LONG = %ld\n", LONG_MIN);
    printf("The maximum value of LONG = %ld\n", LONG_MAX);*/

    logging(MY_LOG_LEVEL,
            26, //(51-36)*2 = 31; subtracting line numbers, to get total number of outputings
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
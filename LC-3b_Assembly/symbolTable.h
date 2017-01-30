/**
 * Created by aria on 1/29/17.
 */


#ifndef LC_3B_ASSEMBLY_SYMBOLTABLE_H
#define LC_3B_ASSEMBLY_SYMBOLTABLE_H

#include <stdio.h>
#include <stdlib.h>

/**
 * A symbol table element object
 *
 * @param address
 *          Address of the memory location where the label is located
 *
 * @param label
 *          Symbolic label for a specific address
 */
typedef struct SYMBOL_TABLE_ELEMENT{
    int16_t address;
    char * label;
} SymbolTableElement;

#endif /*LC_3B_ASSEMBLY_SYMBOLTABLE_H*/

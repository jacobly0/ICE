#ifndef PARSE_H
#define PARSE_H

#include <fileioc.h>

#define TYPE_NUMBER          0
#define TYPE_VARIABLE        1
#define TYPE_FUNCTION_RETURN 2
#define TYPE_C_FUNCTION      2
#define TYPE_CHAIN_ANS       3
#define TYPE_CHAIN_PUSH      4
// Don't change up to here!

#define TYPE_LIST            5
#define TYPE_OS_LIST         6
#define TYPE_STRING          7
#define TYPE_OS_STRING       8

#define TYPE_C_START         252
#define TYPE_ARG_DELIMITER   253
#define TYPE_OPERATOR        254
#define TYPE_FUNCTION        255

#define getc() ti_GetC(currentProgram)

uint8_t parseProgram(ti_var_t);
uint8_t parsePostFixFromIndexToIndex(uint24_t startIndex, uint24_t endIndex);
void optimizeZeroCarryFlagOutput(void);
void skipLine(ti_var_t);

typedef struct {
    uint8_t type;
    uint24_t operand;
} element_t;

#endif

#ifndef PARSE_H
#define PARSE_H

#define TYPE_NUMBER          0
#define TYPE_VARIABLE        1
#define TYPE_FUNCTION_RETURN 2
#define TYPE_CHAIN_ANS       3
#define TYPE_CHAIN_PUSH      4
// Don't change up to here!

#define TYPE_LIST            5
#define TYPE_OS_LIST         6
#define TYPE_STRING          7
#define TYPE_OS_STRING       8

#define TYPE_C_FUNCTION      13
#define TYPE_OPERATOR        14
#define TYPE_FUNCTION        15

#define getc() ti_GetC(currentProgram)

uint8_t parseProgram(ti_var_t);

typedef struct {
    uint8_t type;
    uint24_t operand;
} element_t;

#endif


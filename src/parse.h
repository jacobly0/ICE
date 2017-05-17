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

#define RET_A         (1<<7)
#define RET_HL        (1<<5)
#define RET_NONE      (0)
#define UN            (1<<6)
#define ARG_NORM      (0)
#define SMALL_1       (1<<7)
#define SMALL_2       (1<<6)
#define SMALL_3       (1<<5)
#define SMALL_4       (1<<4)
#define SMALL_5       (1<<3)
#define SMALL_12      (SMALL_1 | SMALL_2)
#define SMALL_123     (SMALL_1 | SMALL_2 | SMALL_3)
#define SMALL_13      (SMALL_1 | SMALL_3)
#define SMALL_23      (SMALL_2 | SMALL_3)
#define SMALL_14      (SMALL_1 | SMALL_4)
#define SMALL_45      (SMALL_4 | SMALL_5)

#define getc() ti_GetC(currentProgram)

uint8_t parseProgram(ti_var_t);

typedef struct {
    uint8_t type;
    uint24_t operand;
} element_t;

#endif

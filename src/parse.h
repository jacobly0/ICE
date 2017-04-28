#ifndef PARSE_H
#define PARSE_H

#define TYPE_NUMBER          0
#define TYPE_VARIABLE        1
#define TYPE_CHAIN_PUSH      2
#define TYPE_CHAIN_ANS       3
#define TYPE_FUNCTION_RETURN 4
#define TYPE_LIST            5
#define TYPE_OS_LIST         6
#define TYPE_STRING          7
#define TYPE_OS_STRING       8
#define TYPE_OPERATOR        9
#define TYPE_FUNCTION        10

uint8_t parseProgram(void);

typedef struct {
    uint8_t type;
    uint24_t operand;
} element_t;

#endif


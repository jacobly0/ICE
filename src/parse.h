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

uint8_t parseProgram(void);

enum { ERROR, VALID };

#endif


#ifndef PARSE_H
#define PARSE_H

// In case I want to add more types, be sure that TYPE_STRING will be the last one, otherwise operator.c will mess up
#define TYPE_NUMBER          0
#define TYPE_VARIABLE        1
#define TYPE_CHAIN_ANS       2
#define TYPE_CHAIN_PUSH      3
#define TYPE_STRING          4

#define TYPE_C_START         124
#define TYPE_ARG_DELIMITER   125
#define TYPE_OPERATOR        126
#define TYPE_FUNCTION        127

#define TYPE_MASK_U8         0
#define TYPE_MASK_U16        1
#define TYPE_MASK_U24        2

uint8_t JumpForward(uint8_t*, uint8_t*, uint24_t, uint8_t, uint8_t);
uint8_t JumpBackwards(uint8_t*, uint8_t);
void optimizeZeroCarryFlagOutput(void);
void skipLine(void);
void insertGotoLabel(void);

uint8_t parsePostFixFromIndexToIndex(uint24_t, uint24_t);
uint8_t functionRepeat(int);
uint8_t parseProgram(void);

typedef struct {
    uint8_t  isString;
    uint8_t  type;
    uint8_t  mask;
    uint24_t operand;
} element_t;

#endif

#ifndef PARSE_H
#define PARSE_H

// In case I want to add more types, be sure that TYPE_STRING will be the last one, otherwise operator.c will mess up
enum {
    TYPE_NUMBER,
    TYPE_VARIABLE,
    TYPE_CHAIN_ANS,
    TYPE_CHAIN_PUSH,
    TYPE_STRING
};

enum {
    TYPE_C_START = 124,
    TYPE_ARG_DELIMITER,
    TYPE_OPERATOR,
    TYPE_FUNCTION
};

enum {
    TYPE_MASK_U8,
    TYPE_MASK_U16,
    TYPE_MASK_U24
};

uint8_t JumpForward(uint8_t*, uint8_t*, uint24_t, uint8_t, uint8_t);
uint8_t JumpBackwards(uint8_t*, uint8_t);
void optimizeZeroCarryFlagOutput(void);
void skipLine(void);
void insertGotoLabel(void);

uint8_t parsePostFixFromIndexToIndex(uint24_t, uint24_t);
uint8_t functionRepeat(int);
uint8_t parseProgram(void);
uint8_t parseProgramUntilEnd(void);

typedef struct {
    uint8_t  isString;
    uint8_t  type;
    uint8_t  mask;
    uint24_t operand;
} element_t;

#endif

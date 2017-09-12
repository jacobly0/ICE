#ifndef PARSE_H
#define PARSE_H

#define TYPE_NUMBER          0
#define TYPE_VARIABLE        1
#define TYPE_FUNCTION_RETURN 2
#define TYPE_C_FUNCTION      2
#define TYPE_CHAIN_ANS       3
#define TYPE_CHAIN_PUSH      4
#define TYPE_STRING          5
#define TYPE_OS_STRING       6
// ---------------------------
#define TYPE_LIST            7
#define TYPE_OS_LIST         8

#define TYPE_INDEX_START     250
#define TYPE_INDEX_END       251
#define TYPE_C_START         252
#define TYPE_ARG_DELIMITER   253
#define TYPE_OPERATOR        254
#define TYPE_FUNCTION        255

#define TYPE_MASK_U8         0
#define TYPE_MASK_U16        1
#define TYPE_MASK_U24        2

void UpdatePointersToData(uint24_t);
uint8_t JumpForward(uint8_t*, uint8_t*, uint24_t);
uint8_t JumpBackwards(uint8_t*, uint8_t);
void optimizeZeroCarryFlagOutput(void);
void skipLine(void);
void insertGotoLabel(void);

uint8_t parsePostFixFromIndexToIndex(uint24_t, uint24_t);
uint8_t functionRepeat(int);
uint8_t parseProgram(void);

typedef struct {
    uint8_t  type;
    uint8_t  mask;
    uint24_t operand;
} element_t;

#endif

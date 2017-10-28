#ifndef OPERATOR_H
#define OPERATOR_H

#define OUTPUT_IN_HL    0
#define OUTPUT_IN_DE    1
#define OUTPUT_IN_BC    2
#define OUTPUT_IN_HL_DE 3
#define OUTPUT_IN_A     4

#define NO_PUSH   false
#define NEED_PUSH true

#define TempString1 0
#define TempString2 1

uint8_t getIndexOfOperator(uint8_t);
uint24_t executeOperator(uint24_t, uint24_t, uint8_t);
void LD_HL_STRING(uint24_t);
uint8_t parseOperator(element_t*, element_t*, element_t*, element_t*);

void StoToChainAns(void);
void EQInsert(void);
void GEInsert(void);
void AddStringString(void);
void StoStringString(void);
void StoStringVariable(void);
void StoStringChainAns(void);

void MultWithNumber(uint24_t, uint8_t*, bool);

extern const char operators[];
extern const uint8_t operatorPrecedence[];
extern const uint8_t operatorPrecedence2[];

#endif

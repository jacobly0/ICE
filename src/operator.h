#ifndef OPERATOR_H
#define OPERATOR_H

#include <stdint.h>
#include <stdbool.h>

#define OUTPUT_IN_BC 0
#define OUTPUT_IN_DE 1
#define OUTPUT_IN_HL 2

#define NO_PUSH   false
#define NEED_PUSH true

#define TempString1 pixelShadow + 64000
#define TempString2 pixelShadow + 65000

uint8_t getIndexOfOperator(uint8_t operator);
uint24_t executeOperator(uint24_t operand1, uint24_t operand2, uint8_t operator);
void LD_HL_NUMBER(uint24_t number);
uint8_t parseOperator(element_t *outputPrevPrev, element_t *outputPrev, element_t *outputCurr);

void insertFunctionReturn(uint24_t function, uint8_t outputRegister, bool needPush);
void EQInsert();
void GEInsert();
void AddStringString(void);
void StoStringString(void);

void MultWithNumber(uint24_t number, uint8_t *programPtr);

extern const char operators[];
extern const uint8_t operatorPrecedence[];
extern const uint8_t operatorPrecedence2[];

#endif

#ifndef OPERATOR_H
#define OPERATOR_H

uint8_t getIndexOfOperator(uint8_t operator);
uint24_t executeOperator(uint24_t operand1, uint24_t operand2, uint8_t operator);
void parseOperator(element_t *outputPrevPrev, element_t *outputPrev, element_t *outputCurr);

#endif
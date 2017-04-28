#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>

#include <fileioc.h>

#include "parse.h"
#include "main.h"
#include "errors.h"
#include "output.h"
#include "operator.h"

extern void (*operatorFunctions[8])(element_t *, element_t *);

uint8_t getIndexOfOperator(uint8_t operator) {
    const char operators[] = {tStore, tAnd, tXor, tOr, tEQ, tLT, tGT, tLE, tGE, tNE, tMul, tDiv, tAdd, tSub};
    char *index;
    if (index = strchr(operators, operator)) {
        return index - operators + 1;
    }
    return 0;
}

uint24_t executeOperator(uint24_t operand1, uint24_t operand2, uint8_t operator) {
    switch ((uint8_t)operator) {
        case tAdd:
            return operand1 + operand2;
        case tSub:
            return operand1 - operand2;
        case tMul:
            return operand1 * operand2;
        case tDiv:
            return operand1 / operand2;
        case tNE:
            return operand1 != operand2;
        case tGE:
            return operand1 >= operand2;
        case tLE:
            return operand1 <= operand2;
        case tGT:
            return operand1 > operand2;
        case tLT:
            return operand1 < operand2;
        case tEQ:
            return operand1 == operand2;
        case tOr:
            return operand1 || operand2;
        case tXor:
            return !operand1 != !operand2;
        case tAnd:
            return operand1 && operand2;
        default:
            return operand1;
    }
}

void parseOperator(element_t *outputPrevPrev, element_t *outputPrev, element_t *outputCurr) {
}

static void AddNumberVariable(element_t *entry1, element_t *entry2) {
	AddVariableNumber(entry2, entry1);
}

static void AddNumberChainAns(element_t *entry1, element_t *entry2) {
	AddChainAnsNumber(entry2, entry1);
}

static void AddNumberFunction(element_t *entry1, element_t *entry2) {
	AddFunctionNumber(entry2, entry1);
}

static void AddVariableNumber(element_t *entry1, element_t *entry2) {
	LD_HL_IND_IX_OFF(entry1->operand);
	AddChainAnsNumber(entry1, entry2);
}

static void AddVariableChainAns(element_t *entry1, element_t *entry2) {
	AddChainAnsVariable(entry2, entry1);
}

static void AddVariableFunction(element_t *entry1, element_t *entry2) {
	AddFunctionVariable(entry2, entry1);
}


static void (*operatorFunctions[8])(element_t, element_t) = {
	AddNumberNumber,
	AddNumberVariable,
	AddNumberChainAns,
	AddNumberFunction,
	AddVariableNumber,
	AddVariableVariable,
	AddVariableChainAns,
	AddVariableFunction,
}
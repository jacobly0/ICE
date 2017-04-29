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
#include <graphx.h>

#include "parse.h"
#include "main.h"
#include "errors.h"
#include "output.h"
#include "operator.h"

extern uint8_t (*operatorFunctions[20])(element_t*, element_t*);
const char operators[] = {tStore, tAnd, tXor, tOr, tEQ, tLT, tGT, tLE, tGE, tNE, tMul, tDiv, tAdd, tSub};
const uint8_t operatorPrecedence[] = {0, 1, 2, 2, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4};

uint8_t getIndexOfOperator(uint8_t operator) {
    char *index;
    if (index = strchr(operators, operator)) {
        return index - operators + 1;
    }
    return 0;
}

uint24_t executeOperator(uint24_t operand1, uint24_t operand2, uint8_t operator) {
    switch (operator) {
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

uint8_t parseOperator(element_t *outputPrevPrev, element_t *outputPrev, element_t *outputCurr) {
    if (outputPrev->operand <= TYPE_CHAIN_ANS && outputPrevPrev->operand <= TYPE_CHAIN_PUSH) {
        return (*operatorFunctions[(outputCurr->operand * 20) + (outputPrevPrev->operand * 5) + (outputPrev->operand)])(outputPrevPrev, outputPrev);
    }
    return E_SYNTAX;
}

void insertFunctionReturn(uint8_t function, uint8_t outputRegister, uint8_t needPush) {
}








static uint8_t AddChainAnsNumber(element_t *entry1, element_t *entry2) {
    uint24_t number = entry2->operand;
    if (number < 5) {
        uint8_t a;
        for (a = 0; a < (uint8_t)number; a++) {
            INC_HL();
        }
    } else {
        LD_DE_IMM(number);
        ADD_HL_DE();
    }
    return VALID;
}

static uint8_t AddVariableNumber(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    return AddChainAnsNumber(entry1, entry2);
}

static uint8_t AddFunctionNumber(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    return AddChainAnsNumber(entry1, entry2);
}

static uint8_t AddChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_DE_IND_IX_OFF(entry2->operand);
    ADD_HL_DE();
    return VALID;
}

static uint8_t AddFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    return AddChainAnsVariable(entry1, entry2);
}

static uint8_t AddNumberVariable(element_t *entry1, element_t *entry2) {
    return AddVariableNumber(entry2, entry1);
}

static uint8_t AddNumberFunction(element_t *entry1, element_t *entry2) {
    return AddFunctionNumber(entry2, entry1);
}

static uint8_t AddNumberChainAns(element_t *entry1, element_t *entry2) {
    return AddChainAnsNumber(entry2, entry1);
}

static uint8_t AddVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    if (entry1->operand == entry2->operand) {
        ADD_HL_HL();
        return VALID;
    } else {
        return AddChainAnsVariable(entry1, entry2);
    }
}

static uint8_t AddVariableFunction(element_t *entry1, element_t *entry2) {
    return AddFunctionVariable(entry2, entry1);
}

static uint8_t AddVariableChainAns(element_t *entry1, element_t *entry2) {
    return AddChainAnsVariable(entry2, entry1);
}

static uint8_t AddFunctionFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NEED_PUSH);
    ADD_HL_DE();
    return VALID;
}

static uint8_t AddChainAnsFunction(element_t *entry1, element_t *entry2) {
    PUSH_HL();
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NO_PUSH);
    POP_DE();
    ADD_HL_DE();
    return VALID;
}

static uint8_t AddFunctionChainAns(element_t *entry1, element_t *entry2) {
    return AddChainAnsFunction(entry2, entry1);
}

static uint8_t AddChainPushNumber(element_t *entry1, element_t *entry2) {
    POP_HL();
    return AddChainAnsNumber(entry1, entry2);
}

static uint8_t AddChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    return AddChainAnsVariable(entry1, entry2);
}

static uint8_t AddChainPushFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NO_PUSH);
    POP_DE();
    ADD_HL_DE();
    return VALID;
}

static uint8_t AddChainPushChainAns(element_t *entry1, element_t *entry2) {
    POP_DE();
    ADD_HL_DE();
    return VALID;
}

static uint8_t AddError(element_t *entry1, element_t *entry2) {
    return E_SYNTAX;
}

uint8_t (*operatorFunctions[20])(element_t*, element_t*) = {
    AddError,
    AddNumberVariable,
    AddNumberFunction,
    AddNumberChainAns,
    AddVariableNumber,
    AddVariableVariable,
    AddVariableFunction,
    AddVariableChainAns,
    AddFunctionNumber,
    AddFunctionVariable,
    AddFunctionFunction,
    AddFunctionChainAns,
    AddChainAnsNumber,
    AddChainAnsVariable,
    AddChainAnsFunction,
    AddError,
    AddChainPushNumber,
    AddChainPushVariable,
    AddChainPushFunction,
    AddChainPushChainAns,
};


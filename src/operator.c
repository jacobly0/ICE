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
            return operand1 ^ operand2;
        case tAnd:
            return operand1 && operand2;
        default:
            return operand1;
    }
}

void parseOperator(element_t *outputPrevPrev, element_t *outputPrev, element_t *outputCurr) {
    switch (outputCurr->type) {
        case tAdd:
            switch (outputPrevPrev->type) {
                case TYPE_NUMBER:
                    // Number, variable, chainans, function
                case TYPE_VARIABLE:
                    // Number, variable, chainans, function
                case TYPE_CHAIN_PUSH:
                    // Number, variable, chainans, function
                case TYPE_CHAIN_ANS:
                    // Number, variable,         , function
                case TYPE_FUNCTION_RETURN:
                    // Number, variable, chainans, function
                       return;
            }
        case tSub:
        case tMul:
        case tDiv:
        case tNE:
        case tGE:
        case tLE:
        case tGT:
        case tLT:
        case tEQ:
        case tOr:
        case tXor:
        case tAnd:
        case tStore:
            return;
    }
}
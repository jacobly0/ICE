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
#include "stack.h"

extern void (*operatorFunctions[60])(element_t*, element_t*);
const char operators[] = {tStore, tAnd, tXor, tOr, tEQ, tLT, tGT, tLE, tGE, tNE, tMul, tDiv, tAdd, tSub};
const uint8_t operatorPrecedence[] = {0, 1, 2, 2, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4};

uint8_t getIndexOfOperator(uint8_t operator) {
    char *index;
    if ((index = strchr(operators, operator))) {
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
    uint8_t typeMasked1 = outputPrevPrev->type & 15;
    uint8_t typeMasked2 = outputPrev->type & 15;
    
    // Only call the function if both types are valid
    if ((typeMasked1 == typeMasked2 && (typeMasked1 == TYPE_NUMBER || typeMasked1 == TYPE_CHAIN_ANS)) || \
        (typeMasked1 > TYPE_CHAIN_PUSH && typeMasked2 > TYPE_CHAIN_ANS)) {
        return E_SYNTAX;
    }
    (*operatorFunctions[((getIndexOfOperator(outputCurr->operand) - 1) * 20) + (outputPrevPrev->operand * 5) + (outputPrev->operand)]) \
        (outputPrevPrev, outputPrev);
    return VALID;
}

void insertFunctionReturn(uint24_t function, uint8_t outputRegister, uint8_t needPush) {
    if ((uint8_t)function == tRand) {
        // TODO
    } else {
        // Check if the getKey has a fast direct key argument; if so, the second byte is 1
        if (function & 0x00FF00) {
            uint8_t key = function >> 16;
            // TODO
        }
        
        // Else, a standalone "getKey"
        else {
            // The key should be in HL
            if (outputRegister == OUTPUT_IN_HL) {
                CALL(GetCSC);
                OR_A_A();
                SBC_HL_HL();
                LD_L_A();
            }
            
            // The key should be in DE
            else if (outputRegister == OUTPUT_IN_DE) {
                // HL may not be destroyed
                if (needPush) {
                    PUSH_HL();
                    CALL(GetCSC);
                    POP_HL();
                } else {
                    CALL(GetCSC);
                }
                LD_DE_IMM(0);
                LD_E_A();
            }
            
            // The key should be in BC
            else if (outputRegister == OUTPUT_IN_BC) {
                // HL may not be destroyed
                if (needPush) {
                    PUSH_HL();
                    CALL(GetCSC);
                    POP_HL();
                } else {
                    CALL(GetCSC);
                }
                LD_BC_IMM(0);
                LD_C_A();
            }
        }
    }
}

static void AddChainAnsNumber(element_t *entry1, element_t *entry2) {
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
}

static void AddVariableNumber(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    AddChainAnsNumber(entry1, entry2);
}

static void AddFunctionNumber(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    AddChainAnsNumber(entry1, entry2);
}

static void AddChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_DE_IND_IX_OFF(entry2->operand);
    ADD_HL_DE();
}

static void AddFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    AddChainAnsVariable(entry1, entry2);
}

static void AddNumberVariable(element_t *entry1, element_t *entry2) {
    AddVariableNumber(entry2, entry1);
}

static void AddNumberFunction(element_t *entry1, element_t *entry2) {
    AddFunctionNumber(entry2, entry1);
}

static void AddNumberChainAns(element_t *entry1, element_t *entry2) {
    AddChainAnsNumber(entry2, entry1);
}

static void AddVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    if (entry1->operand == entry2->operand) {
        ADD_HL_HL();
    } else {
        AddChainAnsVariable(entry1, entry2);
    }
}

static void AddVariableFunction(element_t *entry1, element_t *entry2) {
    AddFunctionVariable(entry2, entry1);
}

static void AddVariableChainAns(element_t *entry1, element_t *entry2) {
    AddChainAnsVariable(entry2, entry1);
}

static void AddFunctionFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NEED_PUSH);
    ADD_HL_DE();
}

static void AddChainAnsFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_DE, NEED_PUSH);
    ADD_HL_DE();
}

static void AddFunctionChainAns(element_t *entry1, element_t *entry2) {
    AddChainAnsFunction(entry2, entry1);
}

static void AddChainPushNumber(element_t *entry1, element_t *entry2) {
    POP_HL();
    AddChainAnsNumber(entry1, entry2);
}

static void AddChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    AddChainAnsVariable(entry1, entry2);
}

static void AddChainPushFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NO_PUSH);
    POP_DE();
    ADD_HL_DE();
}

static void AddChainPushChainAns(element_t *entry1, element_t *entry2) {
    POP_DE();
    ADD_HL_DE();
}

static void SubChainAnsNumber(element_t *entry1, element_t *entry2) {
    uint24_t number = entry2->operand;
    
    if (number < 5) {
        uint8_t a;
        for (a = 0; a < (uint8_t)number; a++) {
            DEC_HL();
        }
    } else {
        LD_DE_IMM(0x1000000 - number);
        ADD_HL_DE();
    }
}

static void SubChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_DE_IND_IX_OFF(entry2->operand);
    SBC_HL_DE();
}

static void SubNumberVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IMM(entry1->operand);
    LD_DE_IND_IX_OFF(entry1->operand);
    SBC_HL_DE();
}

static void SubNumberFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_DE, NO_PUSH);
    LD_HL_IMM(entry1->operand);
    SBC_HL_DE();
}

static void SubNumberChainAns(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    LD_HL_IMM(entry1->operand);
    SBC_HL_DE();
}

static void SubVariableNumber(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    return SubChainAnsNumber(entry1, entry2);
}

static void SubVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    return SubChainAnsVariable(entry1, entry2);
}

static void SubVariableFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_DE, NO_PUSH);
    LD_HL_IND_IX_OFF(entry1->operand);
}

static void SubVariableChainAns(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    LD_HL_IND_IX_OFF(entry1->operand);
    SBC_HL_DE();
}

static void SubFunctionNumber(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    SubChainAnsNumber(entry1, entry2);
}

static void SubFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    SubChainAnsVariable(entry1, entry2);
}

static void SubFunctionFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NEED_PUSH);
    SBC_HL_DE();
}

static void SubFunctionChainAns(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NEED_PUSH);
    SBC_HL_DE();
}

static void SubChainAnsFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_DE, NEED_PUSH);
    SBC_HL_DE();
}

static void SubChainPushNumber(element_t *entry1, element_t *entry2) {
    POP_HL();
    SubChainAnsNumber(entry1, entry2);
}

static void SubChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    SubChainAnsVariable(entry1, entry2);
}

static void SubChainPushFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_DE, NO_PUSH);
    POP_HL();
    SBC_HL_DE();
}

static void SubChainPushChainAns(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    POP_HL();
    SBC_HL_DE();
}

static void MulChainAnsNumber(element_t *entry1, element_t *entry2) {
    uint24_t number = entry2->operand;
    
    if (number == 0) {
        OR_A_A();
        SBC_HL_HL();
    } else if (number >= 1 && number <= 20) {
        // TODO
    } else if (number > 20){
        LD_BC_IMM(number);
        CALL_IMULS();
    }
}

static void MulVariableNumber(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    MulChainAnsNumber(entry1, entry2);
}

static void MulFunctionNumber(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    MulChainAnsNumber(entry1, entry2);
}

static void MulChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_BC_IND_IX_OFF(entry2->operand);
    CALL_IMULS();
}

static void MulFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    MulChainAnsVariable(entry1, entry2);
}

static void MulNumberVariable(element_t *entry1, element_t *entry2) {
    MulVariableNumber(entry2, entry1);
}

static void MulNumberFunction(element_t *entry1, element_t *entry2) {
    MulFunctionNumber(entry2, entry1);
}

static void MulNumberChainAns(element_t *entry1, element_t *entry2) {
    MulChainAnsNumber(entry2, entry1);
}

static void MulVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    MulChainAnsVariable(entry1, entry2);
}

static void MulVariableFunction(element_t *entry1, element_t *entry2) {
    MulFunctionVariable(entry2, entry1);
}

static void MulVariableChainAns(element_t *entry1, element_t *entry2) {
    MulChainAnsVariable(entry2, entry1);
}

static void MulFunctionFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_BC, NO_PUSH);
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NEED_PUSH);
    CALL_IMULS();
}

static void MulChainAnsFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_BC, NEED_PUSH);
    CALL_IMULS();
}

static void MulFunctionChainAns(element_t *entry1, element_t *entry2) {
    MulChainAnsFunction(entry2, entry1);
}

static void MulChainPushNumber(element_t *entry1, element_t *entry2) {
    POP_HL();
    MulChainAnsNumber(entry1, entry2);
}

static void MulChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    MulChainAnsVariable(entry1, entry2);
}

static void MulChainPushFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NO_PUSH);
    POP_BC();
    CALL_IMULS();
}

static void MulChainPushChainAns(element_t *entry1, element_t *entry2) {
    POP_BC();
    CALL_IMULS();
}

static void OperatorError(element_t *entry1, element_t *entry2) {
}

void (*operatorFunctions[60])(element_t*, element_t*) = {
    OperatorError,
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
    OperatorError,
    AddChainPushNumber,
    AddChainPushVariable,
    AddChainPushFunction,
    AddChainPushChainAns,
    
    OperatorError,
    SubNumberVariable,
    SubNumberFunction,
    SubNumberChainAns,
    SubVariableNumber,
    SubVariableVariable,
    SubVariableFunction,
    SubVariableChainAns,
    SubFunctionNumber,
    SubFunctionVariable,
    SubFunctionFunction,
    SubFunctionChainAns,
    SubChainAnsNumber,
    SubChainAnsVariable,
    SubChainAnsFunction,
    OperatorError,
    SubChainPushNumber,
    SubChainPushVariable,
    SubChainPushFunction,
    SubChainPushChainAns,
    
    OperatorError,
    MulNumberVariable,
    MulNumberFunction,
    MulNumberChainAns,
    MulVariableNumber,
    MulVariableVariable,
    MulVariableFunction,
    MulVariableChainAns,
    MulFunctionNumber,
    MulFunctionVariable,
    MulFunctionFunction,
    MulFunctionChainAns,
    MulChainAnsNumber,
    MulChainAnsVariable,
    MulChainAnsFunction,
    OperatorError,
    MulChainPushNumber,
    MulChainPushVariable,
    MulChainPushFunction,
    MulChainPushChainAns,
};


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

extern uint8_t (*operatorFunctions[60])(element_t*, element_t*);
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
    // Only call the function if the types of both arguments are not more than TYPE_CHAIN_PUSH
    if ((outputPrev->type & 15) <= TYPE_CHAIN_ANS && (outputPrevPrev->type & 15) <= TYPE_CHAIN_PUSH) {
        return (*operatorFunctions[((getIndexOfOperator(outputCurr->operand) - 1) * 20) + \
               (outputPrevPrev->operand * 5) + \
               (outputPrev->operand)]) \
                (outputPrevPrev, outputPrev);
    }
    return E_SYNTAX;
}

void insertFunctionReturn(uint24_t function, uint8_t outputRegister, uint8_t needPush) {
    if ((uint8_t)function == tRand) {
        
    } else {
        // Check if the getKey has a fast direct key argument; if so, the second byte is 1
        if (function & 0x00FF00) {
            uint8_t key = function >> 16;
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
    insertFunctionReturn((uint8_t)entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    return AddChainAnsNumber(entry1, entry2);
}

static uint8_t AddChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_DE_IND_IX_OFF(entry2->operand);
    ADD_HL_DE();
    return VALID;
}

static uint8_t AddFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry1->operand, OUTPUT_IN_HL, NO_PUSH);
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
    insertFunctionReturn((uint8_t)entry1->operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn((uint8_t)entry2->operand, OUTPUT_IN_HL, NEED_PUSH);
    ADD_HL_DE();
    return VALID;
}

static uint8_t AddChainAnsFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry2->operand, OUTPUT_IN_DE, NEED_PUSH);
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
    insertFunctionReturn((uint8_t)entry2->operand, OUTPUT_IN_HL, NO_PUSH);
    POP_DE();
    ADD_HL_DE();
    return VALID;
}

static uint8_t AddChainPushChainAns(element_t *entry1, element_t *entry2) {
    POP_DE();
    ADD_HL_DE();
    return VALID;
}









static uint8_t SubChainAnsNumber(element_t *entry1, element_t *entry2) {
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
    return VALID;

}

static uint8_t SubChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_DE_IND_IX_OFF(entry2->operand);
    SBC_HL_DE();
    return VALID;
}

static uint8_t SubNumberVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IMM(entry1->operand);
    LD_DE_IND_IX_OFF(entry1->operand);
    SBC_HL_DE();
    return VALID;
}

static uint8_t SubNumberFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry2->operand, OUTPUT_IN_DE, NO_PUSH);
    LD_HL_IMM(entry1->operand);
    SBC_HL_DE();
    return VALID;
}

static uint8_t SubNumberChainAns(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    LD_HL_IMM(entry1->operand);
    SBC_HL_DE();
    return VALID;
}

static uint8_t SubVariableNumber(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    return SubChainAnsNumber(entry1, entry2);
}

static uint8_t SubVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    return SubChainAnsVariable(entry1, entry2);
}

static uint8_t SubVariableFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry2->operand, OUTPUT_IN_DE, NO_PUSH);
    LD_HL_IND_IX_OFF(entry1->operand);
    return VALID;
}

static uint8_t SubVariableChainAns(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    LD_HL_IND_IX_OFF(entry1->operand);
    SBC_HL_DE();
    return VALID;
}

static uint8_t SubFunctionNumber(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    return SubChainAnsNumber(entry1, entry2);
}

static uint8_t SubFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    return SubChainAnsVariable(entry1, entry2);
}

static uint8_t SubFunctionFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry2->operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn((uint8_t)entry1->operand, OUTPUT_IN_HL, NEED_PUSH);
    SBC_HL_DE();
    return VALID;
}

static uint8_t SubFunctionChainAns(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    insertFunctionReturn((uint8_t)entry1->operand, OUTPUT_IN_HL, NEED_PUSH);
    SBC_HL_DE();
    return VALID;
}

static uint8_t SubChainAnsFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry2->operand, OUTPUT_IN_DE, NEED_PUSH);
    SBC_HL_DE();
    return VALID;
}

static uint8_t SubChainPushNumber(element_t *entry1, element_t *entry2) {
    POP_HL();
    return SubChainAnsNumber(entry1, entry2);
}

static uint8_t SubChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    return SubChainAnsVariable(entry1, entry2);
}

static uint8_t SubChainPushFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry2->operand, OUTPUT_IN_DE, NO_PUSH);
    POP_HL();
    SBC_HL_DE();
    return VALID;
}

static uint8_t SubChainPushChainAns(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    POP_HL();
    SBC_HL_DE();
    return VALID;
}





static uint8_t MulChainAnsNumber(element_t *entry1, element_t *entry2) {
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
    return VALID;
}

static uint8_t MulVariableNumber(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    return MulChainAnsNumber(entry1, entry2);
}

static uint8_t MulFunctionNumber(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    return MulChainAnsNumber(entry1, entry2);
}

static uint8_t MulChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_BC_IND_IX_OFF(entry2->operand);
    CALL_IMULS();
    return VALID;
}

static uint8_t MulFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    return MulChainAnsVariable(entry1, entry2);
}

static uint8_t MulNumberVariable(element_t *entry1, element_t *entry2) {
    return MulVariableNumber(entry2, entry1);
}

static uint8_t MulNumberFunction(element_t *entry1, element_t *entry2) {
    return MulFunctionNumber(entry2, entry1);
}

static uint8_t MulNumberChainAns(element_t *entry1, element_t *entry2) {
    return MulChainAnsNumber(entry2, entry1);
}

static uint8_t MulVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    return MulChainAnsVariable(entry1, entry2);
}

static uint8_t MulVariableFunction(element_t *entry1, element_t *entry2) {
    return MulFunctionVariable(entry2, entry1);
}

static uint8_t MulVariableChainAns(element_t *entry1, element_t *entry2) {
    return MulChainAnsVariable(entry2, entry1);
}

static uint8_t MulFunctionFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry1->operand, OUTPUT_IN_BC, NO_PUSH);
    insertFunctionReturn((uint8_t)entry2->operand, OUTPUT_IN_HL, NEED_PUSH);
    CALL_IMULS();
    return VALID;
}

static uint8_t MulChainAnsFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry2->operand, OUTPUT_IN_BC, NEED_PUSH);
    CALL_IMULS();
    return VALID;
}

static uint8_t MulFunctionChainAns(element_t *entry1, element_t *entry2) {
    return MulChainAnsFunction(entry2, entry1);
}

static uint8_t MulChainPushNumber(element_t *entry1, element_t *entry2) {
    POP_HL();
    return MulChainAnsNumber(entry1, entry2);
}

static uint8_t MulChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    return MulChainAnsVariable(entry1, entry2);
}

static uint8_t MulChainPushFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn((uint8_t)entry2->operand, OUTPUT_IN_HL, NO_PUSH);
    POP_BC();
    CALL_IMULS();
    return VALID;
}

static uint8_t MulChainPushChainAns(element_t *entry1, element_t *entry2) {
    POP_BC();
    CALL_IMULS();
    return VALID;
}



static uint8_t OperatorError(element_t *entry1, element_t *entry2) {
    return E_SYNTAX;
}

uint8_t (*operatorFunctions[60])(element_t*, element_t*) = {
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


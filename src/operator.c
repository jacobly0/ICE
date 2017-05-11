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

extern void (*operatorFunctions[280])(element_t*, element_t*);
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
    
    // Call the right function!
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













static void OperatorError(element_t *entry1, element_t *entry2) {
}
static void StoNumberVariable(element_t *entry1, element_t *entry2) {
}
static void StoVariableVariable(element_t *entry1, element_t *entry2) {
}
static void StoFunctionVariable(element_t *entry1, element_t *entry2) {
}
static void StoChainAnsVariable(element_t *entry1, element_t *entry2) {
}
static void StoChainPushVariable(element_t *entry1, element_t *entry2) {
}
static void AndNumberVariable(element_t *entry1, element_t *entry2) {
}
static void AndNumberFunction(element_t *entry1, element_t *entry2) {
}
static void AndNumberChainAns(element_t *entry1, element_t *entry2) {
}
static void AndVariableNumber(element_t *entry1, element_t *entry2) {
}
static void AndVariableVariable(element_t *entry1, element_t *entry2) {
}
static void AndVariableFunction(element_t *entry1, element_t *entry2) {
}
static void AndVariableChainAns(element_t *entry1, element_t *entry2) {
}
static void AndFunctionNumber(element_t *entry1, element_t *entry2) {
}
static void AndFunctionVariable(element_t *entry1, element_t *entry2) {
}
static void AndFunctionFunction(element_t *entry1, element_t *entry2) {
}
static void AndFunctionChainAns(element_t *entry1, element_t *entry2) {
}
static void AndChainAnsNumber(element_t *entry1, element_t *entry2) {
}
static void AndChainAnsVariable(element_t *entry1, element_t *entry2) {
}
static void AndChainAnsFunction(element_t *entry1, element_t *entry2) {
}
static void AndChainPushNumber(element_t *entry1, element_t *entry2) {
}
static void AndChainPushVariable(element_t *entry1, element_t *entry2) {
}
static void AndChainPushFunction(element_t *entry1, element_t *entry2) {
}
static void AndChainPushChainAns(element_t *entry1, element_t *entry2) {
}
static void XorNumberVariable(element_t *entry1, element_t *entry2) {
}
static void XorNumberFunction(element_t *entry1, element_t *entry2) {
}
static void XorNumberChainAns(element_t *entry1, element_t *entry2) {
}
static void XorVariableNumber(element_t *entry1, element_t *entry2) {
}
static void XorVariableVariable(element_t *entry1, element_t *entry2) {
}
static void XorVariableFunction(element_t *entry1, element_t *entry2) {
}
static void XorVariableChainAns(element_t *entry1, element_t *entry2) {
}
static void XorFunctionNumber(element_t *entry1, element_t *entry2) {
}
static void XorFunctionVariable(element_t *entry1, element_t *entry2) {
}
static void XorFunctionFunction(element_t *entry1, element_t *entry2) {
}
static void XorFunctionChainAns(element_t *entry1, element_t *entry2) {
}
static void XorChainAnsNumber(element_t *entry1, element_t *entry2) {
}
static void XorChainAnsVariable(element_t *entry1, element_t *entry2) {
}
static void XorChainAnsFunction(element_t *entry1, element_t *entry2) {
}
static void XorChainPushNumber(element_t *entry1, element_t *entry2) {
}
static void XorChainPushVariable(element_t *entry1, element_t *entry2) {
}
static void XorChainPushFunction(element_t *entry1, element_t *entry2) {
}
static void XorChainPushChainAns(element_t *entry1, element_t *entry2) {
}
static void OrNumberVariable(element_t *entry1, element_t *entry2) {
}
static void OrNumberFunction(element_t *entry1, element_t *entry2) {
}
static void OrNumberChainAns(element_t *entry1, element_t *entry2) {
}
static void OrVariableNumber(element_t *entry1, element_t *entry2) {
}
static void OrVariableVariable(element_t *entry1, element_t *entry2) {
}
static void OrVariableFunction(element_t *entry1, element_t *entry2) {
}
static void OrVariableChainAns(element_t *entry1, element_t *entry2) {
}
static void OrFunctionNumber(element_t *entry1, element_t *entry2) {
}
static void OrFunctionVariable(element_t *entry1, element_t *entry2) {
}
static void OrFunctionFunction(element_t *entry1, element_t *entry2) {
}
static void OrFunctionChainAns(element_t *entry1, element_t *entry2) {
}
static void OrChainAnsNumber(element_t *entry1, element_t *entry2) {
}
static void OrChainAnsVariable(element_t *entry1, element_t *entry2) {
}
static void OrChainAnsFunction(element_t *entry1, element_t *entry2) {
}
static void OrChainPushNumber(element_t *entry1, element_t *entry2) {
}
static void OrChainPushVariable(element_t *entry1, element_t *entry2) {
}
static void OrChainPushFunction(element_t *entry1, element_t *entry2) {
}
static void OrChainPushChainAns(element_t *entry1, element_t *entry2) {
}
static void EQNumberVariable(element_t *entry1, element_t *entry2) {
}
static void EQNumberFunction(element_t *entry1, element_t *entry2) {
}
static void EQNumberChainAns(element_t *entry1, element_t *entry2) {
}
static void EQVariableNumber(element_t *entry1, element_t *entry2) {
}
static void EQVariableVariable(element_t *entry1, element_t *entry2) {
}
static void EQVariableFunction(element_t *entry1, element_t *entry2) {
}
static void EQVariableChainAns(element_t *entry1, element_t *entry2) {
}
static void EQFunctionNumber(element_t *entry1, element_t *entry2) {
}
static void EQFunctionVariable(element_t *entry1, element_t *entry2) {
}
static void EQFunctionFunction(element_t *entry1, element_t *entry2) {
}
static void EQFunctionChainAns(element_t *entry1, element_t *entry2) {
}
static void EQChainAnsNumber(element_t *entry1, element_t *entry2) {
}
static void EQChainAnsVariable(element_t *entry1, element_t *entry2) {
}
static void EQChainAnsFunction(element_t *entry1, element_t *entry2) {
}
static void EQChainPushNumber(element_t *entry1, element_t *entry2) {
}
static void EQChainPushVariable(element_t *entry1, element_t *entry2) {
}
static void EQChainPushFunction(element_t *entry1, element_t *entry2) {
}
static void EQChainPushChainAns(element_t *entry1, element_t *entry2) {
}
static void LTNumberVariable(element_t *entry1, element_t *entry2) {
}
static void LTNumberFunction(element_t *entry1, element_t *entry2) {
}
static void LTNumberChainAns(element_t *entry1, element_t *entry2) {
}
static void LTVariableNumber(element_t *entry1, element_t *entry2) {
}
static void LTVariableVariable(element_t *entry1, element_t *entry2) {
}
static void LTVariableFunction(element_t *entry1, element_t *entry2) {
}
static void LTVariableChainAns(element_t *entry1, element_t *entry2) {
}
static void LTFunctionNumber(element_t *entry1, element_t *entry2) {
}
static void LTFunctionVariable(element_t *entry1, element_t *entry2) {
}
static void LTFunctionFunction(element_t *entry1, element_t *entry2) {
}
static void LTFunctionChainAns(element_t *entry1, element_t *entry2) {
}
static void LTChainAnsNumber(element_t *entry1, element_t *entry2) {
}
static void LTChainAnsVariable(element_t *entry1, element_t *entry2) {
}
static void LTChainAnsFunction(element_t *entry1, element_t *entry2) {
}
static void LTChainPushNumber(element_t *entry1, element_t *entry2) {
}
static void LTChainPushVariable(element_t *entry1, element_t *entry2) {
}
static void LTChainPushFunction(element_t *entry1, element_t *entry2) {
}
static void LTChainPushChainAns(element_t *entry1, element_t *entry2) {
}
static void GTNumberVariable(element_t *entry1, element_t *entry2) {
}
static void GTNumberFunction(element_t *entry1, element_t *entry2) {
}
static void GTNumberChainAns(element_t *entry1, element_t *entry2) {
}
static void GTVariableNumber(element_t *entry1, element_t *entry2) {
}
static void GTVariableVariable(element_t *entry1, element_t *entry2) {
}
static void GTVariableFunction(element_t *entry1, element_t *entry2) {
}
static void GTVariableChainAns(element_t *entry1, element_t *entry2) {
}
static void GTFunctionNumber(element_t *entry1, element_t *entry2) {
}
static void GTFunctionVariable(element_t *entry1, element_t *entry2) {
}
static void GTFunctionFunction(element_t *entry1, element_t *entry2) {
}
static void GTFunctionChainAns(element_t *entry1, element_t *entry2) {
}
static void GTChainAnsNumber(element_t *entry1, element_t *entry2) {
}
static void GTChainAnsVariable(element_t *entry1, element_t *entry2) {
}
static void GTChainAnsFunction(element_t *entry1, element_t *entry2) {
}
static void GTChainPushNumber(element_t *entry1, element_t *entry2) {
}
static void GTChainPushVariable(element_t *entry1, element_t *entry2) {
}
static void GTChainPushFunction(element_t *entry1, element_t *entry2) {
}
static void GTChainPushChainAns(element_t *entry1, element_t *entry2) {
}
static void LENumberVariable(element_t *entry1, element_t *entry2) {
}
static void LENumberFunction(element_t *entry1, element_t *entry2) {
}
static void LENumberChainAns(element_t *entry1, element_t *entry2) {
}
static void LEVariableNumber(element_t *entry1, element_t *entry2) {
}
static void LEVariableVariable(element_t *entry1, element_t *entry2) {
}
static void LEVariableFunction(element_t *entry1, element_t *entry2) {
}
static void LEVariableChainAns(element_t *entry1, element_t *entry2) {
}
static void LEFunctionNumber(element_t *entry1, element_t *entry2) {
}
static void LEFunctionVariable(element_t *entry1, element_t *entry2) {
}
static void LEFunctionFunction(element_t *entry1, element_t *entry2) {
}
static void LEFunctionChainAns(element_t *entry1, element_t *entry2) {
}
static void LEChainAnsNumber(element_t *entry1, element_t *entry2) {
}
static void LEChainAnsVariable(element_t *entry1, element_t *entry2) {
}
static void LEChainAnsFunction(element_t *entry1, element_t *entry2) {
}
static void LEChainPushNumber(element_t *entry1, element_t *entry2) {
}
static void LEChainPushVariable(element_t *entry1, element_t *entry2) {
}
static void LEChainPushFunction(element_t *entry1, element_t *entry2) {
}
static void LEChainPushChainAns(element_t *entry1, element_t *entry2) {
}
static void GENumberVariable(element_t *entry1, element_t *entry2) {
}
static void GENumberFunction(element_t *entry1, element_t *entry2) {
}
static void GENumberChainAns(element_t *entry1, element_t *entry2) {
}
static void GEVariableNumber(element_t *entry1, element_t *entry2) {
}
static void GEVariableVariable(element_t *entry1, element_t *entry2) {
}
static void GEVariableFunction(element_t *entry1, element_t *entry2) {
}
static void GEVariableChainAns(element_t *entry1, element_t *entry2) {
}
static void GEFunctionNumber(element_t *entry1, element_t *entry2) {
}
static void GEFunctionVariable(element_t *entry1, element_t *entry2) {
}
static void GEFunctionFunction(element_t *entry1, element_t *entry2) {
}
static void GEFunctionChainAns(element_t *entry1, element_t *entry2) {
}
static void GEChainAnsNumber(element_t *entry1, element_t *entry2) {
}
static void GEChainAnsVariable(element_t *entry1, element_t *entry2) {
}
static void GEChainAnsFunction(element_t *entry1, element_t *entry2) {
}
static void GEChainPushNumber(element_t *entry1, element_t *entry2) {
}
static void GEChainPushVariable(element_t *entry1, element_t *entry2) {
}
static void GEChainPushFunction(element_t *entry1, element_t *entry2) {
}
static void GEChainPushChainAns(element_t *entry1, element_t *entry2) {
}
static void NENumberVariable(element_t *entry1, element_t *entry2) {
}
static void NENumberFunction(element_t *entry1, element_t *entry2) {
}
static void NENumberChainAns(element_t *entry1, element_t *entry2) {
}
static void NEVariableNumber(element_t *entry1, element_t *entry2) {
}
static void NEVariableVariable(element_t *entry1, element_t *entry2) {
}
static void NEVariableFunction(element_t *entry1, element_t *entry2) {
}
static void NEVariableChainAns(element_t *entry1, element_t *entry2) {
}
static void NEFunctionNumber(element_t *entry1, element_t *entry2) {
}
static void NEFunctionVariable(element_t *entry1, element_t *entry2) {
}
static void NEFunctionFunction(element_t *entry1, element_t *entry2) {
}
static void NEFunctionChainAns(element_t *entry1, element_t *entry2) {
}
static void NEChainAnsNumber(element_t *entry1, element_t *entry2) {
}
static void NEChainAnsVariable(element_t *entry1, element_t *entry2) {
}
static void NEChainAnsFunction(element_t *entry1, element_t *entry2) {
}
static void NEChainPushNumber(element_t *entry1, element_t *entry2) {
}
static void NEChainPushVariable(element_t *entry1, element_t *entry2) {
}
static void NEChainPushFunction(element_t *entry1, element_t *entry2) {
}
static void NEChainPushChainAns(element_t *entry1, element_t *entry2) {
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
        CALL(_imuls);
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
    CALL(_imuls);
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
    CALL(_imuls);
}
static void MulChainAnsFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_BC, NEED_PUSH);
    CALL(_imuls);
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
    CALL(_imuls);
}
static void MulChainPushChainAns(element_t *entry1, element_t *entry2) {
    POP_BC();
    CALL(_imuls);
}
static void DivNumberVariable(element_t *entry1, element_t *entry2) {
}
static void DivNumberFunction(element_t *entry1, element_t *entry2) {
}
static void DivNumberChainAns(element_t *entry1, element_t *entry2) {
}
static void DivVariableNumber(element_t *entry1, element_t *entry2) {
}
static void DivVariableVariable(element_t *entry1, element_t *entry2) {
}
static void DivVariableFunction(element_t *entry1, element_t *entry2) {
}
static void DivVariableChainAns(element_t *entry1, element_t *entry2) {
}
static void DivFunctionNumber(element_t *entry1, element_t *entry2) {
}
static void DivFunctionVariable(element_t *entry1, element_t *entry2) {
}
static void DivFunctionFunction(element_t *entry1, element_t *entry2) {
}
static void DivFunctionChainAns(element_t *entry1, element_t *entry2) {
}
static void DivChainAnsNumber(element_t *entry1, element_t *entry2) {
}
static void DivChainAnsVariable(element_t *entry1, element_t *entry2) {
}
static void DivChainAnsFunction(element_t *entry1, element_t *entry2) {
}
static void DivChainPushNumber(element_t *entry1, element_t *entry2) {
}
static void DivChainPushVariable(element_t *entry1, element_t *entry2) {
}
static void DivChainPushFunction(element_t *entry1, element_t *entry2) {
}
static void DivChainPushChainAns(element_t *entry1, element_t *entry2) {
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



void (*operatorFunctions[280])(element_t*, element_t*) = {
    OperatorError,
    StoNumberVariable,
    OperatorError,
    OperatorError,
    OperatorError,
    StoVariableVariable,
    OperatorError,
    OperatorError,
    OperatorError,
    StoFunctionVariable,
    OperatorError,
    OperatorError,
    OperatorError,
    StoChainAnsVariable,
    OperatorError,
    OperatorError,
    OperatorError,
    StoChainPushVariable,
    OperatorError,
    OperatorError,
    
    OperatorError,
    AndNumberVariable,
    AndNumberFunction,
    AndNumberChainAns,
    AndVariableNumber,
    AndVariableVariable,
    AndVariableFunction,
    AndVariableChainAns,
    AndFunctionNumber,
    AndFunctionVariable,
    AndFunctionFunction,
    AndFunctionChainAns,
    AndChainAnsNumber,
    AndChainAnsVariable,
    AndChainAnsFunction,
    OperatorError,
    AndChainPushNumber,
    AndChainPushVariable,
    AndChainPushFunction,
    AndChainPushChainAns,
    
    OperatorError,
    XorNumberVariable,
    XorNumberFunction,
    XorNumberChainAns,
    XorVariableNumber,
    XorVariableVariable,
    XorVariableFunction,
    XorVariableChainAns,
    XorFunctionNumber,
    XorFunctionVariable,
    XorFunctionFunction,
    XorFunctionChainAns,
    XorChainAnsNumber,
    XorChainAnsVariable,
    XorChainAnsFunction,
    OperatorError,
    XorChainPushNumber,
    XorChainPushVariable,
    XorChainPushFunction,
    XorChainPushChainAns,
    
    OperatorError,
    OrNumberVariable,
    OrNumberFunction,
    OrNumberChainAns,
    OrVariableNumber,
    OrVariableVariable,
    OrVariableFunction,
    OrVariableChainAns,
    OrFunctionNumber,
    OrFunctionVariable,
    OrFunctionFunction,
    OrFunctionChainAns,
    OrChainAnsNumber,
    OrChainAnsVariable,
    OrChainAnsFunction,
    OperatorError,
    OrChainPushNumber,
    OrChainPushVariable,
    OrChainPushFunction,
    OrChainPushChainAns,
    
    OperatorError,
    EQNumberVariable,
    EQNumberFunction,
    EQNumberChainAns,
    EQVariableNumber,
    EQVariableVariable,
    EQVariableFunction,
    EQVariableChainAns,
    EQFunctionNumber,
    EQFunctionVariable,
    EQFunctionFunction,
    EQFunctionChainAns,
    EQChainAnsNumber,
    EQChainAnsVariable,
    EQChainAnsFunction,
    OperatorError,
    EQChainPushNumber,
    EQChainPushVariable,
    EQChainPushFunction,
    EQChainPushChainAns,
    
    OperatorError,
    LTNumberVariable,
    LTNumberFunction,
    LTNumberChainAns,
    LTVariableNumber,
    LTVariableVariable,
    LTVariableFunction,
    LTVariableChainAns,
    LTFunctionNumber,
    LTFunctionVariable,
    LTFunctionFunction,
    LTFunctionChainAns,
    LTChainAnsNumber,
    LTChainAnsVariable,
    LTChainAnsFunction,
    OperatorError,
    LTChainPushNumber,
    LTChainPushVariable,
    LTChainPushFunction,
    LTChainPushChainAns,
    
    OperatorError,
    GTNumberVariable,
    GTNumberFunction,
    GTNumberChainAns,
    GTVariableNumber,
    GTVariableVariable,
    GTVariableFunction,
    GTVariableChainAns,
    GTFunctionNumber,
    GTFunctionVariable,
    GTFunctionFunction,
    GTFunctionChainAns,
    GTChainAnsNumber,
    GTChainAnsVariable,
    GTChainAnsFunction,
    OperatorError,
    GTChainPushNumber,
    GTChainPushVariable,
    GTChainPushFunction,
    GTChainPushChainAns,
    
    OperatorError,
    LENumberVariable,
    LENumberFunction,
    LENumberChainAns,
    LEVariableNumber,
    LEVariableVariable,
    LEVariableFunction,
    LEVariableChainAns,
    LEFunctionNumber,
    LEFunctionVariable,
    LEFunctionFunction,
    LEFunctionChainAns,
    LEChainAnsNumber,
    LEChainAnsVariable,
    LEChainAnsFunction,
    OperatorError,
    LEChainPushNumber,
    LEChainPushVariable,
    LEChainPushFunction,
    LEChainPushChainAns,
    
    OperatorError,
    GENumberVariable,
    GENumberFunction,
    GENumberChainAns,
    GEVariableNumber,
    GEVariableVariable,
    GEVariableFunction,
    GEVariableChainAns,
    GEFunctionNumber,
    GEFunctionVariable,
    GEFunctionFunction,
    GEFunctionChainAns,
    GEChainAnsNumber,
    GEChainAnsVariable,
    GEChainAnsFunction,
    OperatorError,
    GEChainPushNumber,
    GEChainPushVariable,
    GEChainPushFunction,
    GEChainPushChainAns,
    
    OperatorError,
    NENumberVariable,
    NENumberFunction,
    NENumberChainAns,
    NEVariableNumber,
    NEVariableVariable,
    NEVariableFunction,
    NEVariableChainAns,
    NEFunctionNumber,
    NEFunctionVariable,
    NEFunctionFunction,
    NEFunctionChainAns,
    NEChainAnsNumber,
    NEChainAnsVariable,
    NEChainAnsFunction,
    OperatorError,
    NEChainPushNumber,
    NEChainPushVariable,
    NEChainPushFunction,
    NEChainPushChainAns,
    
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
    
    OperatorError,
    DivNumberVariable,
    DivNumberFunction,
    DivNumberChainAns,
    DivVariableNumber,
    DivVariableVariable,
    DivVariableFunction,
    DivVariableChainAns,
    DivFunctionNumber,
    DivFunctionVariable,
    DivFunctionFunction,
    DivFunctionChainAns,
    DivChainAnsNumber,
    DivChainAnsVariable,
    DivChainAnsFunction,
    OperatorError,
    DivChainPushNumber,
    DivChainPushVariable,
    DivChainPushFunction,
    DivChainPushChainAns,
    
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
};


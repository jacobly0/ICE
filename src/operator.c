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
    
    dbg_Debugger();
    
    // Only call the function if both types are valid
    if ((typeMasked1 == typeMasked2 && (typeMasked1 == TYPE_NUMBER || typeMasked1 == TYPE_CHAIN_ANS)) || \
        (typeMasked1 > TYPE_CHAIN_PUSH && typeMasked2 > TYPE_CHAIN_ANS)) {
        return E_SYNTAX;
    }
    
    // Call the right function!
    (*operatorFunctions[((getIndexOfOperator(outputCurr->operand) - 1) * 20) + (typeMasked1 * 4) + typeMasked2]) \
        (outputPrevPrev, outputPrev);
    return VALID;
}

void insertFunctionReturn(uint24_t function, uint8_t outputRegister, uint8_t needPush) {
    if ((uint8_t)function == tRand) {
        // We need to save a register before using the routine
        if (needPush) {
            if (outputRegister == OUTPUT_IN_HL) {
                PUSH_DE();
            } else {
                PUSH_HL();
            }
        }
        
        // Store the pointer to the call to the stack, to replace later
        *ice.dataOffsetPtr++ = (uint24_t)ice.programPtr + 1;
        
        // We need to add the rand routine to the data section
        if (!ice.usedAlreadyRand) {
            ice.randAddr = (uint24_t)ice.programDataPtr;
            memcpy(ice.programDataPtr, RandRoutine, 54);
            ice.programDataPtr += 54;
            ice.usedAlreadyRand = true;
        }
        
        CALL(ice.randAddr);
        
        // Store the value to the right register
        if (outputRegister == OUTPUT_IN_DE) {
            EX_DE_HL();
        } else if (outputRegister == OUTPUT_IN_BC) {
            PUSH_HL();
            POP_BC();
        }
        
        // And restore the register of course
        if (needPush) {
            if (outputRegister == OUTPUT_IN_HL) {
                POP_DE();
            } else {
                POP_HL();
            }
        }
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
                CALL(_GetCSC);
                OR_A_A();
                SBC_HL_HL();
                LD_L_A();
            }
            
            // The key should be in DE
            else if (outputRegister == OUTPUT_IN_DE) {
                // HL may not be destroyed
                if (needPush) {
                    PUSH_HL();
                    CALL(_GetCSC);
                    POP_HL();
                } else {
                    CALL(_GetCSC);
                }
                LD_DE_IMM(0);
                LD_E_A();
            }
            
            // The key should be in BC
            else if (outputRegister == OUTPUT_IN_BC) {
                // HL may not be destroyed
                if (needPush) {
                    PUSH_HL();
                    CALL(_GetCSC);
                    POP_HL();
                } else {
                    CALL(_GetCSC);
                }
                LD_BC_IMM(0);
                LD_C_A();
            }
        }
    }
}



void LD_HL_NUMBER(uint24_t number) {
    if (!number) {
        OR_A_A();
        SBC_HL_HL();
    } else {
        LD_HL_IMM(number);
    }
}













void OperatorError(element_t *entry1, element_t *entry2) {
}
void StoChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_IX_OFF_IND_HL(entry2->operand);
}
void StoNumberVariable(element_t *entry1, element_t *entry2) {
    LD_HL_NUMBER(entry1->operand);
    StoChainAnsVariable(entry1, entry2);
}
void StoVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    StoChainAnsVariable(entry1, entry2);
}
void StoFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    StoChainAnsVariable(entry1, entry2);
}
void StoChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    StoChainAnsVariable(entry1, entry2);
}
void AndChainAnsNumber(element_t *entry1, element_t *entry2) {
    uint24_t number = entry2->operand;
    if (!number) {
        LD_HL_NUMBER(0);
    } else {
        LD_DE_IMM(-1);
        ADD_HL_DE();
        SBC_HL_HL();
        CCF();
        INC_HL();
    }
}
void AndChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_DE_IND_IX_OFF(entry2->operand);
    memcpy(ice.programPtr, AndData, 16);
    ice.programPtr += 16;
}
void AndChainAnsFunction(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NEED_PUSH);
    memcpy(ice.programPtr, AndData, 16);
    ice.programPtr += 16;
}
void AndFunctionNumber(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    AndChainAnsNumber(entry1, entry2);
}
void AndVariableNumber(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    AndChainAnsNumber(entry1, entry2);
}
void AndFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    AndChainAnsVariable(entry1, entry2);
}
void AndNumberVariable(element_t *entry1, element_t *entry2) {
    AndVariableNumber(entry2, entry1);
}
void AndNumberFunction(element_t *entry1, element_t *entry2) {
    AndFunctionNumber(entry2, entry1);
}
void AndNumberChainAns(element_t *entry1, element_t *entry2) {
    AndChainAnsNumber(entry2, entry1);
}
void AndVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    AndChainAnsVariable(entry1, entry2);
}
void AndVariableFunction(element_t *entry1, element_t *entry2) {
    AndFunctionVariable(entry2, entry1);
}
void AndVariableChainAns(element_t *entry1, element_t *entry2) {
    AndChainAnsVariable(entry2, entry1);
}
void AndFunctionFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NEED_PUSH);
    memcpy(ice.programPtr, AndData, 16);
    ice.programPtr += 16;
}
void AndFunctionChainAns(element_t *entry1, element_t *entry2) {
    AndChainAnsFunction(entry2, entry1);
}
void AndChainPushNumber(element_t *entry1, element_t *entry2) {
    POP_HL();
    AndChainAnsNumber(entry1, entry2);
}
void AndChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    AndChainAnsVariable(entry1, entry2);
}
void AndChainPushFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NO_PUSH);
    POP_DE();
    memcpy(ice.programPtr, AndData, 16);
    ice.programPtr += 16;
}
void AndChainPushChainAns(element_t *entry1, element_t *entry2) {
    POP_DE();
    memcpy(ice.programPtr, AndData, 16);
    ice.programPtr += 16;
}
void XorChainAnsNumber(element_t *entry1, element_t *entry2) {
    uint24_t number = entry2->operand;
    LD_DE_IMM(-1);
    ADD_HL_DE();
    if (!number) {
        CCF();
    }
    SBC_HL_HL();
    INC_HL();
}
void XorChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_DE_IND_IX_OFF(entry2->operand);
    memcpy(ice.programPtr, XorData, 16);
    ice.programPtr += 16;
}
void XorChainAnsFunction(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NEED_PUSH);
    memcpy(ice.programPtr, XorData, 16);
    ice.programPtr += 16;
}
void XorFunctionNumber(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    XorChainAnsNumber(entry1, entry2);
}
void XorVariableNumber(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    XorChainAnsNumber(entry1, entry2);
}
void XorFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    XorChainAnsVariable(entry1, entry2);
}
void XorNumberVariable(element_t *entry1, element_t *entry2) {
    XorVariableNumber(entry2, entry1);
}
void XorNumberFunction(element_t *entry1, element_t *entry2) {
    XorFunctionNumber(entry2, entry1);
}
void XorNumberChainAns(element_t *entry1, element_t *entry2) {
    XorChainAnsNumber(entry2, entry1);
}
void XorVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    XorChainAnsVariable(entry1, entry2);
}
void XorVariableFunction(element_t *entry1, element_t *entry2) {
    XorFunctionVariable(entry2, entry1);
}
void XorVariableChainAns(element_t *entry1, element_t *entry2) {
    XorChainAnsVariable(entry2, entry1);
}
void XorFunctionFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NEED_PUSH);
    memcpy(ice.programPtr, XorData, 16);
    ice.programPtr += 16;
}
void XorFunctionChainAns(element_t *entry1, element_t *entry2) {
    XorChainAnsFunction(entry2, entry1);
}
void XorChainPushNumber(element_t *entry1, element_t *entry2) {
    POP_HL();
    XorChainAnsNumber(entry1, entry2);
}
void XorChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    XorChainAnsVariable(entry1, entry2);
}
void XorChainPushFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NO_PUSH);
    POP_DE();
    memcpy(ice.programPtr, XorData, 16);
    ice.programPtr += 16;
}
void XorChainPushChainAns(element_t *entry1, element_t *entry2) {
    POP_DE();
    memcpy(ice.programPtr, XorData, 16);
    ice.programPtr += 16;
}
void OrChainAnsNumber(element_t *entry1, element_t *entry2) {
    uint24_t number = entry2->operand;
    if (!number) {
        LD_DE_IMM(-1);
        ADD_HL_DE();
        CCF();
        SBC_HL_HL();
        INC_HL();
    } else {
        LD_HL_NUMBER(1);
    }
}
void OrChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_DE_IND_IX_OFF(entry2->operand);
    memcpy(ice.programPtr, OrData, 16);
    ice.programPtr += 16;
}
void OrChainAnsFunction(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NEED_PUSH);
    memcpy(ice.programPtr, OrData, 16);
    ice.programPtr += 16;
}
void OrFunctionNumber(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    OrChainAnsNumber(entry1, entry2);
}
void OrVariableNumber(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    OrChainAnsNumber(entry1, entry2);
}
void OrFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    OrChainAnsVariable(entry1, entry2);
}
void OrNumberVariable(element_t *entry1, element_t *entry2) {
    OrVariableNumber(entry2, entry1);
}
void OrNumberFunction(element_t *entry1, element_t *entry2) {
    OrFunctionNumber(entry2, entry1);
}
void OrNumberChainAns(element_t *entry1, element_t *entry2) {
    OrChainAnsNumber(entry2, entry1);
}
void OrVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    OrChainAnsVariable(entry1, entry2);
}
void OrVariableFunction(element_t *entry1, element_t *entry2) {
    OrFunctionVariable(entry2, entry1);
}
void OrVariableChainAns(element_t *entry1, element_t *entry2) {
    OrChainAnsVariable(entry2, entry1);
}
void OrFunctionFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NEED_PUSH);
    memcpy(ice.programPtr, OrData, 16);
    ice.programPtr += 16;
}
void OrFunctionChainAns(element_t *entry1, element_t *entry2) {
    OrChainAnsFunction(entry2, entry1);
}
void OrChainPushNumber(element_t *entry1, element_t *entry2) {
    POP_HL();
    OrChainAnsNumber(entry1, entry2);
}
void OrChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    OrChainAnsVariable(entry1, entry2);
}
void OrChainPushFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NO_PUSH);
    POP_DE();
    memcpy(ice.programPtr, OrData, 16);
    ice.programPtr += 16;
}
void OrChainPushChainAns(element_t *entry1, element_t *entry2) {
    POP_DE();
    memcpy(ice.programPtr, OrData, 16);
    ice.programPtr += 16;
}
void EQNumberVariable(element_t *entry1, element_t *entry2) {
}
void EQNumberFunction(element_t *entry1, element_t *entry2) {
}
void EQNumberChainAns(element_t *entry1, element_t *entry2) {
}
void EQVariableNumber(element_t *entry1, element_t *entry2) {
}
void EQVariableVariable(element_t *entry1, element_t *entry2) {
}
void EQVariableFunction(element_t *entry1, element_t *entry2) {
}
void EQVariableChainAns(element_t *entry1, element_t *entry2) {
}
void EQFunctionNumber(element_t *entry1, element_t *entry2) {
}
void EQFunctionVariable(element_t *entry1, element_t *entry2) {
}
void EQFunctionFunction(element_t *entry1, element_t *entry2) {
}
void EQFunctionChainAns(element_t *entry1, element_t *entry2) {
}
void EQChainAnsNumber(element_t *entry1, element_t *entry2) {
}
void EQChainAnsVariable(element_t *entry1, element_t *entry2) {
}
void EQChainAnsFunction(element_t *entry1, element_t *entry2) {
}
void EQChainPushNumber(element_t *entry1, element_t *entry2) {
}
void EQChainPushVariable(element_t *entry1, element_t *entry2) {
}
void EQChainPushFunction(element_t *entry1, element_t *entry2) {
}
void EQChainPushChainAns(element_t *entry1, element_t *entry2) {
}
void LTNumberVariable(element_t *entry1, element_t *entry2) {
}
void LTNumberFunction(element_t *entry1, element_t *entry2) {
}
void LTNumberChainAns(element_t *entry1, element_t *entry2) {
}
void LTVariableNumber(element_t *entry1, element_t *entry2) {
}
void LTVariableVariable(element_t *entry1, element_t *entry2) {
}
void LTVariableFunction(element_t *entry1, element_t *entry2) {
}
void LTVariableChainAns(element_t *entry1, element_t *entry2) {
}
void LTFunctionNumber(element_t *entry1, element_t *entry2) {
}
void LTFunctionVariable(element_t *entry1, element_t *entry2) {
}
void LTFunctionFunction(element_t *entry1, element_t *entry2) {
}
void LTFunctionChainAns(element_t *entry1, element_t *entry2) {
}
void LTChainAnsNumber(element_t *entry1, element_t *entry2) {
}
void LTChainAnsVariable(element_t *entry1, element_t *entry2) {
}
void LTChainAnsFunction(element_t *entry1, element_t *entry2) {
}
void LTChainPushNumber(element_t *entry1, element_t *entry2) {
}
void LTChainPushVariable(element_t *entry1, element_t *entry2) {
}
void LTChainPushFunction(element_t *entry1, element_t *entry2) {
}
void LTChainPushChainAns(element_t *entry1, element_t *entry2) {
}
void GTNumberVariable(element_t *entry1, element_t *entry2) {
}
void GTNumberFunction(element_t *entry1, element_t *entry2) {
}
void GTNumberChainAns(element_t *entry1, element_t *entry2) {
}
void GTVariableNumber(element_t *entry1, element_t *entry2) {
}
void GTVariableVariable(element_t *entry1, element_t *entry2) {
}
void GTVariableFunction(element_t *entry1, element_t *entry2) {
}
void GTVariableChainAns(element_t *entry1, element_t *entry2) {
}
void GTFunctionNumber(element_t *entry1, element_t *entry2) {
}
void GTFunctionVariable(element_t *entry1, element_t *entry2) {
}
void GTFunctionFunction(element_t *entry1, element_t *entry2) {
}
void GTFunctionChainAns(element_t *entry1, element_t *entry2) {
}
void GTChainAnsNumber(element_t *entry1, element_t *entry2) {
}
void GTChainAnsVariable(element_t *entry1, element_t *entry2) {
}
void GTChainAnsFunction(element_t *entry1, element_t *entry2) {
}
void GTChainPushNumber(element_t *entry1, element_t *entry2) {
}
void GTChainPushVariable(element_t *entry1, element_t *entry2) {
}
void GTChainPushFunction(element_t *entry1, element_t *entry2) {
}
void GTChainPushChainAns(element_t *entry1, element_t *entry2) {
}
void LENumberVariable(element_t *entry1, element_t *entry2) {
}
void LENumberFunction(element_t *entry1, element_t *entry2) {
}
void LENumberChainAns(element_t *entry1, element_t *entry2) {
}
void LEVariableNumber(element_t *entry1, element_t *entry2) {
}
void LEVariableVariable(element_t *entry1, element_t *entry2) {
}
void LEVariableFunction(element_t *entry1, element_t *entry2) {
}
void LEVariableChainAns(element_t *entry1, element_t *entry2) {
}
void LEFunctionNumber(element_t *entry1, element_t *entry2) {
}
void LEFunctionVariable(element_t *entry1, element_t *entry2) {
}
void LEFunctionFunction(element_t *entry1, element_t *entry2) {
}
void LEFunctionChainAns(element_t *entry1, element_t *entry2) {
}
void LEChainAnsNumber(element_t *entry1, element_t *entry2) {
}
void LEChainAnsVariable(element_t *entry1, element_t *entry2) {
}
void LEChainAnsFunction(element_t *entry1, element_t *entry2) {
}
void LEChainPushNumber(element_t *entry1, element_t *entry2) {
}
void LEChainPushVariable(element_t *entry1, element_t *entry2) {
}
void LEChainPushFunction(element_t *entry1, element_t *entry2) {
}
void LEChainPushChainAns(element_t *entry1, element_t *entry2) {
}
void GENumberVariable(element_t *entry1, element_t *entry2) {
}
void GENumberFunction(element_t *entry1, element_t *entry2) {
}
void GENumberChainAns(element_t *entry1, element_t *entry2) {
}
void GEVariableNumber(element_t *entry1, element_t *entry2) {
}
void GEVariableVariable(element_t *entry1, element_t *entry2) {
}
void GEVariableFunction(element_t *entry1, element_t *entry2) {
}
void GEVariableChainAns(element_t *entry1, element_t *entry2) {
}
void GEFunctionNumber(element_t *entry1, element_t *entry2) {
}
void GEFunctionVariable(element_t *entry1, element_t *entry2) {
}
void GEFunctionFunction(element_t *entry1, element_t *entry2) {
}
void GEFunctionChainAns(element_t *entry1, element_t *entry2) {
}
void GEChainAnsNumber(element_t *entry1, element_t *entry2) {
}
void GEChainAnsVariable(element_t *entry1, element_t *entry2) {
}
void GEChainAnsFunction(element_t *entry1, element_t *entry2) {
}
void GEChainPushNumber(element_t *entry1, element_t *entry2) {
}
void GEChainPushVariable(element_t *entry1, element_t *entry2) {
}
void GEChainPushFunction(element_t *entry1, element_t *entry2) {
}
void GEChainPushChainAns(element_t *entry1, element_t *entry2) {
}
void NENumberVariable(element_t *entry1, element_t *entry2) {
}
void NENumberFunction(element_t *entry1, element_t *entry2) {
}
void NENumberChainAns(element_t *entry1, element_t *entry2) {
}
void NEVariableNumber(element_t *entry1, element_t *entry2) {
}
void NEVariableVariable(element_t *entry1, element_t *entry2) {
}
void NEVariableFunction(element_t *entry1, element_t *entry2) {
}
void NEVariableChainAns(element_t *entry1, element_t *entry2) {
}
void NEFunctionNumber(element_t *entry1, element_t *entry2) {
}
void NEFunctionVariable(element_t *entry1, element_t *entry2) {
}
void NEFunctionFunction(element_t *entry1, element_t *entry2) {
}
void NEFunctionChainAns(element_t *entry1, element_t *entry2) {
}
void NEChainAnsNumber(element_t *entry1, element_t *entry2) {
}
void NEChainAnsVariable(element_t *entry1, element_t *entry2) {
}
void NEChainAnsFunction(element_t *entry1, element_t *entry2) {
}
void NEChainPushNumber(element_t *entry1, element_t *entry2) {
}
void NEChainPushVariable(element_t *entry1, element_t *entry2) {
}
void NEChainPushFunction(element_t *entry1, element_t *entry2) {
}
void NEChainPushChainAns(element_t *entry1, element_t *entry2) {
}
void MulChainAnsNumber(element_t *entry1, element_t *entry2) {
    uint24_t number = entry2->operand;
    if (number == 0) {
        OR_A_A();
        SBC_HL_HL();
    } else {
        MultWithNumber(number, (uint24_t *)&ice.programPtr);
    }
}
void MulVariableNumber(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    MulChainAnsNumber(entry1, entry2);
}
void MulFunctionNumber(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    MulChainAnsNumber(entry1, entry2);
}
void MulChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_BC_IND_IX_OFF(entry2->operand);
    CALL(__imuls);
}
void MulFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    MulChainAnsVariable(entry1, entry2);
}
void MulNumberVariable(element_t *entry1, element_t *entry2) {
    MulVariableNumber(entry2, entry1);
}
void MulNumberFunction(element_t *entry1, element_t *entry2) {
    MulFunctionNumber(entry2, entry1);
}
void MulNumberChainAns(element_t *entry1, element_t *entry2) {
    MulChainAnsNumber(entry2, entry1);
}
void MulVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    MulChainAnsVariable(entry1, entry2);
}
void MulVariableFunction(element_t *entry1, element_t *entry2) {
    MulFunctionVariable(entry2, entry1);
}
void MulVariableChainAns(element_t *entry1, element_t *entry2) {
    MulChainAnsVariable(entry2, entry1);
}
void MulFunctionFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_BC, NO_PUSH);
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NEED_PUSH);
    CALL(__imuls);
}
void MulChainAnsFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_BC, NEED_PUSH);
    CALL(__imuls);
}
void MulFunctionChainAns(element_t *entry1, element_t *entry2) {
    MulChainAnsFunction(entry2, entry1);
}
void MulChainPushNumber(element_t *entry1, element_t *entry2) {
    POP_HL();
    MulChainAnsNumber(entry1, entry2);
}
void MulChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    MulChainAnsVariable(entry1, entry2);
}
void MulChainPushFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NO_PUSH);
    POP_BC();
    CALL(__imuls);
}
void MulChainPushChainAns(element_t *entry1, element_t *entry2) {
    POP_BC();
    CALL(__imuls);
}
void DivChainAnsNumber(element_t *entry1, element_t *entry2) {
    LD_BC_IMM(entry2->operand);
    CALL(__idvrmu);
    EX_DE_HL();
}
void DivChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_BC_IND_IX_OFF(entry2->operand);
    CALL(__idvrmu);
    EX_DE_HL();
}
void DivNumberVariable(element_t *entry1, element_t *entry2) {
    LD_HL_NUMBER(entry1->operand);
    DivChainAnsVariable(entry1, entry2);
}
void DivNumberFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_BC, NO_PUSH);
    LD_HL_NUMBER(entry1->operand);
    CALL(__idvrmu);
    EX_DE_HL();
}
void DivNumberChainAns(element_t *entry1, element_t *entry2) {
    PUSH_HL();
    POP_BC();
    LD_HL_NUMBER(entry1->operand);
    CALL(__idvrmu);
    EX_DE_HL();
}
void DivVariableNumber(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    DivChainAnsNumber(entry1, entry2);
}
void DivVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    DivChainAnsVariable(entry1, entry2);
}
void DivVariableFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_BC, NO_PUSH);
    LD_HL_IND_IX_OFF(entry1->operand);
    CALL(__idvrmu);
    EX_DE_HL();
}
void DivVariableChainAns(element_t *entry1, element_t *entry2) {
    PUSH_HL();
    POP_BC();
    LD_HL_IND_IX_OFF(entry1->operand);
    CALL(__idvrmu);
    EX_DE_HL();
}
void DivFunctionNumber(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    DivChainAnsNumber(entry1, entry2);
}
void DivFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    DivChainAnsVariable(entry1, entry2);
}
void DivFunctionFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_BC, NO_PUSH);
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NEED_PUSH);
    CALL(__idvrmu);
    EX_DE_HL();
}
void DivFunctionChainAns(element_t *entry1, element_t *entry2) {
    PUSH_HL();
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    POP_BC();
    CALL(__idvrmu);
    EX_DE_HL();
}
void DivChainAnsFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_BC, NEED_PUSH);
    CALL(__idvrmu);
    EX_DE_HL();
}
void DivChainPushNumber(element_t *entry1, element_t *entry2) {
    POP_HL();
    DivChainAnsNumber(entry1, entry2);
}
void DivChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    DivChainAnsVariable(entry1, entry2);
}
void DivChainPushFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_BC, NO_PUSH);
    POP_HL();
    CALL(__idvrmu);
    EX_DE_HL();
}
void DivChainPushChainAns(element_t *entry1, element_t *entry2) {
    PUSH_HL();
    POP_BC();
    POP_HL();
    CALL(__idvrmu);
    EX_DE_HL();
}
void AddChainAnsNumber(element_t *entry1, element_t *entry2) {
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
void AddVariableNumber(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    AddChainAnsNumber(entry1, entry2);
}
void AddFunctionNumber(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    AddChainAnsNumber(entry1, entry2);
}
void AddChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_DE_IND_IX_OFF(entry2->operand);
    ADD_HL_DE();
}
void AddFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    AddChainAnsVariable(entry1, entry2);
}
void AddNumberVariable(element_t *entry1, element_t *entry2) {
    AddVariableNumber(entry2, entry1);
}
void AddNumberFunction(element_t *entry1, element_t *entry2) {
    AddFunctionNumber(entry2, entry1);
}
void AddNumberChainAns(element_t *entry1, element_t *entry2) {
    AddChainAnsNumber(entry2, entry1);
}
void AddVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    if (entry1->operand == entry2->operand) {
        ADD_HL_HL();
    } else {
        AddChainAnsVariable(entry1, entry2);
    }
}
void AddVariableFunction(element_t *entry1, element_t *entry2) {
    AddFunctionVariable(entry2, entry1);
}
void AddVariableChainAns(element_t *entry1, element_t *entry2) {
    AddChainAnsVariable(entry2, entry1);
}
void AddFunctionFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NEED_PUSH);
    ADD_HL_DE();
}
void AddChainAnsFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_DE, NEED_PUSH);
    ADD_HL_DE();
}
void AddFunctionChainAns(element_t *entry1, element_t *entry2) {
    AddChainAnsFunction(entry2, entry1);
}
void AddChainPushNumber(element_t *entry1, element_t *entry2) {
    POP_HL();
    AddChainAnsNumber(entry1, entry2);
}
void AddChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    AddChainAnsVariable(entry1, entry2);
}
void AddChainPushFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_HL, NO_PUSH);
    POP_DE();
    ADD_HL_DE();
}
void AddChainPushChainAns(element_t *entry1, element_t *entry2) {
    POP_DE();
    ADD_HL_DE();
}
void SubChainAnsNumber(element_t *entry1, element_t *entry2) {
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
void SubChainAnsVariable(element_t *entry1, element_t *entry2) {
    LD_DE_IND_IX_OFF(entry2->operand);
    SBC_HL_DE();
}
void SubNumberVariable(element_t *entry1, element_t *entry2) {
    LD_HL_NUMBER(entry1->operand);
    SubChainAnsVariable(entry1, entry2);
}
void SubNumberFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_DE, NO_PUSH);
    LD_HL_NUMBER(entry1->operand);
    SBC_HL_DE();
}
void SubNumberChainAns(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    LD_HL_NUMBER(entry1->operand);
    SBC_HL_DE();
}
void SubVariableNumber(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    return SubChainAnsNumber(entry1, entry2);
}
void SubVariableVariable(element_t *entry1, element_t *entry2) {
    LD_HL_IND_IX_OFF(entry1->operand);
    return SubChainAnsVariable(entry1, entry2);
}
void SubVariableFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_DE, NO_PUSH);
    LD_HL_IND_IX_OFF(entry1->operand);
}
void SubVariableChainAns(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    LD_HL_IND_IX_OFF(entry1->operand);
    SBC_HL_DE();
}
void SubFunctionNumber(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    SubChainAnsNumber(entry1, entry2);
}
void SubFunctionVariable(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NO_PUSH);
    SubChainAnsVariable(entry1, entry2);
}
void SubFunctionFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NEED_PUSH);
    SBC_HL_DE();
}
void SubFunctionChainAns(element_t *entry1, element_t *entry2) {
    EX_DE_HL();
    insertFunctionReturn(entry1->operand, OUTPUT_IN_HL, NEED_PUSH);
    SBC_HL_DE();
}
void SubChainAnsFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_DE, NEED_PUSH);
    SBC_HL_DE();
}
void SubChainPushNumber(element_t *entry1, element_t *entry2) {
    POP_HL();
    SubChainAnsNumber(entry1, entry2);
}
void SubChainPushVariable(element_t *entry1, element_t *entry2) {
    POP_HL();
    SubChainAnsVariable(entry1, entry2);
}
void SubChainPushFunction(element_t *entry1, element_t *entry2) {
    insertFunctionReturn(entry2->operand, OUTPUT_IN_DE, NO_PUSH);
    POP_HL();
    SBC_HL_DE();
}
void SubChainPushChainAns(element_t *entry1, element_t *entry2) {
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


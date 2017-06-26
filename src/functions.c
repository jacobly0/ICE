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
#include "functions.h"

uint8_t parseFunction(uint24_t index) {
    const uint8_t *outputStack = (uint8_t*)0xD62C00;
    element_t *outputPtr = (element_t*)outputStack;
    element_t *outputPrev, *outputPrevPrev;
    uint8_t function, amountOfArguments, tempType;
    uint24_t output;
    
    outputPrev        = &outputPtr[getIndexOffset(-2)];
    outputPrevPrev    = &outputPtr[getIndexOffset(-3)];
    output            = (&outputPtr[index])->operand;
    function          = (uint8_t)output;
    amountOfArguments = (uint8_t)(output >> 8);
    
    dbg_Debugger();
    
    // Dummy thing, it's great, it's true! :D
    switch (function) {
        case tNot:
            switch (outputPrev->type) {
                case TYPE_VARIABLE:
                    LD_HL_IND_IX_OFF(outputPrev->operand);
                    break;
                case TYPE_FUNCTION_RETURN:
                    insertFunctionReturn(outputPrev->operand, OUTPUT_IN_HL, NO_PUSH);
                case TYPE_CHAIN_ANS:
                    break;
                default:
                    return E_ICE_ERROR;
            }
            LD_DE_IMM(-1);
            ADD_HL_DE();
            SBC_HL_HL();
            INC_HL();
            break;
        case tMin:
        case tMax:
        case tMean:
            tempType = outputPrev->type;

            switch (outputPrevPrev->type) {
                case TYPE_NUMBER:
                    if (tempType == TYPE_VARIABLE) {
                        LD_HL_IND_IX_OFF(outputPrev->operand);
                    } else if (tempType == TYPE_FUNCTION_RETURN) {
                        insertFunctionReturn(outputPrev->operand, OUTPUT_IN_HL, NO_PUSH);
                    } else if (tempType != TYPE_CHAIN_ANS) {
                        return E_ICE_ERROR;
                    }
                    
                    if (expr.outputRegister == OutputRegisterHL) {
                        LD_DE_IMM(outputPrevPrev->operand);
                    } else {
                        LD_HL_IMM(outputPrevPrev->operand);
                    }
                    break;
                case TYPE_VARIABLE:
                    if (tempType == TYPE_NUMBER) {
                        LD_HL_NUMBER(outputPrev->operand);
                    } else if (tempType == TYPE_VARIABLE) {
                        LD_HL_IND_IX_OFF(outputPrev->operand);
                        if (outputPrev->operand == outputPrevPrev->operand) {
                            return VALID;
                        }
                    } else if (tempType == TYPE_FUNCTION_RETURN) {
                        insertFunctionReturn(outputPrev->operand, OUTPUT_IN_HL, NO_PUSH);
                    } else if (tempType != TYPE_CHAIN_ANS) {
                        return E_ICE_ERROR;
                    }
                    
                    if (expr.outputRegister == OutputRegisterHL) {
                        LD_DE_IND_IX_OFF(outputPrevPrev->operand);
                    } else {
                        LD_HL_IND_IX_OFF(outputPrevPrev->operand);
                    }
                    break;
                case TYPE_FUNCTION_RETURN:
                    if (tempType == TYPE_NUMBER) {
                        insertFunctionReturn(outputPrevPrev->operand, OUTPUT_IN_HL, NO_PUSH);
                        LD_DE_IMM(outputPrev->operand);
                    } else if (tempType == TYPE_VARIABLE) {
                        insertFunctionReturn(outputPrevPrev->operand, OUTPUT_IN_HL, NO_PUSH);
                    } else if (tempType == TYPE_FUNCTION) {
                        insertFunctionReturn(outputPrev->operand, OUTPUT_IN_DE, NO_PUSH);
                        insertFunctionReturn(outputPrevPrev->operand, OUTPUT_IN_HL, NEED_PUSH);
                    } else if (tempType == TYPE_CHAIN_ANS) {
                        if (expr.outputRegister == OutputRegisterHL) {
                            insertFunctionReturn(outputPrevPrev->operand, OUTPUT_IN_DE, NEED_PUSH);
                        } else {
                            insertFunctionReturn(outputPrevPrev->operand, OUTPUT_IN_HL, NEED_PUSH);
                        }
                    }
                    break;
                case TYPE_CHAIN_ANS:
                    switch (outputPrev->type) {
                        case TYPE_NUMBER:
                        case TYPE_VARIABLE:
                        case TYPE_FUNCTION_RETURN:
                            break;
                    }
                    break;
                case TYPE_CHAIN_PUSH:
                    if (outputPrev->type != TYPE_CHAIN_ANS) {
                        return E_ICE_ERROR;
                    }
                    
                    if (expr.outputRegister == OutputRegisterHL) {
                        POP_DE();
                    } else {
                        POP_HL();
                    }
            }
            
            if (function == tMean) {
                ProgramPtrToOffsetStack();
        
                // We need to add the mean routine to the data section
                if (!ice.usedAlreadyMean) {
                    ice.MeanAddr = (uint24_t)ice.programDataPtr;
                    memcpy(ice.programDataPtr, MeanRoutine, 24);
                    ice.programDataPtr += 24;
                    ice.usedAlreadyMean = true;
                }
                CALL(ice.MeanAddr);
            } else {
                OR_A_A();
                SBC_HL_DE();
                ADD_HL_DE();
                if (function == tMin) {
                    JR_C(1);
                } else {
                    JR_NC(1);
                }
                EX_DE_HL();
            }
            break;
        case tSqrt:
            switch (outputPrev->type) {
                case TYPE_VARIABLE:
                    LD_HL_IND_IX_OFF(outputPrev->operand);
                    break;
                case TYPE_FUNCTION_RETURN:
                    insertFunctionReturn(outputPrev->operand, OUTPUT_IN_HL, NO_PUSH);
                case TYPE_CHAIN_ANS:
                    break;
                default:
                    return E_ICE_ERROR;
            }
            
            ProgramPtrToOffsetStack();
        
            // We need to add the sqrt routine to the data section
            if (!ice.usedAlreadySqrt) {
                ice.SqrtAddr = (uint24_t)ice.programDataPtr;
                memcpy(ice.programDataPtr, SqrtRoutine, 45);
                ice.programDataPtr += 45;
                ice.usedAlreadySqrt = true;
            }
            CALL(ice.SqrtAddr);
            break;
        case tDet:
            // TODO
            OR_A_A();
            break;
        default:
            return E_ICE_ERROR;
    }
    
    return VALID;
}

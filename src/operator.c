#include "parse.h"
#include "operator.h"

#include "main.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "output.h"

#ifndef COMPUTER_ICE
#include <debug.h>
#endif

#include <tice.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void (*operatorFunctions[224])(void);
extern void (*operatorChainPushChainAnsFunctions[14])(void);
const char operators[] = {tStore, tAnd, tXor, tOr, tEQ, tLT, tGT, tLE, tGE, tNE, tMul, tDiv, tAdd, tSub};
const uint8_t operatorPrecedence[] = {0, 1, 2, 2, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4};
const uint8_t operatorPrecedence2[] = {6, 1, 2, 2, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4};

static element_t *entry1;
static element_t *entry2;
static uint24_t entry1_operand;
static uint24_t entry2_operand;
static uint8_t oper;

#define SIZEOF_KEYPAD_DATA 18
#define SIZEOF_RAND_DATA 54

#ifdef COMPUTER_ICE
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(And, "src/asm/and.bin");
INCBIN(Or, "src/asm/or.bin");
INCBIN(Xor, "src/asm/xor.bin");
INCBIN(Rand, "src/asm/rand.bin");
INCBIN(Keypad, "src/asm/keypad.bin");
#endif

#ifdef COMPUTER_ICE
static uint8_t clz(uint24_t x) {
    uint8_t n = 0;
    if (!x) {
        return 24;
    }
    while (!(x & (1 << 23))) {
        n++;
        x <<= 1;
    }
    return n;
}

void MultWithNumber(uint24_t num, uint8_t *programPtr) {
    (void)programPtr;
    unsigned int bit;
    uint8_t po2 = !(num & (num - 1));
    if (24 - clz(num) + __builtin_popcount(num) - 3 * po2 < 10) {
        if(!po2) {
            PUSH_HL();
            POP_DE();
        }
        for (bit = 1 << (23 - clz(num)); bit; bit >>= 1) {
            ADD_HL_HL();
            if(num & bit) {
                ADD_HL_DE();
            }
        }
    } else if (num < 0x100) {
        CALL(0x00003E | num << 8);
        output(uint24_t, 0x150);
    } else {
        LD_BC_IMM(num);
        CALL(0x154);
    }
}
#endif

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

static void getEntryOperands() {
    entry1_operand = entry1->operand;
    entry2_operand = entry2->operand;
}

static void swapEntries() {
    element_t *temp;
    temp = entry1;
    entry1 = entry2;
    entry2 = temp;
    getEntryOperands();
}

uint8_t parseOperator(element_t *outputPrevPrev, element_t *outputPrev, element_t *outputCurr) {
    uint8_t typeMasked1 = outputPrevPrev->type;
    uint8_t typeMasked2 = outputPrev->type;
    oper = (uint8_t)outputCurr->operand;
    
    // Only call the function if both types are valid
    if ((typeMasked1 == typeMasked2 &&
            (typeMasked1 == TYPE_NUMBER || typeMasked1 == TYPE_CHAIN_ANS)
        ) ||
        (oper == tStore &&
            ((typeMasked2 != TYPE_VARIABLE && typeMasked2 != TYPE_OS_STRING) ||
                (typeMasked2 != TYPE_OS_STRING || typeMasked1 < TYPE_STRING)
            )
        ) ||
        (typeMasked2 == TYPE_CHAIN_PUSH) ||
        (typeMasked1 >= TYPE_STRING && typeMasked2 >= TYPE_STRING && oper != tAdd && oper != tStore)
    ) {
        return E_SYNTAX;
    }
    
    expr.outputRegister2 = OutputRegisterHL;
    expr.AnsSetZeroFlag = false;
    expr.AnsSetZeroFlagReversed = false;
    expr.AnsSetCarryFlag = false;
    expr.AnsSetCarryFlagReversed = false;
    
    // This should not happen
    if (typeMasked1 == TYPE_CHAIN_PUSH && oper != tStore) {
        if (typeMasked2 != TYPE_CHAIN_ANS) {
            return E_ICE_ERROR;
        }
        // Call the right CHAIN_PUSH | CHAIN_ANS function
        (*operatorChainPushChainAnsFunctions[getIndexOfOperator(oper) - 1])();
        expr.outputRegister = expr.outputRegister2;
        return VALID;
    }
    
    // If you have something like "A or 1", the output is always 1, so we can remove the "ld hl, (A)"
    ice.programPtrBackup = ice.programPtr;
    
    // Get the right arguments
    entry1 = outputPrevPrev;
    entry2 = outputPrev;
    getEntryOperands();

    // Swap operands for optimizations
    if (oper == tLE || oper == tLT) {
        swapEntries();
    }
    
    if (typeMasked1 < TYPE_STRING) {
        // Call the right function!
        (*operatorFunctions[((getIndexOfOperator(oper) - 1) * 16) + (typeMasked1 * 4) + typeMasked2])();
    } else {
        if (oper == tAdd) {
            AddStringString();
        } else {
            StoStringString();
        }
    }
    expr.outputRegister = expr.outputRegister2;
    return VALID;
}

void insertFunctionReturn(uint24_t function, uint8_t outputRegister, bool needPush) {
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
        ProgramPtrToOffsetStack();
        
        // We need to add the rand routine to the data section
        if (!ice.usedAlreadyRand) {
            ice.randAddr = (uintptr_t)ice.programDataPtr;
            memcpy(ice.programDataPtr, RandData, SIZEOF_RAND_DATA);
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
    }
    
    else {
        // Check if the getKey has a fast direct key argument; if so, the second byte is 1
        if ((uint8_t)(function >> 8)) {
            uint8_t key = function >> 8;
            /* This is the same as 
                ((key-1)/8 & 7) * 2 = 
                (key-1)/4 & (7*2) = 
                (key-1) >> 2 & 14 
            */
            uint8_t keyAddress = 0x1E - (((key-1) >> 2) & 14);
            uint8_t keyBit = 1;
            
            // Get the right bit for the keypress
            if ((key - 1) & 7) {
                uint8_t a;
                for (a = 0; a < ((key - 1) & 7); a++) {
                    keyBit = keyBit << 1;
                }
            }
            
            LD_B(keyAddress);
            LD_C(keyBit);
            
            // Check if we need to preserve HL
            if (NEED_PUSH && outputRegister != OUTPUT_IN_HL) {
                PUSH_HL();
            }
            
            
            // Store the pointer to the call to the stack, to replace later
            ProgramPtrToOffsetStack();
            
            // We need to add the getKeyFast routine to the data section
            if (!ice.usedAlreadyGetKeyFast) {
                ice.getKeyFastAddr = (uintptr_t)ice.programDataPtr;
                memcpy(ice.programDataPtr, KeypadData, SIZEOF_KEYPAD_DATA);
                ice.programDataPtr += SIZEOF_KEYPAD_DATA;
                ice.usedAlreadyGetKeyFast = true;
            }
            
            CALL(ice.getKeyFastAddr);
            
            // Store the keypress in the right register
            if (outputRegister == OUTPUT_IN_DE) {
                EX_DE_HL();
            } else if (outputRegister == OUTPUT_IN_BC) {
                PUSH_HL();
                POP_BC();
            }
            
            // Check if we need to preserve HL
            if (NEED_PUSH && outputRegister != OUTPUT_IN_HL) {
                POP_HL();
            }
            
            // This routine sets or resets the zero flag, which can be used to optimize conditions
            expr.AnsSetZeroFlag = true;
            expr.ZeroCarryFlagRemoveAmountOfBytes = 0;
        }
        
        // Else, a standalone "getKey"
        else {
            // The key should be in HL
            if (outputRegister == OUTPUT_IN_HL) {
                CALL(_os_GetCSC);
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

void OperatorError(void) {
    // This *should* never be triggered
    displayError(E_ICE_ERROR);
}
void StoChainAnsVariable(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        LD_IX_OFF_IND_HL(entry2_operand);
    } else {
        LD_IX_OFF_IND_DE(entry2_operand);
    }
}
void StoNumberVariable(void) {
    LD_HL_NUMBER(entry1_operand);
    StoChainAnsVariable();
}
void StoVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    StoChainAnsVariable();
}
void StoFunctionVariable(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    StoChainAnsVariable();
}
void StoStringString(void) {
    if (entry1->type == TYPE_STRING && entry1_operand != TempString1 && entry1_operand != TempString2) {
        ProgramPtrToOffsetStack();
    }
    LD_HL_IMM(entry1_operand);
    PUSH_HL();
    CALL(__strlen);
    INC_HL();
    PUSH_HL();
    POP_BC();
    POP_HL();
    LD_DE_IMM(entry2_operand);
    LDIR();
}
void AndInsert(void) {
    if (oper == tOr) {
        memcpy(ice.programPtr, OrData, 10);
        ice.programPtr += 10;
    } else if (oper == tAnd) {
        memcpy(ice.programPtr, AndData, 11);
        ice.programPtr += 11;
    } else {
        memcpy(ice.programPtr, XorData, 13);
        ice.programPtr += 13;
    }
    expr.AnsSetCarryFlag = true;
    expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
}
void AndChainAnsNumber(void) {
    if (oper == tXor) {
        if (expr.outputRegister == OutputRegisterHL) {
            LD_DE_IMM(-1);
        } else {
            LD_HL_IMM(-1);
        }
        ADD_HL_DE();
        expr.ZeroCarryFlagRemoveAmountOfBytes = 0;
        if (!entry2_operand) {
            CCF();
            expr.ZeroCarryFlagRemoveAmountOfBytes++;
            expr.AnsSetCarryFlagReversed = true;
        }
        SBC_HL_HL();
        INC_HL();
        expr.AnsSetCarryFlag = true;
        expr.ZeroCarryFlagRemoveAmountOfBytes += 3;
    } else if (oper == tAnd) {
        if (!entry2_operand) {
            ice.programPtr = ice.programPtrBackup;
            LD_HL_NUMBER(0);
        } else {
            if (expr.outputRegister == OutputRegisterHL) {
                LD_DE_IMM(-1);
            } else {
                LD_HL_IMM(-1);
            }
            ADD_HL_DE();
            CCF();
            SBC_HL_HL();
            INC_HL();
            expr.AnsSetCarryFlag = true;
            expr.ZeroCarryFlagRemoveAmountOfBytes = 4;
            expr.AnsSetCarryFlagReversed = true;
        }
    } else {
        if (!entry2_operand) {
            if (expr.outputRegister == OutputRegisterHL) {
                LD_DE_IMM(-1);
            } else {
                LD_HL_IMM(-1);
            }
            ADD_HL_DE();
            CCF();
            SBC_HL_HL();
            INC_HL();
            expr.AnsSetCarryFlag = true;
            expr.ZeroCarryFlagRemoveAmountOfBytes = 4;
            expr.AnsSetCarryFlagReversed = true;
        } else {
            ice.programPtr = ice.programPtrBackup;
            LD_HL_NUMBER(1);
        }
    }
}
void AndChainAnsVariable(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        LD_DE_IND_IX_OFF(entry2_operand);
    } else {
        LD_HL_IND_IX_OFF(entry2_operand);
    }
    AndInsert();
}
void AndChainAnsFunction(void) {
    insertFunctionReturn(entry2_operand, (expr.outputRegister == OutputRegisterHL) ? OUTPUT_IN_DE : OUTPUT_IN_HL, NEED_PUSH);
    AndInsert();
}
void AndFunctionNumber(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    AndChainAnsNumber();
}
void AndVariableNumber(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    AndChainAnsNumber();
}
void AndFunctionVariable(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    AndChainAnsVariable();
}
void AndNumberVariable(void) {
    swapEntries();
    AndVariableNumber();
}
void AndNumberFunction(void) {
    swapEntries();
    AndFunctionNumber();
}
void AndNumberChainAns(void) {
    swapEntries();
    AndChainAnsNumber();
}
void AndVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    AndChainAnsVariable();
}
void AndVariableFunction(void) {
    swapEntries();
    AndFunctionVariable();
}
void AndVariableChainAns() {
    swapEntries();
    AndChainAnsVariable();
}
void AndFunctionFunction(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry2_operand, OUTPUT_IN_HL, NEED_PUSH);
    AndInsert();
}
void AndFunctionChainAns(void) {
    swapEntries();
    AndChainAnsFunction();
}
void AndChainPushChainAns(void) {
    POP_DE();
    AndInsert();
}

#define XorNumberVariable    AndNumberVariable   
#define XorNumberFunction    AndNumberFunction   
#define XorNumberChainAns    AndNumberChainAns   
#define XorVariableNumber    AndVariableNumber   
#define XorVariableVariable  AndVariableVariable 
#define XorVariableFunction  AndVariableFunction 
#define XorVariableChainAns  AndVariableChainAns 
#define XorFunctionNumber    AndFunctionNumber   
#define XorFunctionVariable  AndFunctionVariable 
#define XorFunctionFunction  AndFunctionFunction 
#define XorFunctionChainAns  AndFunctionChainAns 
#define XorChainAnsNumber    AndChainAnsNumber   
#define XorChainAnsVariable  AndChainAnsVariable 
#define XorChainAnsFunction  AndChainAnsFunction 
#define XorChainPushNumber   AndChainPushNumber  
#define XorChainPushVariable AndChainPushVariable
#define XorChainPushFunction AndChainPushFunction
#define XorChainPushChainAns AndChainPushChainAns

#define OrNumberVariable    AndNumberVariable   
#define OrNumberFunction    AndNumberFunction   
#define OrNumberChainAns    AndNumberChainAns   
#define OrVariableNumber    AndVariableNumber   
#define OrVariableVariable  AndVariableVariable 
#define OrVariableFunction  AndVariableFunction 
#define OrVariableChainAns  AndVariableChainAns 
#define OrFunctionNumber    AndFunctionNumber   
#define OrFunctionVariable  AndFunctionVariable 
#define OrFunctionFunction  AndFunctionFunction 
#define OrFunctionChainAns  AndFunctionChainAns 
#define OrChainAnsNumber    AndChainAnsNumber   
#define OrChainAnsVariable  AndChainAnsVariable 
#define OrChainAnsFunction  AndChainAnsFunction 
#define OrChainPushNumber   AndChainPushNumber  
#define OrChainPushVariable AndChainPushVariable
#define OrChainPushFunction AndChainPushFunction
#define OrChainPushChainAns AndChainPushChainAns

void EQInsert() {
    OR_A_A();
    SBC_HL_DE();
    LD_HL_IMM(0);
    if (oper == tEQ) {
        JR_NZ(1);
        expr.AnsSetZeroFlagReversed = true;
    } else {
        JR_Z(1);
    }
    INC_HL();
    expr.AnsSetZeroFlag = true;
    expr.ZeroCarryFlagRemoveAmountOfBytes = 7;
}
void EQChainAnsNumber(void) {
    uint24_t number = entry2_operand;
    if (number && number < 6) {
        do {
            if (expr.outputRegister == OutputRegisterHL) {
                DEC_HL();
            } else {
                DEC_DE();
            }
        } while (--number);
    }
    if (!number) {
        if (expr.outputRegister == OutputRegisterHL) {
            LD_DE_IMM(-1);
        } else {
            LD_HL_IMM(-1);
        }
        ADD_HL_DE();
        expr.ZeroCarryFlagRemoveAmountOfBytes = 0;
        if (oper == tNE) {
            CCF();
            expr.ZeroCarryFlagRemoveAmountOfBytes++;
            expr.AnsSetCarryFlagReversed = true;
        }
        SBC_HL_HL();
        INC_HL();
        expr.AnsSetCarryFlag = true;
        expr.ZeroCarryFlagRemoveAmountOfBytes += 3;
    } else {
        if (expr.outputRegister == OutputRegisterHL) {
            LD_DE_IMM(number);
        } else {
            LD_HL_IMM(number);
        }
        EQInsert();
    }
}
void EQChainAnsFunction(void) {
    insertFunctionReturn(entry2_operand, (expr.outputRegister == OutputRegisterHL) ? OUTPUT_IN_DE : OUTPUT_IN_HL, NEED_PUSH);
    EQInsert();
}
void EQVariableNumber(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    EQChainAnsNumber();
}
void EQChainAnsVariable(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        LD_DE_IND_IX_OFF(entry2_operand);
    } else {
        LD_HL_IND_IX_OFF(entry2_operand);
    }
    EQInsert();
}
void EQFunctionNumber(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    EQChainAnsNumber();
}
void EQFunctionVariable(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    EQChainAnsVariable();
}
void EQNumberVariable(void) {
    swapEntries();
    EQVariableNumber();
}
void EQNumberFunction(void) {
    swapEntries();
    EQFunctionNumber();
}
void EQNumberChainAns(void) {
    swapEntries();
    EQChainAnsNumber();
}
void EQVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    EQChainAnsVariable();
}
void EQVariableFunction(void) {
    swapEntries();
    EQFunctionVariable();
}
void EQVariableChainAns(void) {
    swapEntries();
    EQChainAnsVariable();
}
void EQFunctionFunction(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry2_operand, OUTPUT_IN_HL, NEED_PUSH);
    EQInsert();
}
void EQFunctionChainAns(void) {
    swapEntries();
    EQChainAnsFunction();
}
void EQChainPushChainAns(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        POP_DE();
    } else {
        POP_HL();
    }
    EQInsert();
}
void GEInsert() {
    if (oper == tGE || oper == tLE) {
        OR_A_A();
    } else {
        SCF();
    }
    SBC_HL_DE();
    SBC_HL_HL();
    INC_HL();
    expr.AnsSetCarryFlag = true;
    expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
}
void GEChainAnsNumber(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        LD_DE_IMM(entry2_operand);
    } else {
        LD_HL_IMM(entry2_operand);
    }
    GEInsert();
}
void GEChainAnsVariable(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        LD_DE_IND_IX_OFF(entry2_operand);
    } else {
        LD_HL_IND_IX_OFF(entry2_operand);
    }
    GEInsert();
}
void GENumberVariable(void) {
    LD_HL_NUMBER(entry1_operand);
    GEChainAnsVariable();
}
void GENumberFunction(void) {
    insertFunctionReturn(entry2_operand, OUTPUT_IN_DE, NO_PUSH);
    LD_HL_NUMBER(entry1_operand);
    GEInsert();
}
void GENumberChainAns(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        LD_DE_IMM(entry1_operand);
    } else {
        LD_HL_NUMBER(entry1_operand);
    }
    GEInsert();
}
void GEVariableNumber(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    GEChainAnsNumber();
}
void GEVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    GEChainAnsVariable();
}
void GEVariableFunction(void) {
    insertFunctionReturn(entry2_operand, OUTPUT_IN_DE, NO_PUSH);
    LD_HL_IND_IX_OFF(entry1_operand);
    GEInsert();
}
void GEVariableChainAns(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        EX_DE_HL();
    }
    LD_HL_IND_IX_OFF(entry1_operand);
    GEInsert();
}
void GEFunctionNumber(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    GEChainAnsNumber();
}
void GEFunctionVariable(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    GEChainAnsVariable();
}
void GEFunctionFunction(void) {
    insertFunctionReturn(entry2_operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NEED_PUSH);
    GEInsert();
}
void GEFunctionChainAns(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        EX_DE_HL();
    }
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NEED_PUSH);
}
void GEChainAnsFunction(void) {
    if (expr.outputRegister != OutputRegisterHL) {
        EX_DE_HL();
    }
    insertFunctionReturn(entry2_operand, OUTPUT_IN_DE, NEED_PUSH);
    GEInsert();
}
void GEChainPushChainAns(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        EX_DE_HL();
    }
    POP_HL();
    GEInsert();
}

#define GTNumberVariable    GENumberVariable   
#define GTNumberFunction    GENumberFunction   
#define GTNumberChainAns    GENumberChainAns   
#define GTVariableNumber    GEVariableNumber   
#define GTVariableVariable  GEVariableVariable 
#define GTVariableFunction  GEVariableFunction 
#define GTVariableChainAns  GEVariableChainAns 
#define GTFunctionNumber    GEFunctionNumber   
#define GTFunctionVariable  GEFunctionVariable 
#define GTFunctionFunction  GEFunctionFunction 
#define GTFunctionChainAns  GEFunctionChainAns 
#define GTChainAnsNumber    GEChainAnsNumber   
#define GTChainAnsVariable  GEChainAnsVariable 
#define GTChainAnsFunction  GEChainAnsFunction 
#define GTChainPushChainAns GEChainPushChainAns

#define LTNumberVariable   GTVariableNumber
#define LTNumberFunction   GTFunctionNumber
#define LTNumberChainAns   GTChainAnsNumber
#define LTVariableNumber   GTNumberVariable
#define LTVariableVariable GTVariableVariable
#define LTVariableFunction GTFunctionVariable
#define LTVariableChainAns GTChainAnsVariable
#define LTFunctionNumber   GTNumberFunction
#define LTFunctionVariable GTVariableFunction
#define LTFunctionFunction GTFunctionFunction
#define LTFunctionChainAns GTChainAnsFunction
#define LTChainAnsNumber   GTNumberChainAns
#define LTChainAnsVariable GTVariableChainAns
#define LTChainAnsFunction GTFunctionChainAns

void LTChainPushChainAns(void) {
    if (expr.outputRegister != OutputRegisterHL) {
        EX_DE_HL();
    }
    POP_DE();
    SCF();
    SBC_HL_DE();
    SBC_HL_HL();
    INC_HL();
    expr.AnsSetCarryFlag = true;
    expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
}

#define LENumberVariable   GEVariableNumber
#define LENumberFunction   GEFunctionNumber
#define LENumberChainAns   GEChainAnsNumber
#define LEVariableNumber   GENumberVariable
#define LEVariableVariable GEVariableVariable
#define LEVariableFunction GEFunctionVariable
#define LEVariableChainAns GEChainAnsVariable
#define LEFunctionNumber   GENumberFunction
#define LEFunctionVariable GEVariableFunction
#define LEFunctionFunction GEFunctionFunction
#define LEFunctionChainAns GEChainAnsFunction
#define LEChainAnsNumber   GENumberChainAns
#define LEChainAnsVariable GEVariableChainAns
#define LEChainAnsFunction GEFunctionChainAns

void LEChainPushChainAns(void) {
    if (expr.outputRegister != OutputRegisterHL) {
        EX_DE_HL();
    }
    POP_DE();
    OR_A_A();
    SBC_HL_DE();
    SBC_HL_HL();
    INC_HL();
    expr.AnsSetCarryFlag = true;
    expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
}

#define NENumberVariable    EQVariableNumber
#define NENumberFunction    EQFunctionNumber
#define NENumberChainAns    EQChainAnsNumber
#define NEVariableNumber    EQNumberVariable
#define NEVariableVariable  EQVariableVariable
#define NEVariableFunction  EQFunctionVariable
#define NEVariableChainAns  EQChainAnsVariable
#define NEFunctionNumber    EQNumberFunction
#define NEFunctionVariable  EQVariableFunction
#define NEFunctionFunction  EQFunctionFunction
#define NEFunctionChainAns  EQChainAnsFunction
#define NEChainAnsNumber    EQNumberChainAns
#define NEChainAnsVariable  EQVariableChainAns
#define NEChainAnsFunction  EQFunctionChainAns
#define NEChainPushChainAns EQChainPushChainAns

void MulChainAnsNumber(void) {
    uint24_t number = entry2_operand;
    if (number == 0) {
        ice.programPtr = ice.programPtrBackup;
        LD_HL_NUMBER(0);
    } else {
        if (expr.outputRegister != OutputRegisterHL) {
            EX_DE_HL();
        }
        MultWithNumber(number, (uint8_t*)&ice.programPtr);
    }
}
void MulVariableNumber(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    MulChainAnsNumber();
}
void MulFunctionNumber(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    MulChainAnsNumber();
}
void MulChainAnsVariable(void) {
    LD_BC_IND_IX_OFF(entry2_operand);
    CALL(__imuls);
}
void MulFunctionVariable(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    MulChainAnsVariable();
}
void MulNumberVariable(void) {
    swapEntries();
    MulVariableNumber();
}
void MulNumberFunction(void) {
    swapEntries();
    MulFunctionNumber();
}
void MulNumberChainAns(void) {
    swapEntries();
    MulChainAnsNumber();
}
void MulVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    MulChainAnsVariable();
}
void MulVariableFunction(void) {
    swapEntries();
    MulFunctionVariable();
}
void MulVariableChainAns(void) {
    swapEntries();
    MulChainAnsVariable();
}
void MulFunctionFunction(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    PUSH_HL();
    insertFunctionReturn(entry2_operand, OUTPUT_IN_HL, NO_PUSH);
    POP_BC();
    CALL(__imuls);
}
void MulChainAnsFunction(void) {
    if (expr.outputRegister != OutputRegisterHL) {
        EX_DE_HL();
    }
    insertFunctionReturn(entry2_operand, OUTPUT_IN_BC, NEED_PUSH);
    CALL(__imuls);
}
void MulFunctionChainAns(void) {
    swapEntries();
    MulChainAnsFunction();
}
void MulChainPushChainAns(void) {
    if (expr.outputRegister != OutputRegisterHL) {
        EX_DE_HL();
    }
    POP_BC();
    CALL(__imuls);
}
void DivChainAnsNumber(void) {
    if (expr.outputRegister != OutputRegisterHL) {
        EX_DE_HL();
    }
    LD_BC_IMM(entry2_operand);
    CALL(__idvrmu);
    expr.outputRegister2 = OutputRegisterDE;
}
void DivChainAnsVariable(void) {
    if (expr.outputRegister != OutputRegisterHL) {
        EX_DE_HL();
    }
    LD_BC_IND_IX_OFF(entry2_operand);
    CALL(__idvrmu);
    expr.outputRegister2 = OutputRegisterDE;
}
void DivNumberVariable(void) {
    LD_HL_NUMBER(entry1_operand);
    DivChainAnsVariable();
}
void DivNumberFunction(void) {
    insertFunctionReturn(entry2_operand, OUTPUT_IN_BC, NO_PUSH);
    LD_HL_NUMBER(entry1_operand);
    CALL(__idvrmu);
    expr.outputRegister2 = OutputRegisterDE;
}
void DivNumberChainAns(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        PUSH_HL();
    } else {
        PUSH_DE();
    }
    POP_BC();
    LD_HL_NUMBER(entry1_operand);
    CALL(__idvrmu);
    expr.outputRegister2 = OutputRegisterDE;
}
void DivVariableNumber(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    DivChainAnsNumber();
}
void DivVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    DivChainAnsVariable();
}
void DivVariableFunction(void) {
    insertFunctionReturn(entry2_operand, OUTPUT_IN_BC, NO_PUSH);
    LD_HL_IND_IX_OFF(entry1_operand);
    CALL(__idvrmu);
    expr.outputRegister2 = OutputRegisterDE;
}
void DivVariableChainAns(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        PUSH_HL();
    } else {
        PUSH_DE();
    }
    POP_BC();
    LD_HL_IND_IX_OFF(entry1_operand);
    CALL(__idvrmu);
    expr.outputRegister2 = OutputRegisterDE;
}
void DivFunctionNumber(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    DivChainAnsNumber();
}
void DivFunctionVariable(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    DivChainAnsVariable();
}
void DivFunctionFunction(void) {
    insertFunctionReturn(entry2_operand, OUTPUT_IN_HL, NO_PUSH);
    PUSH_HL();
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    POP_BC();
    CALL(__idvrmu);
    expr.outputRegister2 = OutputRegisterDE;
}
void DivFunctionChainAns(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        PUSH_HL();
    } else {
        PUSH_DE();
    }
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    POP_BC();
    CALL(__idvrmu);
    expr.outputRegister2 = OutputRegisterDE;
}
void DivChainAnsFunction(void) {
    if (expr.outputRegister != OutputRegisterHL) {
        EX_DE_HL();
    }
    insertFunctionReturn(entry2_operand, OUTPUT_IN_BC, NEED_PUSH);
    CALL(__idvrmu);
    expr.outputRegister2 = OutputRegisterDE;
}
void DivChainPushChainAns(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        PUSH_HL();
    } else {
        PUSH_DE();
    }
    POP_BC();
    POP_HL();
    CALL(__idvrmu);
    expr.outputRegister2 = OutputRegisterDE;
}
void AddChainAnsNumber(void) {
    uint24_t number = entry2_operand;
    if (number < 5) {
        uint8_t a;
        for (a = 0; a < (uint8_t)number; a++) {
            if (expr.outputRegister == OutputRegisterHL) {
                INC_HL();
            } else {
                INC_DE();
            }
        }
    } else {
        if (expr.outputRegister == OutputRegisterHL) {
            LD_DE_IMM(number);
        } else {
            LD_HL_IMM(number);
        }
        ADD_HL_DE();
    }
}
void AddVariableNumber(void) {
    if (entry2_operand < 128 && entry2_operand > 3) {
        LD_IY_IND_IX_OFF(entry1_operand);
        LEA_HL_IY_OFF(entry2_operand);
        ice.modifiedIY = true;
    } else {
        LD_HL_IND_IX_OFF(entry1_operand);
        AddChainAnsNumber();
    }
}
void AddFunctionNumber(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    AddChainAnsNumber();
}
void AddChainAnsVariable(void) {
    LD_DE_IND_IX_OFF(entry2_operand);
    ADD_HL_DE();
}
void AddFunctionVariable(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    AddChainAnsVariable();
}
void AddNumberVariable(void) {
    swapEntries();
    AddVariableNumber();
}
void AddNumberFunction(void) {
    swapEntries();
    AddFunctionNumber();
}
void AddNumberChainAns(void) {
    swapEntries();
    AddChainAnsNumber();
}
void AddVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    if (entry1_operand == entry2_operand) {
        ADD_HL_HL();
    } else {
        AddChainAnsVariable();
    }
}
void AddVariableFunction(void) {
    swapEntries();
    AddFunctionVariable();
}
void AddVariableChainAns(void) {
    swapEntries();
    AddChainAnsVariable();
}
void AddFunctionFunction(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry2_operand, OUTPUT_IN_HL, NEED_PUSH);
    ADD_HL_DE();
}
void AddChainAnsFunction(void) {
    insertFunctionReturn(entry2_operand, (expr.outputRegister == OutputRegisterHL) ? OUTPUT_IN_DE : OUTPUT_IN_HL, NEED_PUSH);
    ADD_HL_DE();
}
void AddFunctionChainAns(void) {
    swapEntries();
    AddChainAnsFunction();
}
void AddChainPushChainAns(void) {
    POP_DE();
    ADD_HL_DE();
}
void AddStringString(void) {
    /*
        Cases:
            <string1>+<string2>             --> strcpy(<TempString1>, <string1>) strcat(<TempString1>, <string2>)
            <string1>+<TempString1>         --> strcpy(<TempString2>, <string1>) strcat(<TempString2>, <TempString1>)
            <string1>+<TempString2>         --> strcpy(<TempString1>, <string1>) strcat(<TempString1>, <TempString2>)
            <TempString1>+<string2>         --> strcat(<TempString1>, <string2>)
            <TempString1>+<TempString2>     --> strcat(<TempString1>, <TempString2>)
            <TempString2>+<string2>         --> strcat(<TempString2>, <string2>)
            <TempString2>+<TempString1>     --> strcat(<TempString2>, <TempString1>)
            
        Output in TempString2 if:
            <TempString2>+X
            X+<TempString1>
    */
    
    if (entry1->type == TYPE_STRING) {
        ProgramPtrToOffsetStack();
        LD_HL_IMM(entry1_operand);
        PUSH_HL();
        if (entry2_operand == TempString1) {
            LD_HL_IMM(TempString2);
        } else {
            LD_HL_IMM(TempString1);
        }
        PUSH_HL();
        CALL(__strcpy);
        POP_DE();
        if (entry2->type == TYPE_STRING) {
            ProgramPtrToOffsetStack();
        }
        LD_HL_IMM(entry2_operand);
        EX_SP_HL();
        PUSH_DE();
        CALL(__strcat);
        POP_BC();
        POP_BC();
    } else {
        if (entry2->type == TYPE_STRING) {
            ProgramPtrToOffsetStack();
        }
        LD_HL_IMM(entry2_operand);
        PUSH_HL();
        LD_HL_IMM(entry1_operand);
        PUSH_HL();
        CALL(__strcat);
        POP_BC();
        POP_BC();
    }
}
void SubChainAnsNumber(void) {
    uint24_t number = entry2_operand;
    if (number < 5) {
        uint8_t a;
        for (a = 0; a < (uint8_t)number; a++) {
            if (expr.outputRegister == OutputRegisterHL) {
                DEC_HL();
            } else {
                DEC_DE();
            }
        }
    } else {
        if (expr.outputRegister == OutputRegisterHL) {
            LD_DE_IMM(0x1000000 - number);
        } else {
            LD_HL_IMM(0x1000000 - number);
        }
        ADD_HL_DE();
    }
}
void SubChainAnsVariable(void) {
    if (expr.outputRegister != OutputRegisterHL) {
        EX_DE_HL();
    }
    LD_DE_IND_IX_OFF(entry2_operand);
    OR_A_A();
    SBC_HL_DE();
}
void SubNumberVariable(void) {
    LD_HL_NUMBER(entry1_operand);
    SubChainAnsVariable();
}
void SubNumberFunction(void) {
    insertFunctionReturn(entry2_operand, OUTPUT_IN_DE, NO_PUSH);
    LD_HL_NUMBER(entry1_operand);
    OR_A_A();
    SBC_HL_DE();
}
void SubNumberChainAns(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        EX_DE_HL();
    }
    LD_HL_NUMBER(entry1_operand);
    OR_A_A();
    SBC_HL_DE();
}
void SubVariableNumber(void) {
    if (entry2_operand < 129 && entry2_operand > 3) {
        LD_IY_IND_IX_OFF(entry1_operand);
        LEA_HL_IY_OFF(-entry2_operand);
        ice.modifiedIY = true;
    } else {
        LD_HL_IND_IX_OFF(entry1_operand);
        SubChainAnsNumber();
    }
}
void SubVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    SubChainAnsVariable();
}
void SubVariableFunction(void) {
    insertFunctionReturn(entry2_operand, OUTPUT_IN_DE, NO_PUSH);
    LD_HL_IND_IX_OFF(entry1_operand);
}
void SubVariableChainAns(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        EX_DE_HL();
    }
    LD_HL_IND_IX_OFF(entry1_operand);
    OR_A_A();
    SBC_HL_DE();
}
void SubFunctionNumber(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    SubChainAnsNumber();
}
void SubFunctionVariable(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    SubChainAnsVariable();
}
void SubFunctionFunction(void) {
    insertFunctionReturn(entry2_operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NEED_PUSH);
    OR_A_A();
    SBC_HL_DE();
}
void SubFunctionChainAns(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        EX_DE_HL();
    }
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NEED_PUSH);
    OR_A_A();
    SBC_HL_DE();
}
void SubChainAnsFunction(void) {
    if (expr.outputRegister != OutputRegisterHL) {
        EX_DE_HL();
    }
    insertFunctionReturn(entry2_operand, OUTPUT_IN_DE, NEED_PUSH);
    OR_A_A();
    SBC_HL_DE();
}
void SubChainPushChainAns(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        EX_DE_HL();
    }
    POP_HL();
    OR_A_A();
    SBC_HL_DE();
}

void (*operatorChainPushChainAnsFunctions[14])(void) = {
    OperatorError,
    AndChainPushChainAns,
    XorChainPushChainAns,
    OrChainPushChainAns,
    EQChainPushChainAns,
    LTChainPushChainAns,
    GTChainPushChainAns,
    LEChainPushChainAns,
    GEChainPushChainAns,
    NEChainPushChainAns,
    MulChainPushChainAns,
    DivChainPushChainAns,
    AddChainPushChainAns,
    SubChainPushChainAns
};

void (*operatorFunctions[224])(void) = {
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
};

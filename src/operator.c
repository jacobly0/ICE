#include "defines.h"
#include "parse.h"
#include "operator.h"

#include "main.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "output.h"
#include "routines.h"
//#include "gfx/gfx_logos.h"

#ifdef COMPUTER_ICE
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(And, "src/asm/and.bin");
INCBIN(Or, "src/asm/or.bin");
INCBIN(Xor, "src/asm/xor.bin");
INCBIN(Rand, "src/asm/rand.bin");
INCBIN(Keypad, "src/asm/keypad.bin");
#endif

extern void (*operatorFunctions[224])(void);
extern void (*operatorChainPushChainAnsFunctions[14])(void);
const char operators[]              = {tStore, tAnd, tXor, tOr, tEQ, tLT, tGT, tLE, tGE, tNE, tMul, tDiv, tAdd, tSub};
const uint8_t operatorPrecedence[]  = {0, 2, 1, 1, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4};
const uint8_t operatorPrecedence2[] = {6, 2, 1, 1, 3, 3, 3, 3, 3, 3, 5, 5, 4, 4};
const uint8_t operatorCanSwap[]     = {0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0};        // Used for operators which can swap the operands, i.e. A*B = B*A

static element_t *entry1;
static element_t *entry2;
static uint24_t entry1_operand;
static uint24_t entry2_operand;
static uint8_t oper;

#define SIZEOF_KEYPAD_DATA 18
#define SIZEOF_RAND_DATA 54

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

void MultWithNumber(uint24_t num, uint8_t *programPtr, bool ChangeRegisters) {
    (void)programPtr;
    uint24_t bit;
    uint8_t po2 = !(num & (num - 1));
    
    if (24 - clz(num) + __builtin_popcount(num) - 2 * po2 < 10) {
        if(!po2) {
            if (!ChangeRegisters) {
                PUSH_HL();
                POP_DE();
            } else {
                PUSH_DE();
                POP_HL();
            }
        }
        for (bit = 1 << (22 - clz(num)); bit; bit >>= 1) {
            ADD_HL_HL();
            if(num & bit) {
                ADD_HL_DE();
            }
        }
    } else if (num < 0x100) {
        LD_A(num);
        CALL(__imul_b);
    } else {
        LD_BC_IMM(num);
        CALL(__imuls);
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
        default:
            return operand1 && operand2;
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
             (typeMasked2 == TYPE_OS_STRING && typeMasked1 < TYPE_STRING)
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

    // Swap operands for compiler optimizations
    if (oper == tLE || oper == tLT || (operatorCanSwap[getIndexOfOperator(oper) - 1] && 
         (typeMasked1 == TYPE_NUMBER || typeMasked2 == TYPE_CHAIN_ANS || 
           (typeMasked1 == TYPE_VARIABLE && typeMasked2 == TYPE_FUNCTION)
         ))
       ) {
        uint8_t temp = typeMasked1;
        typeMasked1 = typeMasked2;
        typeMasked2 = temp;
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
    
    // If the operator is /, the routine ALWAYS ends with call __idvrmu \ expr.outputRegister2 == OutputRegisterDE
    if (oper == tDiv) {
        CALL(__idvrmu);
        expr.outputRegister2 = OutputRegisterDE;
    }
    
    // If the operator is *, and both operands not a number, it always ends with call __imuls
    if (oper == tMul && typeMasked1 != TYPE_NUMBER && typeMasked2 != TYPE_NUMBER) {
        CALL(__imuls);
    }
    
    // If the operator is -, and the second operand not a number, it always ends with or a, a \ sbc hl, de
    if (oper == tSub && typeMasked2 != TYPE_NUMBER) {
        OR_A_A();
        SBC_HL_DE();
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

void LD_HL_STRING(uint24_t StringPtr) {
    if (StringPtr != TempString1 && StringPtr != TempString2) {
        ProgramPtrToOffsetStack();
    }
    LD_HL_IMM(StringPtr);
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
    LD_HL_STRING(entry1_operand);
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
        if (!entry2_operand) {
            CCF();
            expr.AnsSetCarryFlagReversed = true;
        }
        SBC_HL_HL();
        INC_HL();
        expr.AnsSetCarryFlag = true;
        expr.ZeroCarryFlagRemoveAmountOfBytes = 3 + !entry2_operand;
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
void AndVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    AndChainAnsVariable();
}
void AndFunctionFunction(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry2_operand, OUTPUT_IN_HL, NEED_PUSH);
    AndInsert();
}
void AndChainPushChainAns(void) {
    POP_DE();
    AndInsert();
}

#define XorVariableNumber    AndVariableNumber   
#define XorVariableVariable  AndVariableVariable 
#define XorVariableFunction  AndVariableFunction 
#define XorFunctionNumber    AndFunctionNumber   
#define XorFunctionVariable  AndFunctionVariable 
#define XorFunctionFunction  AndFunctionFunction 
#define XorChainAnsNumber    AndChainAnsNumber   
#define XorChainAnsVariable  AndChainAnsVariable 
#define XorChainAnsFunction  AndChainAnsFunction 
#define XorChainPushChainAns AndChainPushChainAns

#define OrVariableNumber    AndVariableNumber   
#define OrVariableVariable  AndVariableVariable 
#define OrVariableFunction  AndVariableFunction 
#define OrFunctionNumber    AndFunctionNumber   
#define OrFunctionVariable  AndFunctionVariable 
#define OrFunctionFunction  AndFunctionFunction 
#define OrChainAnsNumber    AndChainAnsNumber   
#define OrChainAnsVariable  AndChainAnsVariable 
#define OrChainAnsFunction  AndChainAnsFunction 
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
void EQVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    EQChainAnsVariable();
}
void EQFunctionFunction(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_DE, NO_PUSH);
    insertFunctionReturn(entry2_operand, OUTPUT_IN_HL, NEED_PUSH);
    EQInsert();
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
    MaybeHLToDE();
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
    MaybeHLToDE();
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NEED_PUSH);
}
void GEChainAnsFunction(void) {
    MaybeDEToHL();
    insertFunctionReturn(entry2_operand, OUTPUT_IN_DE, NEED_PUSH);
    GEInsert();
}
void GEChainPushChainAns(void) {
    MaybeHLToDE();
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
    MaybeDEToHL();
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
    MaybeDEToHL();
    POP_DE();
    OR_A_A();
    SBC_HL_DE();
    SBC_HL_HL();
    INC_HL();
    expr.AnsSetCarryFlag = true;
    expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
}

#define NEVariableNumber    EQVariableNumber
#define NEVariableVariable  EQVariableVariable
#define NEFunctionNumber    EQFunctionNumber
#define NEFunctionVariable  EQFunctionVariable
#define NEFunctionFunction  EQFunctionFunction
#define NEChainAnsNumber    EQChainAnsNumber
#define NEChainAnsVariable  EQChainAnsVariable
#define NEChainAnsFunction  EQChainAnsFunction
#define NEChainPushChainAns EQChainPushChainAns

void MulChainAnsNumber(void) {
    uint24_t number = entry2_operand;
    if (number == 0) {
        ice.programPtr = ice.programPtrBackup;
        LD_HL_NUMBER(0);
    } else {
        MultWithNumber(number, (uint8_t*)&ice.programPtr, 16*expr.outputRegister);
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
}
void MulFunctionVariable(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    MulChainAnsVariable();
}
void MulVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    MulChainAnsVariable();
}
void MulFunctionFunction(void) {
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    PUSH_HL();
    insertFunctionReturn(entry2_operand, OUTPUT_IN_HL, NO_PUSH);
    POP_BC();
}
void MulChainAnsFunction(void) {
    MaybeDEToHL();
    insertFunctionReturn(entry2_operand, OUTPUT_IN_BC, NEED_PUSH);
}
void MulChainPushChainAns(void) {
    MaybeDEToHL();
    POP_BC();
}
void DivChainAnsNumber(void) {
    MaybeDEToHL();
    LD_BC_IMM(entry2_operand);
}
void DivChainAnsVariable(void) {
    MaybeDEToHL();
    LD_BC_IND_IX_OFF(entry2_operand);
}
void DivNumberVariable(void) {
    LD_HL_NUMBER(entry1_operand);
    DivChainAnsVariable();
}
void DivNumberFunction(void) {
    insertFunctionReturn(entry2_operand, OUTPUT_IN_BC, NO_PUSH);
    LD_HL_NUMBER(entry1_operand);
}
void DivNumberChainAns(void) {
    PushHLDE();
    POP_BC();
    LD_HL_NUMBER(entry1_operand);
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
}
void DivVariableChainAns(void) {
    PushHLDE();
    POP_BC();
    LD_HL_IND_IX_OFF(entry1_operand);
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
}
void DivFunctionChainAns(void) {
    PushHLDE();
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NO_PUSH);
    POP_BC();
}
void DivChainAnsFunction(void) {
    MaybeDEToHL();
    insertFunctionReturn(entry2_operand, OUTPUT_IN_BC, NEED_PUSH);
}
void DivChainPushChainAns(void) {
    PushHLDE();
    POP_BC();
    POP_HL();
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
void AddVariableVariable(void) {
    LD_HL_IND_IX_OFF(entry1_operand);
    if (entry1_operand == entry2_operand) {
        ADD_HL_HL();
    } else {
        AddChainAnsVariable();
    }
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
        LD_HL_STRING(entry1_operand);
        PUSH_HL();
        if (entry2_operand == TempString1) {
            LD_HL_IMM(TempString2);
        } else {
            LD_HL_IMM(TempString1);
        }
        PUSH_HL();
        CALL(__strcpy);
        POP_DE();
        LD_HL_STRING(entry2_operand);
        EX_SP_HL();
        PUSH_DE();
        CALL(__strcat);
        POP_BC();
        POP_BC();
    } else {
        LD_HL_STRING(entry2_operand);
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
    MaybeDEToHL();
    LD_DE_IND_IX_OFF(entry2_operand);
}
void SubNumberVariable(void) {
    LD_HL_NUMBER(entry1_operand);
    SubChainAnsVariable();
}
void SubNumberFunction(void) {
    insertFunctionReturn(entry2_operand, OUTPUT_IN_DE, NO_PUSH);
    LD_HL_NUMBER(entry1_operand);
}
void SubNumberChainAns(void) {
    MaybeHLToDE();
    LD_HL_NUMBER(entry1_operand);
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
    MaybeHLToDE();
    LD_HL_IND_IX_OFF(entry1_operand);
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
}
void SubFunctionChainAns(void) {
    MaybeHLToDE();
    insertFunctionReturn(entry1_operand, OUTPUT_IN_HL, NEED_PUSH);
}
void SubChainAnsFunction(void) {
    MaybeDEToHL();
    insertFunctionReturn(entry2_operand, OUTPUT_IN_DE, NEED_PUSH);
}
void SubChainPushChainAns(void) {
    MaybeHLToDE();
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
    OperatorError,
    OperatorError,
    OperatorError,
    AndVariableNumber,
    AndVariableVariable,
    OperatorError,
    OperatorError,
    AndFunctionNumber,
    AndFunctionVariable,
    AndFunctionFunction,
    OperatorError,
    AndChainAnsNumber,
    AndChainAnsVariable,
    AndChainAnsFunction,
    OperatorError,
    
    OperatorError,
    OperatorError,
    OperatorError,
    OperatorError,
    XorVariableNumber,
    XorVariableVariable,
    OperatorError,
    OperatorError,
    XorFunctionNumber,
    XorFunctionVariable,
    XorFunctionFunction,
    OperatorError,
    XorChainAnsNumber,
    XorChainAnsVariable,
    XorChainAnsFunction,
    OperatorError,
    
    OperatorError,
    OperatorError,
    OperatorError,
    OperatorError,
    OrVariableNumber,
    OrVariableVariable,
    OperatorError,
    OperatorError,
    OrFunctionNumber,
    OrFunctionVariable,
    OrFunctionFunction,
    OperatorError,
    OrChainAnsNumber,
    OrChainAnsVariable,
    OrChainAnsFunction,
    OperatorError,
    
    OperatorError,
    OperatorError,
    OperatorError,
    OperatorError,
    EQVariableNumber,
    EQVariableVariable,
    OperatorError,
    OperatorError,
    EQFunctionNumber,
    EQFunctionVariable,
    EQFunctionFunction,
    OperatorError,
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
    OperatorError,
    OperatorError,
    OperatorError,
    NEVariableNumber,
    NEVariableVariable,
    OperatorError,
    OperatorError,
    NEFunctionNumber,
    NEFunctionVariable,
    NEFunctionFunction,
    OperatorError,
    NEChainAnsNumber,
    NEChainAnsVariable,
    NEChainAnsFunction,
    OperatorError,
    
    OperatorError,
    OperatorError,
    OperatorError,
    OperatorError,
    MulVariableNumber,
    MulVariableVariable,
    OperatorError,
    OperatorError,
    MulFunctionNumber,
    MulFunctionVariable,
    MulFunctionFunction,
    OperatorError,
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
    OperatorError,
    OperatorError,
    OperatorError,
    AddVariableNumber,
    AddVariableVariable,
    OperatorError,
    OperatorError,
    AddFunctionNumber,
    AddFunctionVariable,
    AddFunctionFunction,
    OperatorError,
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

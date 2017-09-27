#include "defines.h"
#include "routines.h"

#include "main.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "output.h"
#include "operator.h"

#define LB_X 40
#define LB_Y 200
#define LB_W 240
#define LB_H 14
#define LB_R (LB_H / 2)

extern variable_t variableStack[85];

void ProgramPtrToOffsetStack(void) {
    ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(ice.programPtr + 1);
}

void ProgramDataPtrToOffsetStack(void) {
    ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)ice.programDataPtr;
}

void AnsToHL(void) {
    if (expr.outputRegister == OUTPUT_IN_DE) {
        EX_DE_HL();
    } else if (expr.outputRegister == OUTPUT_IN_A) {
        OR_A_A();
        SBC_HL_HL();
        LD_L_A();
    }
    expr.outputRegister = OUTPUT_IN_HL;
}

void AnsToDE(void) {
    if (expr.outputRegister == OUTPUT_IN_HL) {
        EX_DE_HL();
    } else if (expr.outputRegister == OUTPUT_IN_A) {
        LD_DE_IMM(0);
        LD_E_A();
    }
}

void MaybeAToHL(void) {
    if (expr.outputRegister == OUTPUT_IN_A) {
        OR_A_A();
        SBC_HL_HL();
        LD_L_A();
        expr.outputRegister = OUTPUT_IN_HL;
    }
}

void MaybeLDIYFlags(void) {
    if (ice.modifiedIY) {
        LD_IY_IMM(flags);
        ice.modifiedIY = false;
    }
}

void PushHLDE(void) {
    MaybeAToHL();
    if (expr.outputRegister == OUTPUT_IN_HL) {
        PUSH_HL();
    } else {
        PUSH_DE();
    }
}

uint8_t IsHexadecimal(int token) {
    uint8_t tok = token;
    
    if (tok >= t0 && tok <= t9) {
        return tok - t0;
    } else if (tok >= tA && tok <= tF) {
        return tok - tA + 10;
    } else {
        return 16;
    }
}

bool CheckEOL(void) {
    int token;
    
    if ((token = _getc(ice.inPrgm)) == EOF || (uint8_t)token == tEnter) {
        return true;
    }
    return false;
}

uint8_t SquishHexadecimals(uint8_t *prevDataPtr) {
    uint8_t *prevDataPtr2 = prevDataPtr;
            
    // Replace the hexadecimal string to hexadecimal bytes
    while (prevDataPtr != ice.programDataPtr - 1) {
        uint8_t tok1, tok2;
        
        if ((tok1 = IsHexadecimal(*prevDataPtr++)) == 16 || (tok2 = IsHexadecimal(*prevDataPtr++)) == 16) {
            return E_SYNTAX;
        }
        *prevDataPtr2++ = (tok1 << 4) + tok2;
    }
    
    ice.programDataPtr = prevDataPtr2;
    return VALID;
}

uint8_t GetVariableOffset(uint8_t tok) {
    char variableName[10] = {0,0,0,0,0,0,0,0,0,0};
    variable_t *variableNew;
    uint24_t a = 1, b;
    
    variableName[0] = tok;
    while ((tok = _getc(ice.inPrgm)) >= tA && tok <= tTheta) {
        variableName[a++] = tok;
    }
    variableName[a] = 0;
    if (tok != 0xFF) {
        _seek(-1, SEEK_CUR, ice.inPrgm);
    }
    
    // This variable already exists
    for (b = 0; b < ice.amountOfVariablesUsed; b++) {
        if (!strcmp(variableName, (&variableStack[b])->name)) {
            return (&variableStack[b])->offset;
        }
    }
    
    // Create new variable
    variableNew = &variableStack[ice.amountOfVariablesUsed];
    memcpy(variableNew->name, variableName, a + 1);
    return variableNew->offset = ice.amountOfVariablesUsed++ * 3 - 128;
}

#ifndef COMPUTER_ICE

void displayLoadingBarFrame(void) {
    // Display a fancy loading bar during compiling ;)
    gfx_SetColor(255);
    gfx_FillRectangle_NoClip(LB_X - LB_R, LB_Y - LB_R, LB_W + LB_H, LB_H);
    gfx_SetColor(0);
    gfx_Circle_NoClip(LB_X, LB_Y, LB_R);
    gfx_Circle_NoClip(LB_X + LB_W, LB_Y, LB_R);
    gfx_HorizLine_NoClip(LB_X, LB_Y - LB_R, LB_W);
    gfx_HorizLine_NoClip(LB_X, LB_Y + LB_R, LB_W);
    gfx_SetColor(255);
    gfx_FillRectangle_NoClip(LB_X, LB_Y - LB_R + 1, LB_R + 1, LB_H - 1);
    gfx_FillRectangle_NoClip(LB_X + LB_W - LB_R, LB_Y - LB_R + 1, LB_R + 1, LB_H - 1);
}

void displayLoadingBar(ti_var_t inPrgm) {
    gfx_SetClipRegion(
        LB_X - LB_R + 1, 
        LB_Y - LB_R, 
        LB_X - LB_R + 2 + ti_Tell(inPrgm) * (LB_W + LB_R - 1 + LB_R - 1) / ti_GetSize(inPrgm), 
        LB_Y + LB_R
    );
    gfx_SetColor(4);
    gfx_FillCircle(LB_X, LB_Y, LB_R - 1);
    gfx_FillCircle(LB_X + LB_W, LB_Y, LB_R - 1);
    gfx_FillRectangle(LB_X, LB_Y - LB_R + 1, LB_W, LB_H);
}

uint24_t getNextToken(ti_var_t inPrgm) {
    // Display loading bar
    displayLoadingBar(inPrgm);
    return ti_GetC(inPrgm);
}

#else
    
uint24_t getNextToken(ti_var_t inPrgm) {
    if ((uint24_t)_tell(inPrgm) < ice.programLength - 2) {
        return _getc(inPrgm);
    }
    return EOF;
}

#endif

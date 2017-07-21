#include "routines.h"

#include "main.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "output.h"
#include "operator.h"
#include "gfx/gfx_logos.h"

#ifndef COMPUTER_ICE
#include <graphx.h>
#endif

#include <tice.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LB_X 40
#define LB_Y 200
#define LB_W 240
#define LB_H 14
#define LB_R (LB_H / 2)

void ProgramPtrToOffsetStack(void) {
    ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(ice.programPtr + 1);
}

void MaybeDEToHL(void) {
    if (expr.outputRegister != OutputRegisterHL) {
        EX_DE_HL();
    }
}

void MaybeHLToDE(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        EX_DE_HL();
    }
}

void PushHLDE(void) {
    if (expr.outputRegister == OutputRegisterHL) {
        PUSH_HL();
    } else {
        PUSH_DE();
    }
}

uint8_t IsHexadecimal(uint24_t token) {
    uint8_t tok = (uint8_t)token;
    if (tok >= t0 && tok <= t9) {
        return tok - t0;
    } else if (tok >= tA && tok <= tF) {
        return tok - tA + 10;
    } else {
        return 16;
    }
}

bool CheckEOL(ti_var_t currentProgram) {
    uint24_t token;
    if ((int)(token = __getc()) == EOF || (uint8_t)token == tEnter) {
        return true;
    }
    return false;
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

void displayLoadingBar(ti_var_t currentProgram) {
    gfx_SetClipRegion(
        LB_X - LB_R + 1, 
        LB_Y - LB_R, 
        LB_X - LB_R + 2 + ti_Tell(currentProgram) * (LB_W + LB_R - 1 + LB_R - 1) / ti_GetSize(currentProgram), 
        LB_Y + LB_R
    );
    gfx_SetColor(4);
    gfx_FillCircle(LB_X, LB_Y, LB_R - 1);
    gfx_FillCircle(LB_X + LB_W, LB_Y, LB_R - 1);
    gfx_FillRectangle(LB_X, LB_Y - LB_R + 1, LB_W, LB_H);
}

unsigned int getNextToken(ti_var_t currentProgram) {
    // Display loading bar
    displayLoadingBar(currentProgram);
    return ti_GetC(currentProgram);
}

void setCurrentOffset(int offset, int origin, ti_var_t stream) {
    ti_Seek(offset, origin, stream);
}

unsigned int getCurrentOffset(ti_var_t current) {
    return ti_Tell(current);
}

#else
    
unsigned int getNextToken(ti_var_t currentProgram) {
    if (ftell(ice.inPrgm) < ice.programLength - 2) {
        return getc(currentProgram);
    }
    return EOF;
}

void setCurrentOffset(int offset, int origin, ti_var_t stream) {
    fseek(stream, offset, origin);
}

unsigned int getCurrentOffset(ti_var_t current) {
    return ftell(current);
}

#endif

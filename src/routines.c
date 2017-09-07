#include "defines.h"
#include "routines.h"

#include "main.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "output.h"
#include "operator.h"
//#include "gfx/gfx_logos.h"

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

bool CheckEOL(void) {
    uint24_t token;
    if ((int)(token = _getc(ice.inPrgm)) == EOF || (uint8_t)token == tEnter) {
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

void setCurrentOffset(uint24_t offset, uint24_t origin) {
    ti_Seek(offset, origin, ice.inPrgm);
}

uint24_t getCurrentOffset(void) {
    return ti_Tell(ice.inPrgm);
}

#else
    
uint24_t getNextToken(ti_var_t inPrgm) {
    if (_tell(inPrgm) < ice.programLength - 2) {
        return _getc(inPrgm);
    }
    return EOF;
}

#endif

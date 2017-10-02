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
    expr.outputRegister = OUTPUT_IN_DE;
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
    
    if ((token = _getc()) == EOF || (uint8_t)token == tEnter) {
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
    while ((tok = _getc()) >= tA && tok <= tTheta) {
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

uint24_t getNextToken(void) {
    // Display loading bar
    displayLoadingBar(ice.inPrgm);
    return ti_GetC(ice.inPrgm);
}

int grabString(uint8_t **outputPtr, bool stopAtStoreAndString) {
    void *dataPtr = ti_GetDataPtr(ice.inPrgm);
    uint24_t token;
    
    while ((token = _getc()) != EOF && !(stopAtStoreAndString && ((uint8_t)token == tString || (uint8_t)token == tStore)) && (uint8_t)token != tEnter) {
        uint24_t strLength, a;
        const char *dataString;
        uint8_t tokSize;
        
        // Get the token in characters, and copy to the output
        dataString = ti_GetTokenString(&dataPtr, &tokSize, &strLength);
        memcpy(*outputPtr, dataString, strLength);
        
        for (a = 0; a < strLength; a++) {
            uint8_t char2 = *(*outputPtr + a);
            
            // Differences in TI-ASCII and ASCII set, C functions expect the normal ASCII set
            // There are no 4sqrt( and theta symbol in the first 128 characters of the ASCII set
            if (char2 == 0x24 || char2 == 0x5B) {
                char2 = 0;
            }
            
            // $ = 0x24
            if (char2 == 0xF2) {
                char2 = 0x24;
            }
            
            // [ = 0x5B
            if (char2 == 0xC1) {
                char2 = 0x5B;
            }
            
            // All the first 32 and last 128 characters are different
            if (char2 < 32 || char2 > 127) {
                displayError(E_WRONG_CHAR);
                char2 = 0;
            }
            
            *(*outputPtr + a) = char2;
        }
        
        *outputPtr += strLength;
        
        // If it's a 2-byte token, we also need to get the second byte of it
        if (tokSize == 2) {
            _getc();
        }
    }
    
    return token;
}

#else
    
const char *tokenStrings[] = {
    "►DMS",
	"►Dec",
	"►Frac",
	"→",
	"Boxplot",
	"[",
	"]",
	"{",
	"}",
	"ʳ",
	"°",
	"ˉ¹",
	"²",
	"ᵀ",
	"³",
	"(",
	")",
	"round(",
	"pxl-Test(",
	"augment(",
	"rowSwap(",
	"row+(",
	"*row(",
	"*row+(",
	"max(",
	"min(",
	"R►Pr(",
	"R►Pθ(",
	"P►Rx(",
	"P►Ry(",
	"median(",
	"randM(",
	"mean(",
	"solve(",
	"seq(",
	"fnInt(",
	"nDeriv(",
	"",
	"0",
	"fMax(",
	" ",
	"\"",
	",",
	"i",
	"!",
	"CubicReg ",
	"QuartReg ",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	".",
	"ᴇ",
	" or ",
	" xor ",
	":",
	"\n",
	" and ",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"θ",
	"",
	"",
	"",
	"prgm",
	"",
	"",
	"",
	"",
	"Radian",
	"Degree",
	"Normal",
	"Sci",
	"Eng",
	"Float",
	"=",
	"<",
	">",
	"≤",
	"≥",
	"≠",
	"+",
	"-",
	"Ans",
	"Fix ",
	"Horiz",
	"Full",
	"Func",
	"Param",
	"Polar",
	"Seq",
	"IndpntAuto",
	"IndpntAsk",
	"DependAuto",
	"DependAsk",
	"",
	"□",
	"﹢",
	"·",
	"*",
	"/",
	"Trace",
	"ClrDraw",
	"ZStandard",
	"ZTrig",
	"ZBox",
	"Zoom In",
	"Zoom Out",
	"ZSquare",
	"ZInteger",
	"ZPrevious",
	"ZDecimal",
	"ZoomStat",
	"ZoomRcl",
	"PrintScreen",
	"ZoomSto",
	"Text(",
	" nPr ",
	" nCr ",
	"FnOn ",
	"FnOff ",
	"StorePic ",
	"RecallPic ",
	"StoreGDB ",
	"RecallGDB ",
	"Line(",
	"Vertical ",
	"Pt-On(",
	"Pt-Off(",
	"Pt-Change(",
	"Pxl-On(",
	"Pxl-Off(",
	"Pxl-Change(",
	"Shade(",
	"Circle(",
	"Horizontal ",
	"Tangent(",
	"DrawInv ",
	"DrawF ",
	"",
	"rand",
	"π",
	"getKey",
	"'",
	"?",
	"⁻",
	"int(",
	"abs(",
	"det(",
	"identity(",
	"dim(",
	"sum(",
	"prod(",
	"not(",
	"iPart(",
	"fPart(",
	"",
	"√(",
	"³√(",
	"ln(",
	"e^(",
	"log(",
	"₁₀^(",
	"sin(",
	"sin⁻¹(",
	"cos(",
	"cos⁻¹(",
	"tan(",
	"tan⁻¹(",
	"sinh(",
	"sinh⁻¹(",
	"cosh(",
	"soch⁻¹(",
	"tanh(",
	"tanh⁻¹(",
	"If ",
	"Then",
	"Else",
	"While ",
	"Repeat ",
	"For(",
	"End",
	"Return",
	"Lbl ",
	"Goto ",
	"Pause ",
	"Stop",
	"IS>(",
	"DS<(",
	"Input ",
	"Prompt ",
	"Disp ",
	"DispGraph",
	"Output(",
	"ClrHome",
	"Fill(",
	"SortA(",
	"SortD(",
	"DispTable",
	"Menu(",
	"Send(",
	"Get(",
	"PlotsOn ",
	"PlotsOff ",
	"⌊",
	"Plot1(",
	"Plot2(",
	"Plot3(",
	"",
	"^",
	"ˣ√",
	"1-Var Stats ",
	"2-Var Stats ",
	"LinReg(a+bx) ",
	"ExpReg ",
	"LnReg ",
	"PwrReg ",
	"Med-Med ",
	"QuadReg ",
	"ClrList ",
	"ClrTable",
	"Histogram",
	"xyLine",
	"Scatter",
	"LinReg(ax+b) ",
	"[A]",
	"[B]",
	"[C]",
	"[D]",
	"[E]",
	"[F]",
	"[G]",
	"[H]",
	"[I]",
	"[J]",
	"L₁",
	"L₂",
	"L₃",
	"L₄",
	"L₅",
	"L₆",
	"Y₁",
	"Y₂",
	"Y₃",
	"Y₄",
	"Y₅",
	"Y₆",
	"Y₇",
	"Y₈",
	"Y₉",
	"Y₀",
	"X₁ᴛ",
	"Y₁ᴛ",
	"X₂ᴛ",
	"Y₂ᴛ",
	"X₃ᴛ",
	"Y₃ᴛ",
	"X₄ᴛ",
	"Y₄ᴛ",
	"X₅ᴛ",
	"Y₅ᴛ",
	"X₆ᴛ",
	"Y₆ᴛ",
	"r₁",
	"r₂",
	"r₃",
	"r₄",
	"r₅",
	"r₆",
	"|u",
	"|v",
	"|w",
	"Pic1",
	"Pic2",
	"Pic3",
	"Pic4",
	"Pic5",
	"Pic6",
	"Pic7",
	"Pic8",
	"Pic9",
	"Pic0",
	"GDB1",
	"GDB2",
	"GDB3",
	"GDB4",
	"GDB5",
	"GDB6",
	"GDB7",
	"GDB8",
	"GDB9",
	"GDB0",
	"RegEQ",
	"n",
	"x̄",
	"Σx",
	"Σx²",
	"[Sx]",
	"σx",
	"[minX]",
	"[maxX]",
	"[minY]",
	"[maxY]",
	"ȳ",
	"Σy",
	"Σy²",
	"[Sy]",
	"σy",
	"Σxy",
	"[r]",
	"[Med]",
	"Q₁",
	"Q₃",
	"[|a]",
	"[|b]",
	"[|c]",
	"[|d]",
	"[|e]",
	"x₁",
	"x₂",
	"x₃",
	"y₁",
	"y₂",
	"y₃",
	"𝒏",
	"[p]",
	"[z]",
	"[t]",
	"χ²",
	"[|F]",
	"[df]",
	"p̂",
	"p̂₁",
	"p̂₂",
	"x̄₁",
	"Sx₁",
	"n₁",
	"x̄₂",
	"Sx₂",
	"n₂",
	"Sxp",
	"lower",
	"upper",
	"[s]",
	"r²",
	"R²",
	"[factordf]",
	"[factorSS]",
	"[factorMS]",
	"[errordf]",
	"[errorSS]",
	"[errorMS]",
	"ZXscl",
	"ZYscl",
	"Xscl",
	"Yscl",
	"u(nMin)",
	"v(nMin)",
	"Un-₁",
	"Vn-₁",
	"Zu(nmin)",
	"Zv(nmin)",
	"Xmin",
	"Xmax",
	"Ymin",
	"Ymax",
	"Tmin",
	"Tmax",
	"θMin",
	"θMax",
	"ZXmin",
	"ZXmax",
	"ZYmin",
	"ZYmax",
	"Zθmin",
	"Zθmax",
	"ZTmin",
	"ZTmax",
	"TblStart",
	"PlotStart",
	"ZPlotStart",
	"nMax",
	"ZnMax",
	"nMin",
	"ZnMin",
	"∆Tbl",
	"Tstep",
	"θstep",
	"ZTstep",
	"Zθstep",
	"∆X",
	"∆Y",
	"XFact",
	"YFact",
	"TblInput",
	"𝗡",
	"I%",
	"PV",
	"PMT",
	"FV",
	"|P/Y",
	"|C/Y",
	"w(nMin)",
	"Zw(nMin)",
	"PlotStep",
	"ZPlotStep",
	"Xres",
	"ZXres",
	"TraceStep",
	"Sequential",
	"Simul",
	"PolarGC",
	"RectGC",
	"CoordOn",
	"CoordOff",
	"Connected",
	"Thick",
	"Dot",
	"Dot-Thick",
	"AxesOn ",
	"AxesOff",
	"GridDot ",
	"GridOn",
	"GridOff",
	"LabelOn",
	"LabelOff",
	"Web",
	"Time",
	"uvAxes",
	"vwAxes",
	"uwAxes",
	"Str1",
	"Str2",
	"Str3",
	"Str4",
	"Str5",
	"Str6",
	"Str7",
	"Str8",
	"Str9",
	"Str0",
	"npv(",
	"irr(",
	"bal(",
	"ΣPrn(",
	"ΣInt(",
	"►Nom(",
	"►Eff(",
	"dbd(",
	"lcm(",
	"gcd(",
	"randInt(",
	"randBin(",
	"sub(",
	"stdDev(",
	"variance(",
	"inString(",
	"normalcdf(",
	"invNorm(",
	"tcdf(",
	"Fcdf(",
	"binompdf(",
	"binomcdf(",
	"poissonpdf(",
	"poissoncdf(",
	"geometpdf(",
	"geometcdf(",
	"normalpdf(",
	"tpdf(",
	"χ²pdf(",
	"Fpdf(",
	"randNorm(",
	"tvm_Pmt",
	"tvm_I%",
	"tvm_PV",
	"tvm_N",
	"tvm_FV",
	"conj(",
	"real(",
	"imag(",
	"angle(",
	"cumSum(",
	"expr(",
	"length(",
	"ΔList(",
	"ref(",
	"rref(",
	"►Rect",
	"►Polar",
	"e",
	"SinReg ",
	"Logistic ",
	"LinRegTTest ",
	"ShadeNorm(",
	"Shade_t(",
	"Shadeχ²(",
	"ShadeF(",
	"Matr►list(",
	"List►matr(",
	"Z-Test(",
	"T-Test ",
	"2-SampZTest(",
	"1-PropZTest(",
	"2-PropZTest(",
	"χ²-Test(",
	"ZInterval ",
	"2-SampZInt(",
	"1-PropZInt(",
	"2-PropZInt(",
	"GraphStyle(",
	"2-SampTTest ",
	"2-SampFTest ",
	"TInterval ",
	"2-SampTInt ",
	"SetUpEditor ",
	"Pmt_End",
	"Pmt_Bgn",
	"Real",
	"re^θi",
	"a+bi",
	"ExprOn",
	"ExprOff",
	"ClrAllLists",
	"GetCalc(",
	"DelVar ",
	"Equ►String(",
	"String►Equ(",
	"Clear Entries",
	"Select(",
	"ANOVA(",
	"ModBoxplot",
	"NormProbPlot",
	"G-T",
	"ZoomFit",
	"DiagnosticOn",
	"DiagnosticOff",
	"Archive ",
	"UnArchive ",
	"Asm(",
	"AsmComp(",
	"AsmPrgm",
	"compiled asm",
	"Á",
	"À",
	"Â",
	"Ä",
	"á",
	"à",
	"â",
	"ä",
	"É",
	"È",
	"Ê",
	"Ë",
	"é",
	"è",
	"ê",
	"ë",
	"Ì",
	"Î",
	"Ï",
	"í",
	"ì",
	"î",
	"ï",
	"Ó",
	"Ò",
	"Ô",
	"Ö",
	"ó",
	"ò",
	"ô",
	"ö",
	"Ú",
	"Ù",
	"Û",
	"Ü",
	"ú",
	"ù",
	"û",
	"ü",
	"Ç",
	"ç",
	"Ñ",
	"ñ",
	"´",
	"`",
	"¨",
	"¿",
	"¡",
	"α",
	"β",
	"γ",
	"Δ",
	"δ",
	"ε",
	"λ",
	"μ",
	"|π",
	"ρ",
	"Σ",
	"Φ",
	"Ω",
	"ṗ",
	"χ",
	"𝐅",
	"a",
	"b",
	"c",
	"d",
	"e",
	"f",
	"g",
	"h",
	"i",
	"j",
	"k",
	"l",
	"m",
	"n",
	"o",
	"p",
	"q",
	"r",
	"s",
	"t",
	"u",
	"v",
	"w",
	"x",
	"y",
	"z",
	"σ",
	"τ",
	"Í",
	"GarbageCollect",
	"~",
	"@",
	"#",
	"$",
	"&",
	"‛",
	""	"",
	"\\",
	"|",
	"_",
	"'",
	"%",
	"…",
	"∠",
	"ß",
	"ˣ",
	"ᴛ",
	"₀",
	"₁",
	"₂",
	"₃",
	"₄",
	"₅",
	"₆",
	"₇",
	"₈",
	"₉",
	"₁₀",
	"◄",
	"►",
	"↑",
	"↓",
	"×",
	"∫",
	"🡅",
	"🡇",
	"√",
	"setDate(",
	"setTime(",
	"checkTmr(",
	"setDtFmt(",
	"setTmFmt(",
	"timeCnv(",
	"dayOfWk(",
	"getDtStr(",
	"getTmStr(",
	"getDate",
	"getTime",
	"startTmr",
	"getDtFmt",
	"getTmFmt",
	"isClockOn",
	"ClockOff",
	"ClockOn",
	"OpenLib(",
	"ExecLib",
	"invT(",
	"χ²GOF-Test(",
	"LinRegTInt ",
	"Manual-Fit ",
	"ZQuadrant1",
	"ZFrac1⁄2",
	"ZFrac1⁄3",
	"ZFrac1⁄4",
	"ZFrac1⁄5",
	"ZFrac1⁄8",
	"ZFrac1⁄10",
	"⬚",
	"⁄",
	"ᵤ",
	"►n⁄d◄►Un⁄d",
	"►F◄►D",
	"remainder(",
	"Σ(",
	"logBASE(",
	"randIntNoRep(",
	"MATHPRINT",
	"CLASSIC",
	"n⁄d",
	"Un⁄d",
	"AUTO",
	"DEC",
	"FRAC",
	"FRAC-APPROX",
	"BLUE",
	"RED",
	"BLACK",
	"MAGENTA",
	"GREEN",
	"ORANGE",
	"BROWN",
	"NAVY",
	"LTBLUE",
	"YELLOW",
	"WHITE",
	"LTGRAY",
	"MEDGRAY",
	"GRAY",
	"DARKGRAY",
	"Image1",
	"Image2",
	"Image3",
	"Image4",
	"Image5",
	"Image6",
	"Image7",
	"Image8",
	"Image9",
	"Image0",
	"GridLine ",
	"BackgroundOn ",
	"BackgroundOff",
	"GraphColor(",
	"QuickPlot&Fit-EQ",
	"TextColor(",
	"Asm84CPrgm",
	"",
	"DetectAsymOn",
	"DetectAsymOff",
	"BorderColor ",
	"invBinom(",
	"Wait ",
	"toString(",
	"eval("
};
    
int grabString(uint8_t **outputPtr, bool stopAtStoreAndString) {
    char tempString[16];
    uint24_t output, a;
    
    while (1) {
        int token = _getc();
        uint8_t tok = token;
        
        if (tok == tEnter || token == EOF || (stopAtStoreAndString && (tok == tStore || tok == tString))) {
            return token;
        }
        
        if (tok == 0x5C) {
            output = 255 + _getc();
        } else if (tok == 0x5D) {
            output = 265 + _getc();
        } else if (tok == 0x5E) {
            tok = _getc();
            output = 271 + tok - 16 - 6 * (tok >= 0x20) - 20 * (tok >= 0x40) - 58 * (tok >= 0x80);
        } else if (tok == 0x60) {
            output = 302 + _getc();
        } else if (tok == 0x61) {
            output = 312 + _getc();
        } else if (tok == 0x62) {
            output = 322 + _getc() - 1;
        } else if (tok == 0x63) {
            output = 382 + _getc();
        } else if (tok == 0x7E) {
            output = 439 + _getc();
        } else if (tok == 0xAA) {
            output = 461 + _getc();
        } else if (tok == 0xBB) {
            output = 471 + _getc();
        } else if (tok == 0xEF) {
            tok = _getc();
            output = 703 + tok - 15 * (tok >= 0x2E) - 3 * (tok >= 0x40) - 8 * (tok >= 0x60) - 40 * (tok >= 0x70);
        } else {
            output = token - 1;
        }
        
        strcpy(tempString, tokenStrings[output]);
        
        for (a = 0; a < 16; a++) {
            uint8_t char2 = tempString[a];
            
            if (!char2) {
                break;
            }
            
            // No weird characters
            if (char2 < 32 || char2 > 127) {
                displayError(E_WRONG_CHAR);
                char2 = 0;
            }
            
            *(*outputPtr)++ = char2;
        }
    }
}
    
uint24_t getNextToken(void) {
    if ((uint24_t)_tell(ice.inPrgm) < ice.programLength - 2) {
        return fgetc(ice.inPrgm);
    }
    return EOF;
}

#endif

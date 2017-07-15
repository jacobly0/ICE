#include "main.h"

#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "output.h"
#include "operator.h"

#ifndef COMPUTER_ICE
#include <fileioc.h>
#include <graphx.h>
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

ice_t ice;
expr_t expr;

#define LB_X 40
#define LB_Y 200
#define LB_W 240
#define LB_H 14
#define LB_R (LB_H / 2)

const char *infoStr = "ICE Compiler v1.6 - By Peter \"PT_\" Tillema";

#ifdef COMPUTER_ICE
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(CHeader, "src/asm/cheader.bin");
INCBIN(CProgramHeader, "src/asm/cprogramheader.bin");
#endif

#ifdef COMPUTER_ICE
void w24(void *x, uint32_t val) {
    uint8_t *ptr = (uint8_t*)(x);
    ptr[0] = val & 0xFF;
    ptr[1] = val >> 8 & 0xFF;
    ptr[2] = val >> 16 & 0xFF;
}
uint32_t r24(void *x) {
    uint8_t *ptr = (uint8_t*)(x);
    return (ptr[2] << 16) | (ptr[1] << 8) | (ptr[0]);
}
#endif

#ifndef COMPUTER_ICE
void main(void) {
#else
void export_program(const char *name, uint8_t *data, size_t size);
int main(int argc, char **argv) {
#endif
    uint8_t a = 0, selectedProgram = 0, key, amountOfPrograms, res, *outputDataPtr, *search_pos = NULL;
    char *var_name;
    uint24_t token, headerSize, programDataSize, offset, totalSize;
    const char ICEheader[] = {tii, 0};
    char buf[30];
    
#ifndef COMPUTER_ICE
    // Yay, GUI! :)
    gfx_Begin(gfx_8bpp);
    gfx_SetColor(189);
    gfx_FillRectangle_NoClip(0, 0, 320, 10);
    gfx_SetColor(0);
    gfx_SetTextFGColor(0);
    gfx_HorizLine_NoClip(0, 10, 320);
    gfx_PrintStringXY(infoStr, 21, 1);
    
    // Get all the programs that start with the [i] token
    while((var_name = ti_DetectVar(&search_pos, ICEheader, TI_PRGM_TYPE)) && ++selectedProgram <= 22) {
        gfx_PrintStringXY(var_name, 10, selectedProgram*10 + 3);
    }
    amountOfPrograms = selectedProgram;
    
    // Check if there are ICE programs
    if (!amountOfPrograms) {
        gfx_PrintStringXY("No programs found!", 10, 13);
        goto stop;
    }
    
    // Select a program
    selectedProgram = 1;
    while ((key = os_GetCSC()) != sk_Enter) {
        uint8_t selectionOffset = selectedProgram*10 + 3;

        gfx_PrintStringXY(">", 1, selectionOffset);

        if (key) {
            gfx_SetColor(255);
            gfx_FillRectangle_NoClip(1, selectionOffset, 8, 8);

            // Stop and quit
            if (key == sk_Clear) {
                goto err;
            }

            // Select the next program
            if (key == sk_Down && selectedProgram != amountOfPrograms) {
                selectedProgram++;
            }
            
            // Select the previous program
            if (key == sk_Up && selectedProgram != 1) {
                selectedProgram--;
            }
        }
    }
    
    // Erase screen
    gfx_SetColor(255);
    gfx_FillRectangle_NoClip(0, 11, 320, 229);
    
    // Grab the right program
    search_pos = NULL;
    while(((var_name = ti_DetectVar(&search_pos, ICEheader, TI_PRGM_TYPE)) != NULL) && --selectedProgram);
    strcpy(ice.inName, var_name);
    
    sprintf(buf, "Compiling program %s...", var_name);
    gfx_PrintStringXY(buf, 1, iceMessageLine);
    
    displayLoadingBarFrame();

    // Find program
    ti_CloseAll();
    ice.inPrgm = ti_OpenVar(ice.inName, "r", TI_PRGM_TYPE);
    if (!ice.inPrgm) {
        goto stop;
    }
#else
    var_name = argv[1];
    if (argc != 2) {
        fprintf(stderr, "Error: Missing program as input\n");
        exit(1);
    }
    fprintf(stdout, "%s\n", infoStr);
    fprintf(stdout, "Compiling program %s...\n", var_name);
    ice.inPrgm = fopen(var_name, "rb");
    if (!ice.inPrgm) {
        fprintf(stdout, "Can't find input program");
        goto stop;
    }
    
    fseek(ice.inPrgm, 0, SEEK_END);
    ice.programLength = ftell(ice.inPrgm);
    fseek(ice.inPrgm, 0, SEEK_SET);
    
#endif
    
    // Setup pointers and header
#ifndef COMPUTER_ICE
    ice.programData    = (uint8_t*)0xD52C00;
#else
    ice.programData    = malloc(50000);
#endif
    ice.programPtr     = ice.programData + 116;
    ice.programDataPtr = ice.programDataData;
    ice.LblPtr         = ice.LblStack;
    ice.GotoPtr        = ice.GotoStack;
    
    memcpy(ice.programData, CHeaderData, 116);
    
    // Pre-scan program (and subprograms) and find all the C routines
    preScanProgram(ice.inPrgm);
    
    // If there are no C functions, remove the entire header
    if (!ice.amountOfCRoutinesUsed) {
        ice.programPtr = (uint8_t*)ice.programData;
    }
    
    memcpy(ice.programPtr, CProgramHeaderData, 5);
    ice.programPtr += 5;
    
    ice.programSize = (uintptr_t)ice.programPtr - (uintptr_t)ice.programData;
   
    // Do the stuff
    resetFileOrigin(ice.inPrgm);
#ifndef COMPUTER_ICE
    displayLoadingBarFrame();
#endif
    res = parseProgram(ice.inPrgm);
    
    // Create or empty the output program if parsing succeeded
    if (res == VALID) {
        // If we modified IY, restore it
        if (ice.modifiedIY) {
            LD_IY_IMM(flags);
        }
        
        // If the last token is not "Return", write a "ret" to the program
        if (!ice.lastTokenIsReturn) {
            RET();
        }
        
        // Get the sizes of the 3 stacks
        ice.programSize = (uintptr_t)ice.programPtr - (uintptr_t)ice.programData;
        programDataSize = (uintptr_t)ice.programDataPtr - (uintptr_t)ice.programDataData;
        
        // Change the pointers to the data as well, but first calculate the offset
        offset = PRGM_START + ice.programSize - (uintptr_t)ice.programDataData;
        while (ice.dataOffsetElements--) {
            *ice.dataOffsetStack[ice.dataOffsetElements] += offset;
        }
        
        // Find all the matching Goto's/Lbl's
        while (ice.GotoPtr != ice.GotoStack) {
            uint24_t GotoAddr  = *--ice.GotoPtr;
            uint24_t GotoPtr = *--ice.GotoPtr;
            uint24_t *temp;
            
            // Check for every label if it matches the Goto
            for (temp = ice.LblStack; temp < ice.LblPtr;) {
                uint24_t LblPtr = *temp++;
                uint24_t LblAddr = *temp++;
                
                // Compare the Goto and the Lbl labels
                if (!memcmp((char*)GotoAddr, (char*)LblAddr, (uint24_t)strchr((char*)LblAddr, tEnter) - LblAddr)) {
                    *(uint24_t*)(GotoPtr + 1) = LblPtr - (uint24_t)ice.programData + PRGM_START;
                    goto findNextLabel;
                }
            }
            
            // Lbl not found
            displayError(E_NO_LABEL);
            goto stop;
findNextLabel:;
        }
        
        totalSize = ice.programSize + programDataSize + 3;
        
#ifndef COMPUTER_ICE
        ice.outPrgm = ti_OpenVar(ice.outName, "w", TI_PPRGM_TYPE);
        if (!ice.outPrgm) {
            goto stop;
        }
        
        // Write ASM header
        ti_PutC(tExtTok, ice.outPrgm);
        ti_PutC(tAsm84CeCmp, ice.outPrgm);
        
        // Write ICE header to be recognized by Cesium
        ti_PutC(0x7F, ice.outPrgm);
        
        // Write the header, main program, and data to output :D
        if (ice.programSize) ti_Write(ice.programData, ice.programSize, 1, ice.outPrgm);
        if (programDataSize) ti_Write(ice.programDataData, programDataSize, 1, ice.outPrgm);
        
        // Yep, we are really done!
        gfx_SetTextFGColor(4);
        gfx_PrintStringXY("Succesfully compiled!", 1, iceMessageLine);
        
        // Skip line
        iceMessageNewLine();
        
        // Display the size
        gfx_SetTextFGColor(0);
        sprintf(buf, "Output size: %u bytes", totalSize);
        gfx_PrintStringXY(buf, 1, iceMessageLine);
#else
        uint8_t *export = malloc(0x10000);
        
        // Write ASM header
        export[0] = tExtTok;
        export[1] = tAsm84CeCmp;
        
        // Write ICE header to be recognized by Cesium
        export[2] = 0x7F;
        
        // Write the header, main program, and data to output :D
        memcpy(&export[3], ice.programData, ice.programSize);
        memcpy(&export[3+ice.programSize], ice.programDataData, programDataSize);
        
        // Write the actual program file
        export_program(ice.outName, export, totalSize);
        free(export);
        
        // Display the size
        fprintf(stdout, "Succesfully compiled %s.8xp!\n", ice.outName);
        fprintf(stdout, "Output size: %u bytes\n", totalSize);
#endif
    } else {
        displayError(res);
    }

#ifndef COMPUTER_ICE
stop:
    gfx_SetTextFGColor(0);
    gfx_PrintStringXY("[Press any key to quit]", 85, 230);
    while (!os_GetCSC());
err:
    // Return immediately
    ti_CloseAll();
    gfx_End();
    prgm_CleanUp();
#else
    return 0;
stop:
    return 1;
#endif
}

void preScanProgram(ti_var_t currentProgram) {
    uint24_t token;
    
#ifdef COMPUTER_ICE
    resetFileOrigin(currentProgram);
#endif
    
    // Scan the entire program
    while ((int)(token = __getc()) != EOF) {
        uint8_t tok = (uint8_t)token;
        
        if (tok == tString) {
            expr.inString = !expr.inString;
        } else if (tok == tEnter) {
            expr.inString = false;
        } else if (tok == tii && !expr.inString) {
            while ((int)(token = __getc()) != EOF && (uint8_t)token != tEnter);
        } else if (tok == tStore) {
            expr.inString = false;
        } else if (tok == tDet && !expr.inString) {
            uint8_t tok1 = __getc();
            uint8_t tok2 = __getc();

            // Invalid det( command
            if (tok1 < t0 || tok1 > t9) {
                break;
            }
            
            // Get the det( command
            if (tok2 < t0 || tok2 > t9) {
                tok = tok1 - t0;
            } else {
                tok = (tok1 - t0) * 10 + (tok2 - t0);
            }
            
            // Insert the C routine
            if (!ice.CRoutinesStack[tok]) {
                JP(tok * 3);
                ice.CRoutinesStack[tok] = ice.amountOfCRoutinesUsed++;
            }
        }
    }
    
    // Well, we scanned the entire program, so let's rewind it
    resetFileOrigin(currentProgram);
}

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

uint8_t GetHexadecimal(ti_var_t currentProgram) {
    uint8_t tok = (uint8_t)__getc();
    if (tok >= t0 && tok <= t9) {
        return tok - t0;
    } else if (tok >= tA && tok <= tF) {
        return tok - tA + 10;
    } else {
        return 16;
    }
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
#endif

unsigned int getNextToken(ti_var_t currentProgram) {
#ifndef COMPUTER_ICE
    // Display loading bar
    displayLoadingBar(currentProgram);
    return ti_GetC(currentProgram);
#else
    if (ftell(ice.inPrgm) < ice.programLength - 2) {
        return getc(currentProgram);
    }
    return EOF;
#endif
}

void setCurrentOffset(int offset, int origin, ti_var_t stream) {
#ifndef COMPUTER_ICE
    ti_Seek(offset, origin, stream);
#else
    fseek(stream, offset, origin);
#endif
}

unsigned int getCurrentOffset(ti_var_t current) {
#ifndef COMPUTER_ICE
    return ti_Tell(current);
#else
    return ftell(current);
#endif
}
    
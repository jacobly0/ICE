#include "main.h"

#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "output.h"
#include "operator.h"

#include <fileioc.h>
#include <graphx.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>

ice_t ice;
expr_t expr;

#define LB_X 40
#define LB_Y 200
#define LB_W 240
#define LB_H 14
#define LB_R (LB_H / 2)

void main() {
    uint8_t a = 0, selectedProgram = 0, key, amountOfPrograms, res, *outputDataPtr, *search_pos = NULL;
    char *var_name;
    uint24_t token, headerSize, programDataSize, offset;
    const char ICEheader[] = {tii, 0};
    char buf[30];

    // Yay, GUI! :)
    gfx_Begin(gfx_8bpp);
    gfx_SetColor(189);
    gfx_FillRectangle_NoClip(0, 0, 320, 10);
    gfx_SetColor(0);
    gfx_SetTextFGColor(0);
    gfx_HorizLine_NoClip(0, 10, 320);
    gfx_PrintStringXY("ICE Compiler v1.6 - By Peter \"PT_\" Tillema", 21, 1);
    
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
    
    // Display fancy loading bar during compiling ;)
    gfx_SetColor(0);
    gfx_Circle_NoClip(LB_X, LB_Y, LB_R);
    gfx_Circle_NoClip(LB_X + LB_W, LB_Y, LB_R);
    gfx_HorizLine_NoClip(LB_X, LB_Y - LB_R, LB_W);
    gfx_HorizLine_NoClip(LB_X, LB_Y + LB_R, LB_W);
    gfx_SetColor(255);
    gfx_FillRectangle_NoClip(LB_X, LB_Y - LB_R + 1, LB_R + 1, LB_H - 1);
    gfx_FillRectangle_NoClip(LB_X + LB_W - LB_R, LB_Y - LB_R + 1, LB_R + 1, LB_H - 1);

    // Find program
    ti_CloseAll();
    ice.inPrgm = ti_OpenVar(ice.inName, "r", TI_PRGM_TYPE);
    if (!ice.inPrgm) {
        goto stop;
    }

    // Setup pointers and header
    ice.programData    = (uint8_t*)0xD52C00;
    ice.programPtr     = (uint8_t*)ice.programData + 116;
    ice.programDataPtr = (uint8_t*)ice.programDataData;
    
    memcpy(ice.programData, CHeaderData, 116);
    
    // Pre-scan program (and subprograms) and find all the C routines
    preScanProgram(ice.inPrgm);
    
    // If there are no C functions, remove the entire header
    if (!ice.amountOfCRoutinesUsed) {
        ice.programPtr = (uint8_t*)ice.programData;
    }
    
    memcpy(ice.programPtr, CProgramHeader, 5);
    ice.programPtr += 5;
    
    ice.programSize = (uint24_t)ice.programPtr - (uint24_t)ice.programData;
   
    // Do the stuff
    ti_Rewind(ice.inPrgm);
    res = parseProgram(ice.inPrgm);
    
    // Create or empty the output program if parsing succeeded
    if (res == VALID) {
        ice.outPrgm = ti_OpenVar(ice.outName, "w", TI_PPRGM_TYPE);
        if (!ice.outPrgm) {
            goto stop;
        }
        
        // If we modified IY, restore it
        if (ice.modifiedIY) {
            LD_IY_IMM(0xD00080);
        }
        
        // If the last token is not "Return", write a "ret" to the program
        if (!ice.lastTokenIsReturn) {
            RET();
        }
        
        // Get the sizes of the 3 stacks
        ice.programSize = (uint24_t)ice.programPtr - (uint24_t)ice.programData;
        programDataSize = (uint24_t)ice.programDataPtr - (uint24_t)ice.programDataData;
        
        // Change the pointers to the data as well, but first calculate the offset
        offset = PRGM_START + ice.programSize - (uint24_t)ice.programDataData;
        while (ice.dataOffsetElements--) {
            *ice.dataOffsetStack[ice.dataOffsetElements] += offset;
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
        sprintf(buf, "Output size: %u bytes", ice.programSize + programDataSize + 2);
        gfx_PrintStringXY(buf, 1, iceMessageLine);
    } else {
        displayError(res);
    }
    
stop:
    gfx_SetTextFGColor(0);
    gfx_PrintStringXY("[Press any key to quit]", 85, 230);
    while (!os_GetCSC());
err:
    // Return immediately
    ti_CloseAll();
    gfx_End();
    prgm_CleanUp();
}

void preScanProgram(ti_var_t currentProgram) {
    uint24_t token;
    
    // Scan the entire program
    while ((token = getc()) != EOF) {
        uint8_t tok = (uint8_t)token;
        
        if (tok == tString) {
            expr.inString = !expr.inString;
        } else if (tok == tEnter) {
            expr.inString = false;
        } else if (tok == tii && !expr.inString) {
            while ((token = getc()) != EOF && (uint8_t)token != tEnter);
        } else if (tok == tStore) {
            expr.inString = false;
        } else if (tok == tDet && !expr.inString) {
            uint8_t tok1 = getc();
            uint8_t tok2 = getc();

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
    ti_Rewind(currentProgram);
}

void ProgramPtrToOffsetStack(void) {
    ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(ice.programPtr + 1);
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
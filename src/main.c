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

ice_t ice;

void main() {
    uint8_t a = 0, selectedProgram = 0, key, amountOfPrograms, res;
    unsigned int token, headerSize, programSize, programDataSize;
    char *var_name;
    uint8_t *outputDataPtr;
    const char headerStart[] = {tii, 0};
    uint8_t *search_pos = NULL;

    // Yay, GUI! :)
    gfx_Begin(gfx_8bpp);
    gfx_SetColor(189);
    gfx_FillRectangle_NoClip(0, 0, 320, 10);
    gfx_SetColor(0);
    gfx_HorizLine_NoClip(0, 10, 320);
    gfx_PrintStringXY("ICE Compiler v1.5 - By Peter \"PT_\" Tillema", 21, 1);
    
    // Get all the programs that start with the [i] token
    while((var_name = ti_DetectVar(&search_pos, headerStart, TI_PRGM_TYPE)) != NULL && ++selectedProgram <= 22) {
        gfx_PrintStringXY(var_name, 10, selectedProgram*10 + 3);
    }
    amountOfPrograms = selectedProgram;
    
    // Check if there are ICE programs
    if (!amountOfPrograms) {
        gfx_PrintStringXY("No programs found!", 10, 13);
        goto err;
    }
    
    // Select a program
    selectedProgram = 1;
    while ((key = os_GetCSC()) != sk_Enter) {
        uint8_t selectionOffset = selectedProgram*10 + 3;

        gfx_SetColor(0);
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
    
    // Grab the right program
    search_pos = NULL;
    while(((var_name = ti_DetectVar(&search_pos, headerStart, TI_PRGM_TYPE)) != NULL) && --selectedProgram);
    strcpy(ice.inName, var_name);

    // Find program
    ti_CloseAll();
    ice.inPrgm = ti_OpenVar(ice.inName, "r", TI_PRGM_TYPE);
    if (!ice.inPrgm) {
        goto err;
    }

    // Setup pointers and header
    ice.programData     = (uint8_t*)0xD52C00;

    ice.headerPtr       = (uint8_t*)ice.headerData + 116;
    ice.programPtr      = (uint8_t*)ice.programData + 5;
    ice.programDataPtr  = (uint8_t*)ice.programDataData;
    
    memcpy(ice.headerData, CHeaderData, 116);
    memcpy(ice.programData, CProgramHeader, 5);
   
    // Do the stuff
    res = parseProgram();
    
    dbg_Debugger();
    
    // Create or empty the output program if parsing succeeded
    if (res == VALID) {
        ice.outPrgm = ti_OpenVar(ice.outName, "w", TI_PPRGM_TYPE);
        if (!ice.outPrgm) {
            goto err;
        }
        
        // Get the sizes of the 3 stacks
        headerSize = (uint24_t)ice.headerPtr - (uint24_t)ice.headerData;
        programSize = (uint24_t)ice.programPtr - (uint24_t)ice.programData;
        programDataSize = (uint24_t)ice.programDataPtr - (uint24_t)ice.programDataData;
        
        // Write ASM header
        ti_PutC(tExtTok, ice.outPrgm);
        ti_PutC(tAsm84CeCmp, ice.outPrgm);
        
        // Write the header, main program, and data to output :D
        ti_Write(ice.headerData, headerSize, 1, ice.outPrgm);
        ti_Write(ice.programData, programSize, 1, ice.outPrgm);
        ti_Write(ice.programDataData, programDataSize, 1, ice.outPrgm);
    } else {
        displayError(res);
    }

err:
    gfx_PrintStringXY("[Press any key to quit]", 85, 230);
    while (!os_GetCSC());
    ti_CloseAll();
    gfx_End();
    prgm_CleanUp();
}


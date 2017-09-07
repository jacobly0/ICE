#include "defines.h"
#include "main.h"

#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "output.h"
#include "operator.h"
#include "routines.h"
//#include "gfx/gfx_logos.h"

ice_t ice;
expr_t expr;

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
int main(int argc, char **argv) {
#endif
    uint8_t a = 0, selectedProgram = 0, key, amountOfPrograms, res, *hooksPtr, *search_pos = NULL;
    uint24_t token, headerSize, programDataSize, offset, totalSize;
    const char ICEheader[] = {tii, 0};
    char buf[30], *var_name;
    
#ifndef COMPUTER_ICE  
    // Install hooks
    ti_CloseAll();
    ice.inPrgm = ti_OpenVar("ICEAPPV", "r", TI_APPVAR_TYPE);
    if (ice.inPrgm) {
        ti_SetArchiveStatus(true, ice.inPrgm);
        hooksPtr = ti_GetDataPtr(ice.inPrgm);
        
        // Manually set the hooks
        asm("ld de, 17");
        asm("add hl, de");
        asm("call 00213CCh");
        asm("ld de, 473");
        asm("add hl, de");
        asm("call 00213F8h");
        asm("ld de, 31");
        asm("add hl, de");
        asm("call 00213C4h");
    }
    
    // Yay, GUI! :)
    gfx_Begin(gfx_8bpp);
    
    // Display flash screen
    /*gfx_ZeroScreen();
    gfx_RLETSprite_NoClip(logo, 53, 70);
    delay(2000);
    gfx_FillScreen(0xFF);*/

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
    
    gfx_PrintStringXY("Prescanning...", 1, iceMessageLine);
    displayLoadingBarFrame();
#else
    var_name = argv[1];
    if (argc != 2) {
        fprintf(stderr, "Error: Missing program as input\n");
        exit(1);
    }
    fprintf(stdout, "%s\nPrescanning...\n", infoStr);
#endif

    ice.inPrgm = _open(var_name);
    ice.inPrgm2 = _open(var_name);
    if (!ice.inPrgm) {
#ifdef COMPUTER_ICE
        fprintf(stdout, "Can't find input program");
#endif
        goto stop;
    }
    _seek(0, SEEK_END, ice.inPrgm2);
    ice.programLength = _tell(ice.inPrgm2);
    
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
    preScanProgram();
    
    // If there are no C functions, remove the entire header
    if (!ice.amountOfCRoutinesUsed) {
        ice.programPtr = (uint8_t*)ice.programData;
    }
    
    memcpy(ice.programPtr, CProgramHeaderData, 5);
    ice.programPtr += 5;
    
    ice.programSize = (uintptr_t)ice.programPtr - (uintptr_t)ice.programData;
   
    // Do the stuff
#ifndef COMPUTER_ICE
    sprintf(buf, "Compiling program %s...", var_name);
    gfx_PrintStringXY(buf, 1, iceMessageLine);
    displayLoadingBarFrame();
#else
    fprintf(stdout, "Compiling program %s...\n", var_name);
#endif
    res = parseProgram();
    
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
            uint24_t *temp, GotoAddr;
            
            _seek(*--ice.GotoPtr, SEEK_SET, ice.inPrgm);
            GotoAddr = *--ice.GotoPtr;
            
            // Check all the labels
            for (temp = ice.LblStack; temp < ice.LblPtr;) {
                int tok1, tok2;
                uint24_t LblAddr = *temp++;
                
                _seek(*temp++, SEEK_SET, ice.inPrgm2);
                
                // Check if the labels are the same
                do {
                    tok1 = _getc(ice.inPrgm);
                    tok2 = _getc(ice.inPrgm2);
                } while ((uint8_t)tok1 != tEnter && tok1 != EOF && (uint8_t)tok2 != tEnter && tok2 != EOF && (uint8_t)tok1 == (uint8_t)tok2);
                
                if (((uint8_t)tok1 == tEnter || tok1 == EOF) && ((uint8_t)tok2 == tEnter || tok2 == EOF)) {
                    w24((uint8_t*)GotoAddr + 1, LblAddr - (uint24_t)ice.programData + PRGM_START);
                    goto findNextLabel;
                }
            }
            
            // Label not found :(
            displayError(E_NO_LABEL);
            goto stop;
findNextLabel:;
        }
        totalSize = ice.programSize + programDataSize + 3;
        
        ice.outPrgm = _new(ice.outName);
        
#ifndef COMPUTER_ICE
        if (!ice.outPrgm) {
            gfx_PrintStringXY("Failed to open output file", 1, iceMessageLine);
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
        if (!ice.outPrgm) {
            fprintf(stdout, "Failed to open output file");
            goto stop;
        }
        
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
        fprintf(stdout, "Succesfully compiled to %s.8xp!\n", ice.outName);
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

void preScanProgram(void) {
    uint24_t token;
    
    _rewind(ice.inPrgm);
    
    // Scan the entire program
    while ((int)(token = _getc(ice.inPrgm)) != EOF) {
        uint8_t tok = (uint8_t)token;
        
        if (tok == tString) {
            expr.inString = !expr.inString;
        } else if (tok == tEnter) {
            expr.inString = false;
        } else if (tok == tii && !expr.inString) {
            skipLine();
        } else if (tok == tStore) {
            expr.inString = false;
        } else if (tok == tVarLst && !expr.inString) {
            if (!ice.OSLists[token = _getc(ice.inPrgm)]) {
                ice.OSLists[token] = pixelShadow + 2000 * (ice.amountOfOSLocationsUsed++);
            }
        } else if (tok == tVarStrng && !expr.inString) {
            if (!ice.OSStrings[token = _getc(ice.inPrgm)]) {
                ice.OSStrings[token] = pixelShadow + 2000 * (ice.amountOfOSLocationsUsed++);
            }
        } else if (tok == tVarOut && !expr.inString) {
            // CompilePrgm(
            if ((uint8_t)_getc(ice.inPrgm) == 0x0D) {
                char tempName[9];
                uint8_t a = 0;
                ti_var_t tempProg = ice.inPrgm;

                while ((int)(token = _getc(ice.inPrgm)) != EOF && (tok = (uint8_t)token) != tEnter && a < 9) {
                    tempName[a++] = (char)tok;
                }
                tempName[a] = 0;
                
                if ((ice.inPrgm = _open(tempName))) {
#ifndef COMPUTER_ICE
                    displayLoadingBarFrame();
                    preScanProgram();
                    ti_Close(ice.inPrgm);
                    displayLoadingBarFrame();
#endif
                }
                ice.inPrgm = tempProg;
            }
        } else if (tok == tDet && !expr.inString) {
            uint8_t tok1 = _getc(ice.inPrgm);
            uint8_t tok2 = _getc(ice.inPrgm);

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
    _rewind(ice.inPrgm);
}

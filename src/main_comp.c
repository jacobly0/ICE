#include "defines.h"
#include "main.h"

#ifdef COMPUTER_ICE

#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "output.h"
#include "operator.h"
#include "routines.h"

ice_t ice;
expr_t expr;
reg_t reg;

const char *infoStr = "ICE Compiler v2.1 - By Peter \"PT_\" Tillema";
extern label_t labelStack[150];
extern label_t gotoStack[150];

#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(SRand, "src/asm/rand.bin");
INCBIN(Cheader, "src/asm/cheader.bin");
INCBIN(Fileiocheader, "src/asm/fileiocheader.bin");

extern char *str_dupcat(const char *s, const char *c);

void w24(void *x, uint32_t val) {
    uint8_t *ptr = (uint8_t*)(x);
    ptr[0] = val & 0xFF;
    ptr[1] = val >> 8 & 0xFF;
    ptr[2] = val >> 16 & 0xFF;
}

void w16(void *x, uint32_t val) {
    uint8_t *ptr = (uint8_t*)(x);
    ptr[0] = val & 0xFF;
    ptr[1] = val >> 8 & 0xFF;
}

uint32_t r24(void *x) {
    uint8_t *ptr = (uint8_t*)(x);
    return (ptr[2] << 16) | (ptr[1] << 8) | (ptr[0]);
}

int main(int argc, char **argv) {
    uint8_t amountOfPrograms, res = VALID;
    uint24_t programDataSize, offset, totalSize;
    char buf[30], *var_name;

    var_name = argv[1];
    if (argc != 2) {
        fprintf(stderr, "Error: Missing program as input\n");
        exit(1);
    }
    
    ice.inPrgm = _open(var_name);
    if (!ice.inPrgm) {
        fprintf(stdout, "Can't find input program\n");
        goto stop;
    }
    fprintf(stdout, "%s\nPrescanning...\n", infoStr);
    _seek(0, SEEK_END, ice.inPrgm);
    
    ice.programLength   = _tell(ice.inPrgm);
    ice.programData     = malloc(0xFFFF + 0x100);
    ice.programPtr      = ice.programData + SIZEOF_CHEADER;
    ice.programDataData = ice.programData + 0xFFFF;
    ice.programDataPtr  = ice.programDataData;
    ice.LblPtr          = ice.LblStack;
    ice.GotoPtr         = ice.GotoStack;
    ice.CBaseAddress    = ice.programPtr;
    
    // Pre-scan program (and subprograms) and find all the GRAPHX routines
    memcpy(ice.programData, CheaderData, SIZEOF_CHEADER);
    preScanProgram(ice.GraphxRoutinesStack, &ice.amountOfGraphxRoutinesUsed, true);
    
    // If there are no GRAPHX functions, remove the GRAPHX header
    if (!ice.amountOfGraphxRoutinesUsed) {
        ice.programPtr -= 9;
        ice.CBaseAddress -= 9;
    }
    
    // Prescan the program again to detect all the FILEIOC routines
    memcpy(ice.programPtr, FileiocheaderData, 10);
    ice.programPtr += 10;
    preScanProgram(ice.FileiocRoutinesStack, &ice.amountOfFileiocRoutinesUsed, false);
    
    // If there are no GRAPHX functions, remove the GRAPHX header
    if (!ice.amountOfFileiocRoutinesUsed) {
        ice.programPtr -= 10;
        
        // No C function at all
        if (!ice.amountOfGraphxRoutinesUsed) {
            ice.programPtr = ice.programData;
        }
    }
    
    ice.CBaseAddress -= ice.programData - (uint8_t*)PRGM_START;
    
    // Clear up program before and after running
    if (ice.amountOfGraphxRoutinesUsed || ice.amountOfFileiocRoutinesUsed) {
        CALL(_RunIndicOff);
        CALL(ice.programPtr - ice.programData + (ice.amountOfGraphxRoutinesUsed ? 12 : 9) + PRGM_START);
        LD_IY_IMM(flags);
        if (ice.amountOfGraphxRoutinesUsed) {
            JP(_DrawStatusBar);
        } else {
            RET();
        }
    } else {
        CALL(ice.programPtr - ice.programData + 9 + PRGM_START);
        LD_IY_IMM(flags);
        RET();
    }
    
    // Sorry :3
    ice.freeMemoryPtr = (ice.tempStrings[1] = (ice.tempStrings[0] = pixelShadow + 2000 * ice.amountOfOSLocationsUsed) + 2000) + 2000;
    
    LD_IX_IMM(IX_VARIABLES);
    
    // Eventually seed the rand
    if (ice.usesRandRoutine) {
        ice.programDataPtr -= SIZEOF_RAND_DATA;
        ice.randAddr = (uint24_t)ice.programDataPtr;
        memcpy(ice.programDataPtr, SRandData, SIZEOF_RAND_DATA);
        ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(ice.randAddr + 2);
        w24((uint8_t*)(ice.randAddr + 2), ice.randAddr + 102);
        ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(ice.randAddr + 6);
        w24((uint8_t*)(ice.randAddr + 6), ice.randAddr + 105);
        ice.dataOffsetStack[ice.dataOffsetElements++] = (uint24_t*)(ice.randAddr + 19);
        w24((uint8_t*)(ice.randAddr + 19), ice.randAddr + 102);
        
        LD_HL_IND(0xF30044);
        ProgramPtrToOffsetStack();
        CALL((uint24_t)ice.programDataPtr);
    }
   
    // Do the stuff
    fprintf(stdout, "Compiling program %s...\n", var_name);
    res = parseProgram();
    
    // Create or empty the output program if parsing succeeded
    if (res == VALID) {
        uint8_t currentGoto, currentLbl;
        uint24_t previousSize = 0;
        
        // If the last token is not "Return", write a "ret" to the program
        if (!ice.lastTokenIsReturn) {
            RET();
        }
        
        // Find all the matching Goto's/Lbl's
        for (currentGoto = 0; currentGoto < ice.amountOfGotos; currentGoto++) {
            label_t *curGoto = &gotoStack[currentGoto];
            
            for (currentLbl = 0; currentLbl < ice.amountOfLbls; currentLbl++) {
                label_t *curLbl = &labelStack[currentLbl];
                
                if (!memcmp(curLbl->name, curGoto->name, 10)) {
                    w24((uint8_t*)(curGoto->addr + 1), curLbl->addr - (uint24_t)ice.programData + PRGM_START);
                    goto findNextLabel;
                }
            }
            
            // Label not found
            displayLabelError(curGoto->name);
            _seek(curGoto->offset, SEEK_SET, ice.inPrgm);
            res = 0;
            goto stop;
findNextLabel:;
        }
        
        // Get the sizes of both stacks
        ice.programSize = (uintptr_t)ice.programPtr - (uintptr_t)ice.programData;
        programDataSize = (uintptr_t)ice.programDataData - (uintptr_t)ice.programDataPtr;
        
        // Change the pointers to the data as well, but first calculate the offset
        offset = PRGM_START + ice.programSize - (uintptr_t)ice.programDataPtr;
        while (ice.dataOffsetElements--) {
            uint24_t *tempDataOffsetStackPtr = ice.dataOffsetStack[ice.dataOffsetElements];
            
            *tempDataOffsetStackPtr += offset;
        }
        totalSize = ice.programSize + programDataSize + 3;
        
        if (ice.startedGRAPHX && !ice.endedGRAPHX) {
            displayError(W_CLOSE_GRAPHX);
        }
        
        uint8_t *export = malloc(0x10000);
        
        // Write ASM header
        export[0] = tExtTok;
        export[1] = tAsm84CeCmp;
        
        // Write ICE header to be recognized by Cesium
        export[2] = 0x7F;
        
        // Write the header, main program, and data to output :D
        memcpy(&export[3], ice.programData, ice.programSize);
        memcpy(&export[3 + ice.programSize], ice.programDataPtr, programDataSize);
        
        // Write the actual program file
        export_program(ice.outName, export, totalSize);
        free(export);
        
        // Display the size
        fprintf(stdout, "Succesfully compiled to %s.8xp!\n", ice.outName);
        fprintf(stdout, "Output size: %u bytes\n", totalSize);
    } else {
        displayError(res);
    }
    return 0;
stop:
    return 1;
}

void preScanProgram(uint24_t CFunctionsStack[], uint8_t *CFunctionsCounter, bool detectOSVars) {
    int token;
    
    _rewind(ice.inPrgm);
    
    // Scan the entire program
    while ((int)(token = _getc()) != EOF) {
        uint8_t tok = (uint8_t)token;
        
        if (tok == tString) {
            expr.inString = !expr.inString;
        } else if (tok == tEnter) {
            expr.inString = false;
        } else if (tok == tii && !expr.inString) {
            skipLine();
        } else if (tok == tStore) {
            expr.inString = false;
        } else if (tok == tVarLst && !expr.inString && detectOSVars) {
            if (!ice.OSLists[token = _getc()]) {
                ice.OSLists[token] = pixelShadow + 2000 * (ice.amountOfOSLocationsUsed++);
            }
        } else if (tok == tRand) {
            ice.usesRandRoutine = true;
        } else if (tok == tExtTok) {
            if ((uint8_t)_getc() == tRandInt) {
                ice.usesRandRoutine = true;
            }
        } else if (tok == tVarStrng && !expr.inString && detectOSVars) {
            if (!ice.OSStrings[token = _getc()]) {
                ice.OSStrings[token] = pixelShadow + 2000 * (ice.amountOfOSLocationsUsed++);
            }
        } else if (tok == t2ByteTok && !expr.inString) {
            // AsmComp(
            if ((uint8_t)_getc() == tAsmComp) {
                char tempName[9] = {0};
                uint8_t a = 0;
                ti_var_t tempProg = ice.inPrgm;

                while ((int)(token = _getc()) != EOF && (tok = (uint8_t)token) != tEnter && a < 9) {
                    tempName[a++] = tok;
                }
                tempName[a] = 0;
                
                if ((ice.inPrgm = _open(str_dupcat(tempName, ".8xp")))) {
                    preScanProgram(CFunctionsStack, CFunctionsCounter, detectOSVars);
                    fclose(ice.inPrgm);
                }
                ice.inPrgm = tempProg;
            }
        } else if (((tok == tDet && detectOSVars) || (tok == tSum && !detectOSVars)) && !expr.inString) {
            uint8_t tok1 = _getc();
            uint8_t tok2 = _getc();

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
            if (!CFunctionsStack[tok]) {
                CFunctionsStack[tok] = ice.programPtr - ice.CBaseAddress;
                JP(tok * 3);
                (*CFunctionsCounter)++;
            }
        }
    }
    
    _rewind(ice.inPrgm);
}

#endif

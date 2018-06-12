#include "defines.h"
#include "main.h"

#if !defined(COMPUTER_ICE) && !defined(__EMSCRIPTEN__)

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

const char *infoStr = "ICE Compiler v2.1.3.3 - By Peter \"PT_\" Tillema";
const uint8_t colorTable[16] = {255,24,224,0,248,36,227,97,9,19,230,255,181,107,106,74};    // Thanks Cesium :D
char *inputPrograms[22];
extern label_t labelStack[150];
extern label_t gotoStack[150];

static int myCompare(const void * a, const void * b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void main(void) {
    uint8_t selectedProgram, amountOfPrograms, res = VALID, temp;
    uint24_t programDataSize, offset, totalSize;
    const char ICEheader[] = {tii, 0};
    char buf[30], *temp_name = "", var_name[9];
    sk_key_t key;
    void *search_pos;
    bool didCompile;
    prog_t *outputPrgm;

    // Install hooks
    ti_CloseAll();
    ice.inPrgm = ti_Open("ICEAPPV", "r");

    if (ice.inPrgm) {
        ti_SetArchiveStatus(true, ice.inPrgm);
        ti_GetDataPtr(ice.inPrgm);

        // Manually set the hooks
        asm("ld de, 17");
        asm("add hl, de");
        asm("call 00213CCh");
        asm("ld de, 492");
        asm("add hl, de");
        asm("call 00213F8h");
        asm("ld de, 32");
        asm("add hl, de");
        asm("call 00213C4h");
    }

    asm("ld iy, 0D00080h");
    asm("set 3, (iy+024h)");

    // Yay, GUI! :)
displayMainScreen:
    gfx_Begin();

    gfx_SetColor(189);
    gfx_FillRectangle_NoClip(0, 0, 320, 10);
    gfx_SetColor(0);
    gfx_SetTextFGColor(0);
    gfx_HorizLine_NoClip(0, 10, 320);
    gfx_PrintStringXY(infoStr, 12, 1);

    // Get all the programs that start with the [i] token
    selectedProgram = 0;
    didCompile = false;
    ti_CloseAll();

    for (temp = TI_PRGM_TYPE; temp <= TI_PPRGM_TYPE; temp++) {
        search_pos = NULL;
        while((temp_name = ti_DetectVar(&search_pos, ICEheader, temp)) && selectedProgram <= 22) {
            if ((uint8_t)(*temp_name) < 64) {
                *temp_name += 64;
            }

            // Save the program name
            inputPrograms[selectedProgram] = malloc(9);
            strcpy(inputPrograms[selectedProgram++], temp_name);
        }
    }

    amountOfPrograms = selectedProgram;

    // Check if there are ICE programs
    if (!amountOfPrograms) {
        gfx_PrintStringXY("No programs found!", 10, 13);
        goto stop;
    }

    // Display all the sorted programs
    qsort(inputPrograms, amountOfPrograms, sizeof(char *), myCompare);
    for (temp = 0; temp < amountOfPrograms; temp++) {
        gfx_PrintStringXY(inputPrograms[temp], 10, temp * 10 + 13);
    }
    
    // Display buttons
    gfx_PrintStringXY("Build", 4, 232);
    printButton(1);
    gfx_PrintStringXY("Debug", 66, 232);
    printButton(65);
    gfx_PrintStringXY("Quit", 285, 232);
    printButton(279);
    gfx_SetColor(0);

    // Select a program
    selectedProgram = 1;
    while ((key = os_GetCSC()) != sk_Enter && key != sk_2nd && key != sk_Yequ && key != sk_Window) {
        uint8_t selectionOffset = selectedProgram * 10 + 3;

        gfx_PrintStringXY(">", 1, selectionOffset);

        if (key) {
            gfx_SetColor(255);
            gfx_FillRectangle_NoClip(1, selectionOffset, 8, 8);

            // Stop and quit
            if (key == sk_Clear || key == sk_Graph) {
                goto err;
            }

            // Select the next program
            if (key == sk_Down) {
                if (selectedProgram != amountOfPrograms) {
                    selectedProgram++;
                } else {
                    selectedProgram = 1;
                }
            }

            // Select the previous program
            if (key == sk_Up) {
                if (selectedProgram != 1) {
                    selectedProgram--;
                } else {
                    selectedProgram = amountOfPrograms;
                }
            }
        }
    }
    
    // Output debug appvar
    if (key == sk_Window) {
        ice.debug = true;
    }

    // Erase screen
    gfx_SetColor(255);
    gfx_FillRectangle_NoClip(0, 11, 320, 210);
    gfx_FillRectangle_NoClip(0, 220, 270, 20);

    // Set some vars
    strcpy(var_name, inputPrograms[selectedProgram - 1]);
    didCompile = true;
    memset(&ice, 0, sizeof ice);

    gfx_SetTextXY(1, 12);
    displayMessageLineScroll("Prescanning...");
    displayLoadingBarFrame();

    ice.inPrgm = _open(var_name);
    _seek(0, SEEK_END, ice.inPrgm);
    strcpy(ice.currProgName[ice.inPrgm], var_name);

    ice.programLength   = _tell(ice.inPrgm);
    ice.programData     = (uint8_t*)0xD52C00;
    ice.programPtr      = ice.programData;
    ice.programDataData = ice.programData + 0xFFFF;
    ice.programDataPtr  = ice.programDataData;
    ice.LblPtr          = ice.LblStack;
    ice.GotoPtr         = ice.GotoStack;

    // Check for icon and description before putting the C functions in the output program
    preScanProgram();
    _getc();
    outputPrgm = GetProgramName();
    if (outputPrgm->errorCode != VALID) {
        displayError(outputPrgm->errorCode);
        goto stop;
    }
    strcpy(ice.outName, outputPrgm->prog);

    // Has icon
    if ((uint8_t)_getc() == tii && (uint8_t)_getc() == tString) {
        uint8_t b = 0;

        *ice.programPtr = OP_JP;
        w24(ice.programPtr + 4, 0x101001);
        ice.programPtr += 7;

        // Get hexadecimal
        do {
            if ((temp = IsHexadecimal(_getc())) == 16) {
                displayError(E_INVALID_HEX);
                goto stop;
            }
            *ice.programPtr++ = colorTable[temp];
        } while (++b);

        if ((uint8_t)_getc() != tString || (uint8_t)_getc() != tEnter) {
            displayError(E_SYNTAX);
            goto stop;
        }

        // Check description
        if ((uint8_t)_getc() == tii) {
            grabString(&ice.programPtr, false);
        }
        *ice.programPtr++ = 0;

        // Write the right jp offset
        w24(ice.programData + 1, ice.programPtr - ice.programData + PRGM_START);
    }
    
    _rewind(ice.inPrgm);

    if (prescan.hasGraphxFunctions) {
        uint8_t a;
        
        // Copy the C header to the program
        memcpy(ice.programPtr, CheaderData, SIZEOF_CHEADER);
        offset = ice.programPtr - ice.programData;
        w24(ice.programPtr + 1, r24(ice.programPtr + 1) + offset);
        w24(ice.programPtr + 52, r24(ice.programPtr + 52) + offset);
        w24(ice.programPtr + 65, r24(ice.programPtr + 65) + offset);
        
        ice.programPtr += SIZEOF_CHEADER;
        for (a = 0; a < AMOUNT_OF_GRAPHX_FUNCTIONS; a++) {
            if (prescan.GraphxRoutinesStack[a]) {
                prescan.GraphxRoutinesStack[a] = (uint24_t)ice.programPtr;
                JP(a * 3);
            }
        }
    } else if (prescan.hasFileiocFunctions) {
        memcpy(ice.programPtr, CheaderData, SIZEOF_CHEADER - 9);
        ice.programPtr += SIZEOF_CHEADER - 9;
    }

    if (prescan.hasFileiocFunctions) {
        uint8_t a;

        memcpy(ice.programPtr, FileiocheaderData, 10);
        ice.programPtr += 10;
        for (a = 0; a < AMOUNT_OF_FILEIOC_FUNCTIONS; a++) {
            if (prescan.FileiocRoutinesStack[a]) {
                prescan.FileiocRoutinesStack[a] = (uint24_t)ice.programPtr;
                JP(a * 3);
            }
        }
    }

    prescan.freeMemoryPtr = (prescan.tempStrings[1] = (prescan.tempStrings[0] = pixelShadow + 2000 * prescan.amountOfOSVarsUsed) + 2000) + 2000;

    // Cleanup code
    if (prescan.hasGraphxFunctions) {
        CALL(_RunIndicOff);
        CALL(ice.programPtr - ice.programData + PRGM_START + 12);
        LD_IY_IMM(flags);
        JP(_DrawStatusBar);
    } else if (prescan.modifiedIY) {
        CALL(ice.programPtr - ice.programData + PRGM_START + 9);
        LD_IY_IMM(flags);
        RET();
    }

    LD_IX_IMM(IX_VARIABLES);

    // Eventually seed the rand
    if (prescan.amountOfRandRoutines) {
        ice.programDataPtr -= SIZEOF_RAND_DATA + SIZEOF_SRAND_DATA;
        ice.randAddr = (uint24_t)ice.programDataPtr;
        memcpy(ice.programDataPtr, SRandData, SIZEOF_SRAND_DATA);
        memcpy(ice.programDataPtr + SIZEOF_SRAND_DATA, RandData, SIZEOF_RAND_DATA);
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
    if (*var_name < 64) {
        *var_name += 64;
    }
    sprintf(buf, "Compiling program %s...", var_name);
    displayMessageLineScroll(buf);
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

        // Export the program
        ice.outPrgm = _open(ice.outName);
        if (ice.outPrgm) {
            // This program already exists
            if ((uint8_t)ti_GetC(ice.outPrgm) != 0xEF || (uint8_t)ti_GetC(ice.outPrgm) != 0x7B) {
                gfx_SetTextFGColor(224);
                displayMessageLineScroll("Output program already exists!");
                displayMessageLineScroll("Delete program to continue.");
                didCompile = false;
                goto stop;
            }
            
            previousSize = ti_GetSize(ice.outPrgm);
            ti_Close(ice.outPrgm);
        }
        ice.outPrgm = _new(ice.outName);
        if (!ice.outPrgm) {
            displayMessageLineScroll("Failed to open output file");
            goto stop;
        }

        // Write ASM header
        ti_PutC(tExtTok, ice.outPrgm);
        ti_PutC(tAsm84CeCmp, ice.outPrgm);

        // Write ICE header to be recognized by Cesium
        ti_PutC(0x7F, ice.outPrgm);

        // Write the header, main program, and data to output :D
        ti_Write(ice.programData, ice.programSize, 1, ice.outPrgm);
        if (programDataSize) ti_Write(ice.programDataPtr, programDataSize, 1, ice.outPrgm);

        // Yep, we are really done!
        gfx_SetTextFGColor(4);
        displayMessageLineScroll("Successfully compiled!");

        // Skip line
        displayMessageLineScroll(" ");

        // Display the size
        gfx_SetTextFGColor(0);
        sprintf(buf, "Output size: %u bytes", totalSize);
        displayMessageLineScroll(buf);
        if (previousSize) {
            sprintf(buf, "Previous size: %u bytes", previousSize);
            displayMessageLineScroll(buf);
        }
        sprintf(buf, "Output program: %s", ice.outName);
        displayMessageLineScroll(buf);
    } else {
        displayError(res);
    }

stop:
    gfx_SetTextFGColor(0);
    if (didCompile) {
        if (res == VALID) {
            gfx_PrintStringXY("Run", 9, 232);
            printButton(1);
        } else {
            gfx_PrintStringXY("Goto", 222, 232);
            printButton(217);
        }
        gfx_PrintStringXY("Back", 70, 232);
        printButton(65);
    }
    while (!(key = os_GetCSC()));
err:
    gfx_End();

    if (didCompile) {
        if (key == sk_Yequ && res == VALID) {
            RunPrgm(ice.outName);
        }
        if (key == sk_Window) {
            // Erase screen
            gfx_SetColor(255);
            gfx_FillRectangle_NoClip(0, 11, 320, 229);

            goto displayMainScreen;
        }
        if (key == sk_Trace && res != VALID && !ti_IsArchived(ice.inPrgm)) {
            GotoEditor(ice.currProgName[ice.inPrgm], ti_Tell(ice.inPrgm) - 1);
        }
    }
    ti_CloseAll();
}

#endif

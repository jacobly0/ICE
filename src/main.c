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

#include "parse.h"
#include "main.h"
#include "errors.h"
#include "output.h"
#include "operator.h"

ice_t ice;

void main() {
    uint8_t a = 0;
    unsigned int token;
    unsigned int res;

    strcpy(ice.inName, "ABC");

    ti_CloseAll();
    ice.inPrgm = ti_OpenVar(ice.inName, "r", TI_PRGM_TYPE);
    if (!ice.inPrgm)                                      goto err;
    
    // Check if it's an ICE program
    if ((uint8_t)ti_GetC(ice.inPrgm) != tii)              goto err;
	
	// Setup pointers
	ice.headerData      = (uint8_t *) malloc(500);
	ice.programData     = (uint8_t *) (uint24_t*)0xD52C00;
	ice.programDataData = (uint8_t *) malloc(40000);
	
	ice.headerPtr       = (uint8_t *) (uint24_t)ice.headerData + 116;
	ice.programPtr      = (uint8_t *) (uint24_t)ice.programData + 5;
	ice.programDataPtr  = (uint8_t *) (uint24_t)ice.programDataData;
	
	memcpy(ice.headerData, CHeaderData, 116);
	memcpy(ice.programData, CProgramHeader, 5);
   
    // Do the stuff
    res = parseProgram();
    
    // Create or empty the output program if parsing succeeded
    if (res == VALID) {
        ice.outPrgm = ti_OpenVar(ice.outName, "w", TI_PPRGM_TYPE);
        if (!ice.outPrgm)                                 goto err;
    } else {
        displayError(res);
    }

err:
    ti_CloseAll();
    prgm_CleanUp();
}


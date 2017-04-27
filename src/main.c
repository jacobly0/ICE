#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fileioc.h>

#include "parse.h"
#include "main.h"
#include "errors.h"

ice_t ice;

void main() {
    uint8_t a = 0;
    unsigned int token;
    uint8_t valid;

    strcpy(ice.inName, "ABC");

    ti_CloseAll();
    ice.inPrgm = ti_OpenVar(ice.inName, "r", TI_PRGM_TYPE);
    if (!ice.inPrgm)                                      goto err;
    
    // Check if it's an ICE program
    if (ti_GetC(ice.inPrgm) != tii)                       goto err;
    
    // Do the stuff
    valid = parseProgram();
    
    // Create or empty the output program if parsing succeeded
    if (valid == VALID) {
        ice.outPrgm = ti_OpenVar(ice.outName, "w", TI_PPRGM_TYPE);
        if (!ice.outPrgm)                                      goto err;
    }

err:
    ti_CloseAll();
    prgm_CleanUp();
}


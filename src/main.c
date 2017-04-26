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

    strcpy(ice.inName, "ABC");

    ti_CloseAll();
    ice.inPrgm = ti_OpenVar(ice.inName, "r", ti_Program);
    if (!ice.inPrgm)                                      goto err;
    
    // Check if it's an ICE program
    if (ti_GetC(ice.inPrgm) != 0x2C)                      goto err;
    
    // Get the output 
    while ((token = ti_GetC(ice.inPrgm) != EOF) && token != tEnter && a < 8) {
        ice.outName[a++] = (uint8_t)token;
    }
    
    // Create or empty the output program
    ice.outPrgm = ti_OpenVar(ice.outName, "w", ti_Program);
    if (!ice.outPrgm)                                      goto err;
    
    // Do the stuff
    parseProgram();
    
err:
    ti_CloseAll();
    prgm_CleanUp();
}


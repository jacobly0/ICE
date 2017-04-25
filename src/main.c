/* Keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
 
/* Standard headers - it's recommended to leave them included */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Shared library headers -- depends on which ones you wish to use */
#include <fileioc.h>

#include "parse.c"

const char prgmName[] = "ABC";
char outputName[9];
unsigned int token;
ti_var_t inputProgram, outputProgram;
uint8_t hasAlreadyCFunction = 0;

void main() {
	uint8_t a = 0;
	
    ti_CloseAll();
    inputProgram = ti_OpenVar(prgmName, "r", ti_Program);
    if (!inputProgram) 					goto err;
		
	// Check if it's an ICE program
	if (ti_GetC(inputProgram) != 0x2C)	goto err;
	
	// Get the output 
	while ((token = ti_GetC(inputProgram)) + 1 && token != 0x3F && a < 8) {
		outputName[a++] = token;
	}
	
	// Create or empty the output program
	outputProgram = ti_OpenVar(outputName, "w", ti_Program);
	if (!outputProgram)					goto err;
	
	// Do the stuff
	parseProgram();
	
err:
	ti_CloseAll();
    prgm_CleanUp();
}
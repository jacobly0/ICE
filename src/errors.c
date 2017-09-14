#include "defines.h"
#include "errors.h"

#include "stack.h"
#include "parse.h"
#include "main.h"
#include "output.h"
#include "operator.h"
#include "functions.h"
#include "routines.h"

static const char *errors[] = {
    "This token is not implemented (yet)",
#ifndef COMPUTER_ICE
    "This token cannot be used at the start of the   line",
#else
    "This token cannot be used at the start of the line",
#endif
    "This token doesn't have a condition",
    "You used 'Else' outside an If-statement",
    "You used 'End' outside a condition block",
    "You have an invalid \")\", \",\", \"(\", \")\", \"}\" or \"]\"",
    "You have an invalid expression",
    "Your icon should start with a quote",
    "Invalid hexadecimal",
    "ICE ERROR: please report it!",
    "You have the wrong number or arguments",
    "This C function is not implemented (yet)",
    "Warning: this C function is deprecated",
#ifndef COMPUTER_ICE
    "Label not found: ",
    "Unknown C function. If you are sure this              function exists, please contact me!",
#else
    "Label not found",
    "Unknown C function",
#endif
    "Subprogram not found",
    "Compiling subprograms not supported",
};

void displayError(uint8_t index) {
    const char *str = errors[index];
#ifndef COMPUTER_ICE
    char buf[30];
    char c;
    
    gfx_SetTextFGColor(224);
    gfx_SetTextXY(1, iceMessageLine);
    
    // Display the error
    while(c = *str++) {
        PrintChar(c);
    }
    
    // If a label is not found, ice.inPrgm points to the label, so just display it
    if (index == E_NO_LABEL) {
        while ((c = _getc(ice.inPrgm)) != tEnter) {
            PrintChar(c);
        }
    }
    
    gfx_SetTextFGColor(0);
    sprintf(buf, "Error at line %u", ice.currentLine);
    gfx_PrintStringXY(buf, 1, iceMessageLine);
#else
    fprintf(stdout, "%s\n", str);
    fprintf(stdout, "Error at line %u\n", ice.currentLine);
#endif
}

#ifndef COMPUTER_ICE
void PrintChar(char c) {
    gfx_PrintChar(c);
    if (gfx_GetTextX() > 312) {
        gfx_SetTextXY(1, iceMessageLine);
    }
}
#endif

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
    "This token/function is not implemented (yet)",
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
#ifndef COMPUTER_ICE
    "Unknown C function. If you are sure this              function exists, please contact me!",
#else
    "Unknown C function",
#endif
    "Subprogram not found",
    "Compiling subprograms not supported",
    "rand and getKey are disabled in this function",
    "Too much memory allocated",
    "Warning: Unknown char in the string!",
    "Invalid program name",
};

void displayLabelError(char *label) {
    char buf[30];
    
    sprintf(buf, "Label %s not found", label);
#ifndef COMPUTER_ICE 
    gfx_SetTextFGColor(224);
    displayMessageLineScroll(buf);
#else
    fprintf(stdout, "%s\n", buf);
#endif
}

void displayError(uint8_t index) {
#ifndef COMPUTER_ICE
    char buf[30];
    gfx_SetTextFGColor(224 + 3 * (index == E_WRONG_CHAR));
    displayMessageLineScroll(errors[index]);
    
    gfx_SetTextFGColor(0);
    sprintf(buf, "Error at line %u", ice.currentLine);
    displayMessageLineScroll(buf);
#else
    fprintf(stdout, "%s\n", errors[index]);
    fprintf(stdout, "Error at line %u\n", ice.currentLine);
#endif
}

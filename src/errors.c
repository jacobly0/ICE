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
#if !defined(COMPUTER_ICE) && !defined(SC)
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
#if !defined(COMPUTER_ICE) && !defined(SC)
    "Unknown C function. If you are sure this              function exists, please contact me!",
#else
    "Unknown C function",
#endif
    "Subprogram not found",
    "Compiling subprograms not supported",
    "Invalid program name",
    "Warning: Unknown char in the string!",
    "Warning: string has been automatically squish-ed!",
#if !defined(COMPUTER_ICE) && !defined(SC)
    "Warning: you need det(0) before using any           other graphics function!",
    "Warning: you need sum(0) before using any           other file i/o function!",
    "Warning: you need det(1) before returning to    the OS!",
#else
    "Warning: you need det(0) before using any other graphics function!",
    "Warning: you need sum(0) before using any other file i/o function!",
    "Warning: you need det(1) before returning to the OS!",
#endif
};

void displayLabelError(char *label) {
    char buf[30];
    
    sprintf(buf, "Label %s not found", label);
#if !defined(COMPUTER_ICE) && !defined(SC)
    gfx_SetTextFGColor(224);
    displayMessageLineScroll(buf);
#else
    fprintf(stdout, "%s\n", buf);
#endif
}

void displayError(uint8_t index) {
#if !defined(COMPUTER_ICE) && !defined(SC)
    char buf[30];
    
    gfx_SetTextFGColor(index < W_WRONG_CHAR ? 224 : 227);
    displayMessageLineScroll(errors[index]);
    
    gfx_SetTextFGColor(0);
    sprintf(buf, "Error at line %u", ice.currentLine);
    displayMessageLineScroll(buf);
#else
    fprintf(stdout, "%s\n", errors[index]);
    fprintf(stdout, "Error at line %u\n", ice.currentLine);
#endif
}

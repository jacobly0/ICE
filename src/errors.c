#include "errors.h"

#include "stack.h"
#include "parse.h"
#include "main.h"
#include "output.h"
#include "operator.h"
#include "functions.h"

#ifndef COMPUTER_ICE
#include <graphx.h>
#include <debug.h>
#endif

#include <tice.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *errors[] = {
    "This token is not implemented (yet)",
#ifndef COMPUTER_ICE
    "This operator cannot be used at the start of   the line",
#else
    "This operator cannot be used at the start of the line",
#endif
    "This token doesn't have a condition",
    "You used 'Else' outside an If-statement",
    "You used 'End' outside a condition block",
    "You have an extra \")\" or \",\"",
    "You have an invalid expression",
    "Your icon should start with a quote",
    "Your icon has invalid syntax or not the right length",
    "ICE ERROR: please report it!",
    "You have the wrong number or arguments",
    "This C function is not implemented (yet)",
    "Warning: this C function is deprecated",
};

void displayError(unsigned int index) {
    const char *str = errors[index];
#ifndef COMPUTER_ICE
    char buf[30];
    char c;
    
    gfx_SetTextFGColor(224);
    gfx_SetTextXY(1, iceMessageLine);
    
    while(c = *str++) {
        gfx_PrintChar(c);
        if (gfx_GetTextX() > 312) {
            gfx_SetTextXY(1, iceMessageLine);
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

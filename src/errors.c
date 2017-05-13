#include <graphx.h>

#include "parse.h"
#include "main.h"
#include "errors.h"
#include "output.h"
#include "operator.h"
#include "stack.h"
#include "functions.h"

static const char *errors[] = {
    "This token is not implemented (yet)",
    "This operator cannot be used at the start of   the line",
    "This token doesn't have a condition",
    "You used 'Else' or End' outside a condition block",
    "You have an extra right paren",
    "You have an invalid expression",
    "Your icon should start with a quote",
    "Your icon has invalid syntax or not the right length",
    "ICE ERROR: please report it!",
    "You have the wrong number or arguments",
};

void displayError(unsigned int index) {
    signed char buf[30];
    signed char c;
    signed char *str = errors[index];
    
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
}


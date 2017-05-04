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
#include <graphx.h>

#include "parse.h"
#include "main.h"
#include "errors.h"
#include "output.h"
#include "operator.h"

static const char *errors[] = {
    "This token is not implemented (yet)",
    "This token cannot be used at this place",
    "This token doesn't have a condition",
    "You used 'Else' or End' outside a condition block",
    "You have an extra right paren",
    "You have an invalid expression",
    "Your icon should start with a quote",
    "Your icon has invalid syntax or not the right length",
};

void displayError(unsigned int index) {
    gfx_SetTextFGColor(224);
    gfx_PrintStringXY(errors[index], 1, ++ice.messageIndex*10+3);
}

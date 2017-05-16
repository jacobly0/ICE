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
#include "stack.h"
#include "functions.h"

uint8_t parseFunction(uint24_t index, uint8_t *outputStack) {
    element_t *outputPtr       = (element_t*)outputStack;
    element_t *outputCurr, *outputPrev;
    uint8_t function, amountOfArguments;
    uint24_t output;
    
    outputCurr        = &outputPtr[index];
    outputPrev        = &outputPtr[index - 1];
    output            = outputCurr->operand;
    function          = (uint8_t)output;
    amountOfArguments = (uint8_t)(output >> 8);
    
    // Oops, we forgot at least 1 argument
    if (amountOfArguments > (uint8_t)index) {
        return E_ARGUMENTS;
    }
    
    // Dummy thing, just to make sure we 'parsed' a function :P
    OR_A_A();
    
    return VALID;
}
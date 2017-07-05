#include "stack.h"

#include "parse.h"
#include "main.h"
#include "errors.h"
#include "output.h"
#include "operator.h"
#include "functions.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>

static unsigned int *p1;
static unsigned int *p2;

void push(unsigned int i) {
    *p1++ = i;
}

unsigned int getNextIndex(void) {
    return *(p2++);
}

unsigned int getIndexOffset(unsigned int offset) {
    return *(p2 + offset);
}

void removeIndexFromStack(unsigned int index) {
    memcpy(ice.stackStart + index, ice.stackStart + index + 1, STACK_SIZE - index);
    p2--;
}

unsigned int getCurrentIndex(void) {
    return p2 - ice.stackStart;
}

unsigned int *getStackVar(uint8_t which) {
    if (which) {
        return p2;
    }
    return p1;
}

void setStackValues(uint24_t* val1, uint24_t* val2) {
    p1 = val1;
    p2 = val2;
}
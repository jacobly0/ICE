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

void setStackVar(uint24_t* val, uint8_t which) {
    if (which) {
        p2 = val;
    } else {
        p1 = val;
    }
}
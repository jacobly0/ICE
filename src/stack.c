#include "stack.h"

#define STACK_SIZE 25

static unsigned int stack[STACK_SIZE];
static unsigned int *p1 = stack;

void push(unsigned int i) {
    *++p1 = i;
}

unsigned int pop(void) {
    return *(p1--);
}

void clearStack() {
    p1 = stack;
}

unsigned int getStackSize(void) {
    return p1 - stack;
}
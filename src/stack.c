#include "stack.h"

#define STACK_SIZE 15

static unsigned int stack[STACK_SIZE];
static unsigned int *p1 = stack;

void push(unsigned int i) {
    *++p1 = i;
}

unsigned int pop(void) {
    return *(p1--);
}

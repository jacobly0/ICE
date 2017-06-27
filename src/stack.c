#include <string.h>

#include "stack.h"

#define STACK_SIZE 25

static unsigned int stack[STACK_SIZE * 3];
static unsigned int *p1 = stack;
static unsigned int *p2 = stack;
static uint8_t amountOfUsedStacks = 0;

void push(unsigned int i) {
    *p1++ = i;
}

unsigned int pop(void) {
    return *(--p1);
}

unsigned int getStackSize(void) {
    return p1 - stack - (amountOfUsedStacks * STACK_SIZE);
}

unsigned int getNextIndex(void) {
    return *(p2++);
}

unsigned int getIndexOffset(unsigned int offset) {
    return *(p2 + offset);
}

void removeIndexFromStack(unsigned int index) {
    memcpy(stack + index, stack + index + 1, STACK_SIZE - index);
    p2--;
}

unsigned int getCurrentIndex(void) {
    return p2 - stack;
}

void getNextFreeStack(void) {
    push(p2);
    push(p1);
    p1 = stack + (++amountOfUsedStacks * STACK_SIZE);
    p2 = p1;
}

void removeStack(void) {
}
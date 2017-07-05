#ifndef STACK_H
#define STACK_H

#include <stdint.h>

void push(unsigned int i);
unsigned int pop(void);
unsigned int getStackSize(void);
unsigned int getNextIndex(void);
unsigned int getCurrentIndex(void);
unsigned int getIndexOffset(unsigned int offset);
void removeIndexFromStack(unsigned int index);
void getNextFreeStack(void);
void removeStack(void);
unsigned int *getStackVar(uint8_t which);
void setStackValues(uint24_t* val1, uint24_t* val2);

#endif

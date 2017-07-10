#ifndef STACK_H
#define STACK_H

#include <stdint.h>

#ifdef COMPUTER_ICE
typedef uint32_t uint24_t;
#endif

void push(uint24_t i);
uint24_t pop(void);
uint24_t getStackSize(void);
uint24_t getNextIndex(void);
uint24_t getCurrentIndex(void);
uint24_t getIndexOffset(uint24_t offset);
void removeIndexFromStack(uint24_t index);
void getNextFreeStack(void);
void removeStack(void);
uint24_t *getStackVar(uint8_t which);
void setStackValues(uint24_t* val1, uint24_t* val2);

#endif

#ifndef STACK_H
#define STACK_H

void push(unsigned int i);
unsigned int pop(void);
unsigned int getStackSize(void);
unsigned int getNextIndex(void);
unsigned int getCurrentIndex(void);
unsigned int getIndexOffset(unsigned int offset);
void removeIndexFromStack(unsigned int index);

#endif

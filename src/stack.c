#define SIZE 50

int  *tos, *p1, stack[SIZE];
 
void push(unsigned int i) {
    *++p1 = i;
}
 
unsigned int pop(void) {
    return *(p1--);
}
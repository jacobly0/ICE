#include "defines.h"
#include "ast.h"

#include "operator.h"
#include "main.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "output.h"
#include "routines.h"
#include "prescan.h"

NODE *push2(NODE *top, element_t data) {
    NODE *tempNode = (NODE*)malloc(sizeof(NODE));
    
    tempNode->data = data;
    tempNode->prev = top;
    top->sibling = tempNode;
    
    return tempNode;
}

NODE *insertData(NODE *top, element_t data, uint8_t index) {
    NODE *tempNode = (NODE*)malloc(sizeof(NODE));
    uint8_t temp;
    
    tempNode->data = data;
    for (temp = 1; temp < index; temp++) {
        top = top->prev;
        
        if (top == NULL) {
            return NULL;
        }
    }
    if (top == NULL) {
        return NULL;
    }
    top->prev->sibling = tempNode;
    tempNode->child = top;
    
    return tempNode;
}

NODE *parseAST(NODE *top) {
    return NULL;
}
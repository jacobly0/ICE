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

extern uint8_t (*functions[256])(unsigned int token);
const char implementedFunctions[] = {tDet, tNot, tRemainder, tMin, tMax, tMean, tSqrt};

unsigned int getc(void) {
    return ti_GetC(ice.inPrgm);
}

uint8_t parseProgram(void) {
    unsigned int token;
    uint8_t ret = VALID;

    ti_Rewind(ice.inPrgm);

    // do things based on the token
    while ((token = getc()) != EOF) {
        if ((uint8_t)token != tii) {
            ice.usedCodeAfterHeader = true;
        }
        if ((ret = (*functions[(uint8_t)token])(token)) != VALID) {
            break;
        }
    }

    // If the last token is not "Return", write a "ret" to the program
    if (ret == VALID && !ice.lastTokenIsReturn) {
        output(uint8_t, OP_RET);
    }

    return ret;
}

/* Static functions */

static uint8_t parseExpression(unsigned int token) {
    const uint8_t *outputStack   = (uint8_t*)0xD62C00;
    const uint8_t *stack         = (uint8_t*)0xD63000;
    unsigned int outputElements  = 0;
    unsigned int stackElements   = 0;
    unsigned int loopIndex, temp, lastDetIndex = 0;
    uint8_t index, usedDetInExpression = false;
    uint8_t amountOfArgumentsStack[20];
    uint8_t *amountOfArgumentsStackPtr;
    
    // Setup pointers
    element_t *outputPtr = (element_t*)outputStack;
    element_t *stackPtr = (element_t*)stack;
    element_t *outputCurr, *outputPrev, *outputPrevPrev;
    element_t *stackCurr, *stackPrev = NULL;
    amountOfArgumentsStackPtr = (uint8_t*)amountOfArgumentsStack;

    while (token != EOF && token != tEnter) {
        uint8_t tok;
        outputCurr = &outputPtr[outputElements];
        stackCurr  = &stackPtr[stackElements];
        tok = (uint8_t)token;
        
        // If the token is det(, set the right variable to true (expressions with det() parse different)
        if (tok == tDet) {
            usedDetInExpression = true;
        }

        // Process a number
        if (tok >= t0 && tok <= t9) {
            uint24_t output = token - t0;
            while ((uint8_t)(token = getc()) >= t0 && (uint8_t)token <= t9) {
                output = output*10 + (uint8_t)token - t0;
            }
            outputCurr->type = TYPE_NUMBER;
            outputCurr->operand = output;
            outputElements++;

            // Don't grab a new token
            continue;
        }

        // Process a variable
        else if (tok >= tA && tok <= tTheta) {
            outputCurr->type = TYPE_VARIABLE;
            outputCurr->operand = tok - tA;
            outputElements++;
        }
        
        // Parse an operator
        else if ((index = getIndexOfOperator(tok))) {
            // If the token is ->, move the entire stack to the output, instead of checking the precedence
            if (tok == tStore) {
                // The last argument is not counted yet, so increment
                (*amountOfArgumentsStackPtr)++;
                
                // Move entire stack to output
                while (stackElements) {
                    outputCurr = &outputPtr[outputElements++];
                    stackPrev = &stackPtr[--stackElements];
                    outputCurr->type = stackPrev->type;
                    temp = stackPrev->operand;
                    if (stackPrev->type == TYPE_FUNCTION) {
                        temp += (*amountOfArgumentsStackPtr--) << 8;
                    }
                    outputCurr->operand = temp;
                }
            }
            while (stackElements) {
                stackPrev = &stackPtr[stackElements-1];
                outputCurr = &outputPtr[outputElements];
                
                // Move the last entry of the stack to the ouput if it's precedence is greater than the precedence of the current token
                if (stackPrev->type == TYPE_OPERATOR && operatorPrecedence[index - 1] <= operatorPrecedence[getIndexOfOperator(stackPrev->operand) - 1]) {
                    outputCurr->type = TYPE_OPERATOR;
                    outputCurr->operand = stackPrev->operand;
                    stackElements--;
                    outputElements++;
                } else {
                    break;
                }
            }
            stackCurr = &stackPtr[stackElements];
            stackCurr->type = TYPE_OPERATOR;
            stackCurr->operand = token;
            stackElements++;
        }
        
        // Push a left parenthesis
        else if (tok == tLParen) {
            stackCurr->type = TYPE_FUNCTION;
            stackCurr->operand = token;
            stackElements++;
        }
        
        // Pop a right parenthesis
        else if (tok == tRParen || tok == tComma) {
            // Move until stack is empty or a function is encountered
            while (stackElements) {
                stackPrev = &stackPtr[stackElements-1];
                outputCurr = &outputPtr[outputElements];
                if (stackPrev->type != TYPE_FUNCTION) {
                    outputCurr->type = stackPrev->type;
                    outputCurr->operand = stackPrev->operand;
                    stackElements--;
                    outputElements++;
                } else {
                    break;
                }
            }
            
            // No matching left parenthesis
            if (!stackElements) {
                return E_EXTRA_RPAREN;
            }
            
            // Increment the amount of arguments for that function
            (*amountOfArgumentsStackPtr)++;
            
            // If the right parenthesis belongs to a function, move the function as well
            if (tok == tRParen && stackPrev->operand != tLParen) {
                // If the operand is a det() function, update the last det() index
                if (stackPrev->operand == tDet) {
                    lastDetIndex = outputElements;
                }
                outputCurr->type = TYPE_FUNCTION;
                outputCurr->operand = stackPrev->operand + ((*amountOfArgumentsStackPtr--) << 8);
                stackElements--;
                outputElements++;
            }
        } 
        
        // Process a function which returns something (not(), sqrt(), det(...))
        else if (strchr(implementedFunctions, tok)) {
            *++amountOfArgumentsStackPtr = 0;
            stackCurr->type = TYPE_FUNCTION;
            stackCurr->operand = token;
            stackElements++;
        } 
        
        // Oops, unknown token...
        else {
            return E_UNIMPLEMENTED;
        }
       
        token = getc();
    }
    
    // The last argument is not counted yet, so increment
    (*amountOfArgumentsStackPtr)++;
    
    // Move entire stack to output
    while (stackElements) {
        outputCurr = &outputPtr[outputElements++];
        stackPrev = &stackPtr[--stackElements];
        outputCurr->type = stackPrev->type;
        temp = stackPrev->operand;
        if (stackPrev->type == TYPE_FUNCTION) {
            temp += (*amountOfArgumentsStackPtr--) << 8;
        }
        outputCurr->operand = temp;
    }
    
    // Remove stupid things like 2+5
    for (loopIndex = 2; loopIndex < outputElements; loopIndex++) {
        outputPrevPrev = &outputPtr[loopIndex-2];
        outputPrev = &outputPtr[loopIndex-1];
        outputCurr = &outputPtr[loopIndex];
        
        // Check if the types are number | number | operator
        if (outputPrevPrev->type == TYPE_NUMBER && outputPrev->type == TYPE_NUMBER && outputCurr->type == TYPE_OPERATOR) {
            // If yes, execute the operator, and store it in the first entry, and remove the other 2
            outputPrevPrev->operand = executeOperator(outputPrevPrev->operand, outputPrev->operand, (uint8_t)outputCurr->operand);
            memcpy(outputPrev, &outputPtr[loopIndex+1], (outputElements-1)*4);
            outputElements -= 2;
            loopIndex--;
        }
    }
    
    // Check if the expression is valid
    if (outputElements == 1) {
        outputCurr = &outputPtr[0];
        
        // Expression is only a single number
        if (outputCurr->type == TYPE_NUMBER) {
            // This boolean is set, because loops may be optimized when the condition is a number
            ice.exprOutputIsNumber = true;
            LD_HL_IMM(outputCurr->operand);
        } 
        
        // Expression is only a variable
        else if (outputCurr->type == TYPE_VARIABLE) {
            LD_HL_IND_IX_OFF(outputCurr->operand);
        } 
        
        // Expression is only a function without arguments that returns something (getKey, rand)
        else if (outputCurr->type == TYPE_FUNCTION_RETURN) {
            insertFunctionReturn(outputCurr->operand, OUTPUT_IN_HL, NO_PUSH);
        }
        
        // Expression is something wrong, for example "not("
        else {
            return E_SYNTAX;
        }
        return VALID;
    } else if (outputElements == 2) {
        return E_SYNTAX;
    }
    
    // Parse the expression in infix notation!
    for (loopIndex = 2; loopIndex < outputElements; loopIndex++) {
        outputCurr = &outputPtr[loopIndex];
        
        // Parse an operator with 2 arguments
        if (outputCurr->type == TYPE_OPERATOR) {
            parseOperator(&outputPtr[loopIndex-2], &outputPtr[loopIndex-1], outputCurr);
        } 
        
        // Parse a function with X arguments
        else if (outputCurr->type == TYPE_FUNCTION) {
        }
    }

    return VALID;
}

static uint8_t functionI(unsigned int token) {
    uint8_t a = 0, b = 0, outputByte, tok;
    const char *dataString;
    const uint8_t colorTable[16] = {255,24,224,0,248,36,227,97,9,19,230,255,181,107,106,74};
    
    // Only get the output name, icon or description at the top of your program
    if (!ice.usedCodeAfterHeader) {
        // Get the output name
        if (!ice.gotName) {
            while ((token = getc()) != EOF && (uint8_t)token != tEnter && a < 9) {
                ice.outName[a++] = (uint8_t)token;
            }
            ice.gotName = true;
            return VALID;
        }

        // Get the icon and description
        else if (!ice.gotIconDescription) {
            // Move header to take place for the icon and description, setup pointer
            memcpy(ice.headerData + 350, ice.headerData, 116);
            ice.headerPtr = ice.headerData;
            
            // Insert "jp <random>" and Cesium header
            *ice.headerPtr = OP_JP;
            *(uint24_t*)(ice.headerPtr+4) = 0x101001;
            ice.headerPtr += 7;
            
            // Icon should start with a "
            if ((uint8_t)getc() != tString) {
                return E_WRONG_ICON;
            }

            // Get hexadecimal
            do {
                tok = (uint8_t)getc();
                if (tok >= t0 && tok <= t9) {
                    outputByte = tok - t0;
                } else if (tok >= tA && tok <= tF) {
                    outputByte = tok - tA + 10;
                } else {
                    return E_INVALID_ICON;
                }
                *ice.headerPtr++ = colorTable[outputByte];
            } while (++b);
            
            // Move on to the description
            if ((uint8_t)(token = getc()) == tString) {
                token = getc();
            }

            if (token != EOF) {
                
                if ((uint8_t)token != tEnter) {
                    return E_INVALID_ICON;
                }
                
                // Check if there is a description
                if ((token = getc()) == tii) {
                    uint8_t *dataPtr = ti_GetDataPtr(ice.inPrgm);
                    
                    // Grab description
                    while ((token = getc()) != EOF && (uint8_t)token != tEnter) {
                        unsigned int tokLength;
                        dataString = ti_GetTokenString(&dataPtr, NULL, &tokLength);
                        memcpy(ice.headerPtr, dataString, tokLength);
                        ice.headerPtr += tokLength;
                    }
                } else if (token != EOF) {
                    ti_Seek(-1, SEEK_CUR, ice.inPrgm);
                }
            }

            // Don't increment the pointer for now, we will do that later :)
            *ice.headerPtr = 0;

            // Write the right jp offset
            *(uint24_t*)(ice.headerData+1) = ice.headerPtr - ice.headerData + PRGM_START;
            
            // Copy header back, and update the 3 pointers in the C header...
            memcpy(ice.headerPtr + 1, ice.headerData + 350, 116);
            *(uint24_t*)(ice.headerPtr+2)  = *(uint24_t*)(ice.headerPtr+2)  + (uint24_t)ice.headerPtr - (uint24_t)ice.headerData;
            *(uint24_t*)(ice.headerPtr+53) = *(uint24_t*)(ice.headerPtr+53) + (uint24_t)ice.headerPtr - (uint24_t)ice.headerData;
            *(uint24_t*)(ice.headerPtr+66) = *(uint24_t*)(ice.headerPtr+66) + (uint24_t)ice.headerPtr - (uint24_t)ice.headerData;
            ice.headerPtr += 117;
            
            ice.gotIconDescription = true;
            return VALID;
        }
        
        // Don't return and treat as a comment
        else {
            ice.usedCodeAfterHeader = true;
        }
    }

    // Treat it as a comment
    while ((token = getc()) != EOF && token != tEnter);

    return VALID;
}

static uint8_t functionPrgm(unsigned int token) {
    return VALID;
}

static uint8_t functionCustom(unsigned int token) {
    return VALID;
}

static uint8_t functionIf(unsigned int token) {
    if ((token = getc()) != EOF && token != tEnter) {
        parseExpression(token);
        return VALID;
    } else {
        return E_NO_CONDITION;
    }
}

static uint8_t functionElseEnd(unsigned int token) {
    // This should return if in nested block
    if (!ice.nestedBlocks) {
        return E_NO_NESTED_BLOCK;
    }
    return E_ELSE_END;
}

static uint8_t dummyReturn(unsigned int token) {
    return VALID;
}

static uint8_t functionWhile(unsigned int token) {
    return VALID;
}

static uint8_t functionRepeat(unsigned int token) {
    return VALID;
}

static uint8_t functionFor(unsigned int token) {
    return VALID;
}

static uint8_t functionReturn(unsigned int token) {
    return VALID;
}

static uint8_t functionLbl(unsigned int token) {
    return VALID;
}

static uint8_t functionGoto(unsigned int token) {
    return VALID;
}

static uint8_t functionPause(unsigned int token) {
    return VALID;
}

static uint8_t functionInput(unsigned int token) {
    return VALID;
}

static uint8_t functionDisp(unsigned int token) {
    return VALID;
}

static uint8_t functionOutput(unsigned int token) {
    return VALID;
}

static uint8_t functionClrHome(unsigned int token) {
    return VALID;
}

static uint8_t tokenWrongPlace(unsigned int token) {
    return E_WRONG_PLACE;
}

static uint8_t tokenUnimplemented(unsigned int token) {
    return E_UNIMPLEMENTED;
}

uint8_t (*functions[256])(unsigned int) = {
    tokenUnimplemented, //0
    tokenUnimplemented, //1
    tokenUnimplemented, //2
    tokenUnimplemented, //3
    tokenWrongPlace,    //4
    tokenUnimplemented, //5
    tokenUnimplemented, //6
    tokenUnimplemented, //7
    parseExpression,    //8
    parseExpression,    //9
    tokenUnimplemented, //10
    tokenUnimplemented, //11
    tokenUnimplemented, //12
    tokenUnimplemented, //13
    tokenUnimplemented, //14
    tokenUnimplemented, //15
    parseExpression,    //16
    parseExpression,    //17
    tokenUnimplemented, //18
    tokenUnimplemented, //19
    tokenUnimplemented, //20
    tokenUnimplemented, //21
    tokenUnimplemented, //22
    tokenUnimplemented, //23
    tokenUnimplemented, //24
    tokenUnimplemented, //25
    tokenUnimplemented, //26
    tokenUnimplemented, //27
    tokenUnimplemented, //28
    tokenUnimplemented, //29
    tokenUnimplemented, //30
    tokenUnimplemented, //31
    tokenUnimplemented, //32
    tokenUnimplemented, //33
    tokenUnimplemented, //34
    tokenUnimplemented, //35
    tokenUnimplemented, //36
    tokenUnimplemented, //37
    tokenUnimplemented, //38
    tokenUnimplemented, //39
    tokenUnimplemented, //40
    tokenUnimplemented, //41
    parseExpression,    //42
    tokenUnimplemented, //43
    functionI,          //44
    tokenUnimplemented, //45
    tokenUnimplemented, //46
    tokenUnimplemented, //47
    parseExpression,    //48
    parseExpression,    //49
    parseExpression,    //50
    parseExpression,    //51
    parseExpression,    //52
    parseExpression,    //53
    parseExpression,    //54
    parseExpression,    //55
    parseExpression,    //56
    parseExpression,    //57
    tokenUnimplemented, //58
    tokenUnimplemented, //59
    tokenWrongPlace,    //60
    tokenWrongPlace,    //61
    tokenUnimplemented, //62
    dummyReturn,        //63
    tokenWrongPlace,    //64
    parseExpression,    //65
    parseExpression,    //66
    parseExpression,    //67
    parseExpression,    //68
    parseExpression,    //69
    parseExpression,    //70
    parseExpression,    //71
    parseExpression,    //72
    parseExpression,    //73
    parseExpression,    //74
    parseExpression,    //75
    parseExpression,    //76
    parseExpression,    //77
    parseExpression,    //78
    parseExpression,    //79
    parseExpression,    //80
    parseExpression,    //81
    parseExpression,    //82
    parseExpression,    //83
    parseExpression,    //84
    parseExpression,    //85
    parseExpression,    //86
    parseExpression,    //87
    parseExpression,    //88
    parseExpression,    //89
    parseExpression,    //90
    parseExpression,    //91
    tokenUnimplemented, //92
    parseExpression,    //93
    tokenUnimplemented, //94
    functionPrgm,       //95
    tokenUnimplemented, //96
    tokenUnimplemented, //97
    functionCustom,     //98
    tokenUnimplemented, //99
    tokenUnimplemented, //100
    tokenUnimplemented, //101
    tokenUnimplemented, //102
    tokenUnimplemented, //103
    tokenUnimplemented, //104
    tokenUnimplemented, //105
    tokenWrongPlace,    //106
    tokenWrongPlace,    //107
    tokenWrongPlace,    //108
    tokenWrongPlace,    //109
    tokenWrongPlace,    //110
    tokenWrongPlace,    //111
    tokenWrongPlace,    //112
    tokenWrongPlace,    //113
    tokenUnimplemented, //114
    tokenUnimplemented, //115
    tokenUnimplemented, //116
    tokenUnimplemented, //117
    tokenUnimplemented, //118
    tokenUnimplemented, //119
    tokenUnimplemented, //120
    tokenUnimplemented, //121
    tokenUnimplemented, //122
    tokenUnimplemented, //123
    tokenUnimplemented, //124
    tokenUnimplemented, //125
    tokenUnimplemented, //126
    tokenUnimplemented, //127
    tokenUnimplemented, //128
    tokenUnimplemented, //129
    tokenWrongPlace,    //130
    tokenWrongPlace,    //131
    tokenUnimplemented, //132
    tokenUnimplemented, //133
    tokenUnimplemented, //134
    tokenUnimplemented, //135
    tokenUnimplemented, //136
    tokenUnimplemented, //137
    tokenUnimplemented, //138
    tokenUnimplemented, //139
    tokenUnimplemented, //140
    tokenUnimplemented, //141
    tokenUnimplemented, //142
    tokenUnimplemented, //143
    tokenUnimplemented, //144
    tokenUnimplemented, //145
    tokenUnimplemented, //146
    tokenUnimplemented, //147
    tokenUnimplemented, //148
    tokenUnimplemented, //149
    tokenUnimplemented, //150
    tokenUnimplemented, //151
    tokenUnimplemented, //152
    tokenUnimplemented, //153
    tokenUnimplemented, //154
    tokenUnimplemented, //155
    tokenUnimplemented, //156
    tokenUnimplemented, //157
    tokenUnimplemented, //158
    tokenUnimplemented, //159
    tokenUnimplemented, //160
    tokenUnimplemented, //161
    tokenUnimplemented, //162
    tokenUnimplemented, //163
    tokenUnimplemented, //164
    tokenUnimplemented, //165
    tokenUnimplemented, //166
    tokenUnimplemented, //167
    tokenUnimplemented, //168
    tokenUnimplemented, //169
    tokenUnimplemented, //170
    parseExpression,    //171
    tokenUnimplemented, //172
    parseExpression,    //173
    tokenUnimplemented, //174
    tokenUnimplemented, //175
    tokenUnimplemented, //176
    tokenUnimplemented, //177
    tokenUnimplemented, //178
    parseExpression,    //179
    tokenUnimplemented, //180
    tokenUnimplemented, //181
    tokenUnimplemented, //182
    tokenUnimplemented, //183
    parseExpression,    //184
    tokenUnimplemented, //185
    tokenUnimplemented, //186
    tokenUnimplemented, //187
    parseExpression,    //188
    tokenUnimplemented, //189
    tokenUnimplemented, //190
    tokenUnimplemented, //191
    tokenUnimplemented, //192
    tokenUnimplemented, //193
    tokenUnimplemented, //194
    tokenUnimplemented, //195
    tokenUnimplemented, //196
    tokenUnimplemented, //197
    tokenUnimplemented, //198
    tokenUnimplemented, //199
    tokenUnimplemented, //200
    tokenUnimplemented, //201
    tokenUnimplemented, //202
    tokenUnimplemented, //203
    tokenUnimplemented, //204
    tokenUnimplemented, //205
    functionIf,         //206
    tokenUnimplemented, //207
    functionElseEnd,    //208
    functionWhile,      //209
    functionRepeat,     //210
    functionFor,        //211
    functionElseEnd,    //212
    functionReturn,     //213
    functionLbl,        //214
    functionGoto,       //215
    functionPause,      //216
    tokenUnimplemented, //217
    tokenUnimplemented, //218
    tokenUnimplemented, //219
    functionInput,      //220
    tokenUnimplemented, //221
    functionDisp,       //222
    tokenUnimplemented, //223
    functionOutput,     //224
    functionClrHome,    //225
    tokenUnimplemented, //226
    tokenUnimplemented, //227
    tokenUnimplemented, //228
    tokenUnimplemented, //229
    tokenUnimplemented, //230
    tokenUnimplemented, //231
    tokenUnimplemented, //232
    tokenUnimplemented, //233
    tokenUnimplemented, //234
    tokenUnimplemented, //235
    tokenUnimplemented, //236
    tokenUnimplemented, //237
    tokenUnimplemented, //238
    tokenUnimplemented, //239
    tokenUnimplemented, //240
    tokenUnimplemented, //241
    tokenUnimplemented, //242
    tokenUnimplemented, //243
    tokenUnimplemented, //244
    tokenUnimplemented, //245
    tokenUnimplemented, //246
    tokenUnimplemented, //247
    tokenUnimplemented, //248
    tokenUnimplemented, //249
    tokenUnimplemented, //250
    tokenUnimplemented, //251
    tokenUnimplemented, //252
    tokenUnimplemented, //253
    tokenUnimplemented, //254
    tokenUnimplemented  //255
};


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

extern uint8_t (*functions[256])(unsigned int token);
const char implementedFunctions[] = {tDet, tNot, tRemainder, tMin, tMax, tMean, tSqrt};

unsigned int getc(void) {
    return ti_GetC(ice.inPrgm);
}

uint8_t parseProgram(void) {
    unsigned int token;
    uint8_t ret = VALID;

    ti_Rewind(ice.inPrgm);

    // Do things based on the token
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
        RET();
    }

    return ret;
}

/* Static functions */

static uint8_t parseExpression(unsigned int token) {
    const uint8_t *outputStack    = (uint8_t*)0xD62C00;
    const uint8_t *stack          = (uint8_t*)0xD64000;
    const uint8_t *tempCFunctions = stack;
    unsigned int outputElements   = 0;
    unsigned int stackElements    = 0;
    unsigned int loopIndex, temp;
    uint8_t index = 0, nestedDets = 0;
    uint8_t amountOfArgumentsStack[20];
    uint8_t *amountOfArgumentsStackPtr = amountOfArgumentsStack;
    uint8_t stackToOutputReturn;

    // Setup pointers
    element_t *outputPtr = (element_t*)outputStack;
    element_t *stackPtr = (element_t*)stack;
    element_t *outputCurr, *outputPrev, *outputPrevPrev;
    element_t *stackCurr, *stackPrev = NULL;
    
    /*
        General explanation stacks:
        - Each entry consists of 4 bytes, the type and the operand
        - Type: first 4 bits is the amount of nested det()'s, the last 4 the type, like number, variable, function, operator etc
        - The operand is either a 3-byte number or consists of 3 bytes:
            - The first byte = the operan: function/variable/operator
            - If it's a function then the second byte is the amount of arguments for that function
    */

    while (token != EOF && token != tEnter) {
        uint8_t tok;
        outputCurr = &outputPtr[outputElements];
        stackCurr  = &stackPtr[stackElements];
        tok = (uint8_t)token;

        // Process a number
        if (tok >= t0 && tok <= t9) {
            uint24_t output = token - t0;
            while ((uint8_t)(token = getc()) >= t0 && (uint8_t)token <= t9) {
                output = output*10 + (uint8_t)token - t0;
            }
            outputCurr->type = TYPE_NUMBER + (nestedDets << 4);
            outputCurr->operand = output;
            outputElements++;

            // Don't grab a new token
            continue;
        }

        // Process a variable
        else if (tok >= tA && tok <= tTheta) {
            outputCurr->type = TYPE_VARIABLE + (nestedDets << 4);
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
                stackToOutputReturn = 1;
                goto stackToOutput;
stackToOutputReturn1:

                // We are in not in any nested det() anymore
                nestedDets = 0;
            }
            
            // Move the stack to the output as long...
            while (stackElements) {
                stackPrev = &stackPtr[stackElements-1];
                outputCurr = &outputPtr[outputElements];
                
                // Move the last entry of the stack to the ouput if it's precedence is greater than the precedence of the current token
                if ((stackPrev->type & 15) == TYPE_OPERATOR && operatorPrecedence[index - 1] <= operatorPrecedence[getIndexOfOperator(stackPrev->operand) - 1]) {
                    outputCurr->type = stackPrev->type;
                    outputCurr->operand = stackPrev->operand;
                    stackElements--;
                    outputElements++;
                } else {
                    break;
                }
            }
            
            // Push the operator to the stack
            stackCurr = &stackPtr[stackElements++];
            stackCurr->type = TYPE_OPERATOR + (nestedDets << 4);
            stackCurr->operand = token;
        }
        
        // Push a left parenthesis
        else if (tok == tLParen) {
            stackCurr->type = TYPE_FUNCTION + (nestedDets << 4);
            stackCurr->operand = token;
            stackElements++;
        }
        
        // Pop a right parenthesis
        else if (tok == tRParen || tok == tComma) {
            // Move until stack is empty or a function is encountered
            while (stackElements) {
                stackPrev = &stackPtr[stackElements-1];
                outputCurr = &outputPtr[outputElements];
                if ((stackPrev->type & 15) != TYPE_FUNCTION) {
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
                outputCurr->type = stackPrev->type;
                outputCurr->operand = stackPrev->operand + ((*amountOfArgumentsStackPtr--) << 8);
                stackElements--;
                outputElements++;
                if ((uint8_t)stackPrev->operand == tDet) {
                    nestedDets--;
                }
            }
        } 
        
        // Process a function
        else if (strchr(implementedFunctions, tok)) {
            *++amountOfArgumentsStackPtr = 0;
            if (tok == tDet) {
                nestedDets++;
            }
            stackCurr->type = TYPE_FUNCTION + (nestedDets << 4);
            stackCurr->operand = token;
            stackElements++;
        }
        
        // Process a function that returns something (rand, getKey(X))
        else if (tok == tRand || tok == tGetKey) {
            outputCurr->type = TYPE_FUNCTION_RETURN + (nestedDets << 4);
            outputCurr->operand = token;
            outputElements++;
            
            // Check for fast key input, i.e. getKey(X)
            if (tok == tGetKey) {
                // The next token must be a left parenthesis
                if ((uint8_t)(token = getc()) != tLParen) {
                    continue;
                }
                
                // The next token must be a number
                if ((uint8_t)(token = getc()) >= t0 && token <= t9) {
                    tok = (uint8_t)token - t0;
                    
                    // The next token can be a number, but also right parenthesis or EOF
                    if ((uint8_t)(token = getc()) >= t0 && token <= t9) {
                        // Add the direct key to the operand
                        outputCurr->operand = tGetKey | (1 << 8) | ((tok * 10 + (uint8_t)token - t0) << 16);
                        if ((token = getc()) == EOF || token == tRParen) {
                            continue;
                        } else {
                            return E_SYNTAX;
                        }
                    } else if ((uint8_t)token == tRParen || token == EOF) {
                        // Add the direct key to the operand
                        outputCurr->operand = tGetKey | (1 << 8) | (tok << 16);
                    } else {
                        return E_SYNTAX;
                    }
                } else {
                    return E_SYNTAX;
                }
            }
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
    stackToOutputReturn = 2;
    goto stackToOutput;
stackToOutputReturn2:

    // Remove stupid things like 2+5
    for (loopIndex = 2; loopIndex < outputElements; loopIndex++) {
        outputPrevPrev = &outputPtr[loopIndex-2];
        outputPrev = &outputPtr[loopIndex-1];
        outputCurr = &outputPtr[loopIndex];
        
        // Check if the types are number | number | operator
        if ((outputPrevPrev->type & 15) == TYPE_NUMBER && (outputPrev->type & 15) == TYPE_NUMBER && (outputCurr->type & 15) == TYPE_OPERATOR) {
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
        
        // Expression is an empty function or operator, i.e. not(, +
        else {
            return E_SYNTAX;
        }
        return VALID;
    }
    
    // This can only happen with a function with a single argument, i.e. det(X), not(X)
    else if (outputElements == 2) {
        outputCurr = &outputPtr[1];
        
        // It must be a function with a single argument
        if ((outputCurr->type & 15) != TYPE_FUNCTION) {
            return E_SYNTAX;
        }
        
        // TODO
    }
    
    // Parse the expression in postfix notation!
    nestedDets = 0;
    for (loopIndex = 1; loopIndex < outputElements; loopIndex++) {
        uint8_t typeMasked;

        outputCurr = &outputPtr[loopIndex];
        typeMasked = outputCurr->type & 15;

        // Parse an operator with 2 arguments
        if (typeMasked == TYPE_OPERATOR && loopIndex > 1) {
            // Yay, we are entering a det() function (#notyay), so store it elsewhere!
            if ((outputCurr->type & 240) != nestedDets) {
                // Push the old programPtr
                push((uint24_t)ice.programPtr);
                nestedDets = outputCurr->type & 240;
                
                // Place the new code at a temp location
                ice.programPtr = tempCFunctions + (500 * (nestedDets >> 4));
            }
            parseOperator(&outputPtr[loopIndex-2], &outputPtr[loopIndex-1], outputCurr);
            
            // Remove the second argument and the operator...
            memcpy(&outputPtr[loopIndex-1], &outputPtr[loopIndex+1], outputElements * 4);
            loopIndex -= 2;
            
            // ... and edit the first argument to be in the chain; if one of the two next entries is either an operator or function, set it as 'chain ans'
            (&outputPtr[loopIndex])->type = TYPE_CHAIN_PUSH - ((outputCurr->type & 15) >= TYPE_OPERATOR || ((&outputPtr[loopIndex-1])->type & 15) >= TYPE_OPERATOR);
        } 
        
        // Parse a function with X arguments
        else if (typeMasked == TYPE_FUNCTION) {
            // TODO
            
            // Yay, we can restore our programPtr!
            if ((uint8_t)outputCurr->operand == tDet) {
                ice.programPtr = (uint8_t *)pop();
            }
        }
    }

    // Actual return here
    return VALID;

    // Duplicated function opt
stackToOutput:

    // Move entire stack to output
    while (stackElements) {
        outputCurr = &outputPtr[outputElements++];
        stackPrev = &stackPtr[--stackElements];
        outputCurr->type = stackPrev->type;
        temp = stackPrev->operand;
        
        // If it's a function, add the amount of arguments as well
        if ((stackPrev->type & 15) == TYPE_FUNCTION) {
            temp += (*amountOfArgumentsStackPtr--) << 8;
        }

        outputCurr->operand = temp;
    }
    
    // Select correct return location
    if (stackToOutputReturn == 2) {
        goto stackToOutputReturn2;
    }

    goto stackToOutputReturn1;
}

static uint8_t functionI(unsigned int token) {
    uint8_t a = 0, b = 0, outputByte, tok;
    const char *dataString;
    const uint8_t colorTable[16] = {255,24,224,0,248,36,227,97,9,19,230,255,181,107,106,74};
    unsigned int offset;

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
            memcpy(ice.programData + 600, ice.programData, 116);
            ice.programPtr = ice.programData;
            
            // Insert "jp <random>" and Cesium header
            *ice.programPtr = OP_JP;
            *(uint24_t*)(ice.programPtr+4) = 0x101001;
            ice.programPtr += 7;
            
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
                *ice.programPtr++ = colorTable[outputByte];
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
                        memcpy(ice.programPtr, dataString, tokLength);
                        ice.programPtr += tokLength;
                    }
                } else if (token != EOF) {
                    ti_Seek(-1, SEEK_CUR, ice.inPrgm);
                }
            }

            // Don't increment the pointer for now, we will do that later :)
            *ice.programPtr = 0;

            // Get the correct offset
            offset = ice.programPtr - ice.programData;

            // Write the right jp offset
            *(uint24_t*)(ice.programData+1) = offset + PRGM_START;
            
            // Copy header back, and update the 3 pointers in the C header...
            memcpy(ice.programPtr + 1, ice.programData + 600, 116);
            *(uint24_t*)(ice.programPtr+2)  += offset;
            *(uint24_t*)(ice.programPtr+53) += offset;
            *(uint24_t*)(ice.programPtr+66) += offset;
            ice.programPtr += 117;
            
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


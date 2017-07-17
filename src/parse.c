#include "parse.h"

#include "operator.h"
#include "main.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "output.h"

#ifndef COMPUTER_ICE
#include <fileioc.h>
#include <debug.h>
#endif

#include <tice.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern uint8_t (*functions[256])(unsigned int token, ti_var_t currentProgram);
const char implementedFunctions[] = {tNot, tMin, tMax, tMean, tSqrt, tDet};
const char implementedFunctions2[] = {tRemainder, tSub, tLength, 0x97 /* toString( */};
const uint24_t OSString[]= {
    pixelShadow + 54000,
    pixelShadow + 55000,
    pixelShadow + 56000,
    pixelShadow + 57000,
    pixelShadow + 58000,
    pixelShadow + 59000,
    pixelShadow + 60000,
    pixelShadow + 61000,
    pixelShadow + 62000,
    pixelShadow + 63000
};

uint8_t outputStack[4096];

uint8_t parseProgram(ti_var_t currentProgram) {
    unsigned int token;
    uint8_t ret = VALID;

    // Do things based on the token
    while ((int)(token = __getc()) != EOF) {
        if ((uint8_t)token != tii) {
            ice.usedCodeAfterHeader = true;
        }
        
        // This function parses per line
        ice.currentLine++;
        ice.lastTokenIsReturn = false;

        if ((ret = (*functions[(uint8_t)token])(token, currentProgram)) != VALID) {
            break;
        }
    }

    return ret;
}

/* Static functions */

uint8_t parseExpression(unsigned int token, ti_var_t currentProgram) {
    uint8_t stack[1500];
    unsigned int stackElements = 0;
    unsigned int loopIndex, temp;
    uint8_t index = 0, a;
    uint8_t amountOfArgumentsStack[20];
    uint8_t *amountOfArgumentsStackPtr = amountOfArgumentsStack;
    uint8_t stackToOutputReturn;

    // Setup pointers
    element_t *outputPtr = (element_t*)outputStack;
    element_t *stackPtr  = (element_t*)stack;
    element_t *outputCurr, *outputPrev, *outputPrevPrev;
    element_t *stackCurr, *stackPrev = NULL;
    ice.outputElements = 0;
    
    /*
        General explanation stacks:
        - Each entry consists of 4 bytes, the type and the operand
        - Type: number, variable, function, operator etc
        - The operand is either a 3-byte number or consists of 3 bytes:
            - The first byte = the operand: function/variable/operator
            - If it's a function then the second byte is the amount of arguments for that function
            - If it's a getKeyFast, the second byte is the key
            - If it's a 2-byte function, the third byte is the second byte of the function
    */

    while ((int)token != EOF && (uint8_t)token != tEnter) {
        uint8_t tok;
        
        outputCurr = &outputPtr[ice.outputElements];
        stackCurr  = &stackPtr[stackElements];
        tok = (uint8_t)token;

        // Process a number
        if (tok >= t0 && tok <= t9) {
            uint24_t output = token - t0;
            while ((uint8_t)(token = __getc()) >= t0 && (uint8_t)token <= t9) {
                output = output*10 + (uint8_t)token - t0;
            }
            outputCurr->type = TYPE_NUMBER;
            outputCurr->operand = output;
            ice.outputElements++;

            // Don't grab a new token
            continue;
        }

        // Process a variable
        else if (tok >= tA && tok <= tTheta) {
            outputCurr->type = TYPE_VARIABLE;
            outputCurr->operand = tok - tA;
            ice.outputElements++;
        }
        
        // Parse an operator
        else if ((index = getIndexOfOperator(tok))) {
            // If the token is ->, move the entire stack to the output, instead of checking the precedence
            if (tok == tStore) {
                // Move entire stack to output
                stackToOutputReturn = 1;
                goto stackToOutput;
stackToOutputReturn1:;
            }
            
            // Move the stack to the output as long as it's not empty
            while (stackElements) {
                stackPrev = &stackPtr[stackElements-1];
                outputCurr = &outputPtr[ice.outputElements];
                
                // Move the last entry of the stack to the ouput if it's precedence is greater than the precedence of the current token
                if (stackPrev->type == TYPE_OPERATOR && operatorPrecedence[index - 1] <= operatorPrecedence2[getIndexOfOperator(stackPrev->operand) - 1]) {
                    outputCurr->type = stackPrev->type;
                    outputCurr->operand = stackPrev->operand;
                    stackElements--;
                    ice.outputElements++;
                } else {
                    break;
                }
            }
            
            // Push the operator to the stack
            stackCurr = &stackPtr[stackElements++];
            stackCurr->type = TYPE_OPERATOR;
            stackCurr->operand = token;
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
                outputCurr = &outputPtr[ice.outputElements];
                if (stackPrev->type != TYPE_FUNCTION) {
                    outputCurr->type = stackPrev->type;
                    outputCurr->operand = stackPrev->operand;
                    stackElements--;
                    ice.outputElements++;
                } else {
                    break;
                }
            }
            
            // No matching left parenthesis
            if (!stackElements) {
                if (expr.inFunction) {
                    ice.tempToken = tok;
                    goto stopParsing;
                }
                return E_EXTRA_RPAREN;
            }
            
            // If it's a det, add an argument delimiter as well
            if (tok == tComma && (uint8_t)stackPrev->operand == tDet) {
                outputCurr->type = TYPE_ARG_DELIMITER;
                ice.outputElements++;
            }
            
            // If the right parenthesis belongs to a function, move the function as well
            if (tok == tRParen && stackPrev->operand != tLParen) {
                outputCurr->type = stackPrev->type;
                outputCurr->operand = stackPrev->operand + ((*amountOfArgumentsStackPtr--) << 8);
                stackElements--;
                ice.outputElements++;
            }
            
            // Increment the amount of arguments for that function
            else {
                (*amountOfArgumentsStackPtr)++;
            }
        }
        
        // Process a function
        else if (strchr(implementedFunctions, tok) || tok == t2ByteTok || tok == tExtTok) {
            if (tok == t2ByteTok || tok == tExtTok) {
                if (!strchr(implementedFunctions2, tok = (uint8_t)__getc())) {
                    return E_SYNTAX;
                }
                token = token + (tok << 16);
            }
            // We always have at least 1 argument
            *++amountOfArgumentsStackPtr = 1;
            stackCurr->type = TYPE_FUNCTION;
            stackCurr->operand = token;
            stackElements++;
            
            // Check if it's a C function
            if (tok == tDet) {
                outputCurr->type = TYPE_C_START;
                ice.outputElements++;
            }
        }
        
        // Process a function that returns something (rand, getKey(X))
        else if (tok == tRand || tok == tGetKey) {
            outputCurr->type = TYPE_FUNCTION_RETURN;
            outputCurr->operand = token;
            ice.outputElements++;
            
            // Check for fast key input, i.e. getKey(X)
            if (tok == tGetKey) {
                // The next token must be a left parenthesis
                if ((uint8_t)(token = __getc()) != tLParen) {
                    continue;
                }
                
                // The next token must be a number
                if ((uint8_t)(token = __getc()) >= t0 && (uint8_t)token <= t9) {
                    tok = (uint8_t)token - t0;
                    
                    // The next token can be a number, but also right parenthesis or EOF
                    if ((uint8_t)(token = __getc()) >= t0 && (uint8_t)token <= t9) {
                        // Add the direct key to the operand
                        outputCurr->operand = tGetKey + ((tok * 10 + (uint8_t)token - t0) << 8);
                        if ((uint8_t)(token = __getc()) == tStore || (uint8_t)token == tEnter) {
                            // Don't grab new token
                            continue;
                        } else if ((int)token != EOF && (uint8_t)token != tRParen) {
                            return E_SYNTAX;
                        }
                    } else if ((uint8_t)token == tRParen || (int)token == EOF || (uint8_t)token == tStore || (uint8_t)token == tEnter) {
                        // Add the direct key to the operand
                        outputCurr->operand = tGetKey + (tok << 8);
                        if ((uint8_t)token == tStore || (uint8_t)token == tEnter) {
                            // Don't grab new token
                            continue;
                        }
                    } else {
                        return E_SYNTAX;
                    }
                } else {
                    return E_SYNTAX;
                }
            }
        }
        
        // Parse a string
        else if (tok == tString) {
            outputCurr->type = TYPE_STRING;
            outputCurr->operand = (uint24_t)ice.programDataPtr;
            ice.outputElements++;
            while ((int)(token = __getc()) != EOF && (uint8_t)token != tString && (uint8_t)token != tStore && (uint8_t)token != tEnter) {
                *ice.programDataPtr++ = (uint8_t)token;
            }
            *ice.programDataPtr++ = 0;
            if ((uint8_t)token == tStore || (uint8_t)token == tEnter) {
                continue;
            }

        }
        
        // Parse an OS string
        else if (tok == tVarStrng) {
            outputCurr->type = TYPE_OS_STRING;
            outputCurr->operand = OSString[(uint8_t)__getc()];
            ice.outputElements++;
        }
        
        // Oops, unknown token...
        else {
            return E_UNIMPLEMENTED;
        }
       
        token = __getc();
    }
    
    // If the expression quits normally, rather than an argument seperator
    ice.tempToken = tEnter;
    
stopParsing:
    // Move entire stack to output
    stackToOutputReturn = 2;
    goto stackToOutput;
stackToOutputReturn2:

    // Remove stupid things like 2+5, and not(1, max(2,3
    for (loopIndex = 1; loopIndex < ice.outputElements; loopIndex++) {
        outputPrevPrev = &outputPtr[loopIndex-2];
        outputPrev = &outputPtr[loopIndex-1];
        outputCurr = &outputPtr[loopIndex];
        index = (uint8_t)(outputCurr->operand >> 8);
        
        // Check if the types are number | number | operator
        if (loopIndex > 1 && outputPrevPrev->type == TYPE_NUMBER && outputPrev->type == TYPE_NUMBER && outputCurr->type == TYPE_OPERATOR) {
            // If yes, execute the operator, and store it in the first entry, and remove the other 2
            outputPrevPrev->operand = executeOperator(outputPrevPrev->operand, outputPrev->operand, (uint8_t)outputCurr->operand);
            memcpy(outputPrev, &outputPtr[loopIndex+1], (ice.outputElements-1)*4);
            ice.outputElements -= 2;
            loopIndex--;
            continue;
        }
        
        // Check if the types are number | number | ... | function (not det)
        if (loopIndex >= index && outputCurr->type == TYPE_FUNCTION && (uint8_t)outputCurr->operand != tDet) {
            for (a = 1; a <= index; a++) {
                if ((&outputPtr[loopIndex-a])->type != TYPE_NUMBER) {
                    goto DontDeleteFunction;
                }
            }
            // The function has only numbers as argument, so remove them as well :)
            switch ((uint8_t)outputCurr->operand) {
                case tNot:
                    temp = !outputPrev->operand;
                    break;
                case tMin:
                    temp = (outputPrev->operand < outputPrevPrev->operand) ? outputPrev->operand : outputPrevPrev->operand;
                    break;
                case tMax:
                    temp = (outputPrev->operand > outputPrevPrev->operand) ? outputPrev->operand : outputPrevPrev->operand;
                    break;
                case tMean:
                    temp = (outputPrev->operand + outputPrevPrev->operand) / 2;
                    break;
                default:
                    temp = sqrt(outputPrev->operand);
            }
            
            // And remove everything
            (&outputPtr[loopIndex - index])->operand = temp;
            memcpy(&outputPtr[loopIndex - index + 1], &outputPtr[loopIndex+1], (ice.outputElements-1)*4);
            ice.outputElements -= index;
            loopIndex -= index - 1;
DontDeleteFunction:;
        }
    }
    
    // Check if the expression is valid
    if (!ice.outputElements) {
        return E_SYNTAX;
    }
    
    return parsePostFixFromIndexToIndex(0, ice.outputElements - 1);

    // Duplicated function opt
stackToOutput:
    // Move entire stack to output
    while (stackElements) {
        outputCurr = &outputPtr[ice.outputElements++];
        stackPrev = &stackPtr[--stackElements];
        
        // Don't move the left paren...
        if (stackPrev->type == TYPE_FUNCTION && (uint8_t)stackPrev->operand == tLParen) {
            ice.outputElements--;
            continue;
        }
        
        outputCurr->type = stackPrev->type;
        temp = stackPrev->operand;
        
        // If it's a function, add the amount of arguments as well
        if (stackPrev->type == TYPE_FUNCTION) {
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

uint8_t parsePostFixFromIndexToIndex(uint24_t startIndex, uint24_t endIndex) {
    element_t *outputCurr;
    element_t *outputPtr = (element_t*)outputStack;
    uint8_t outputType, temp, operandDepth = 0;
    uint24_t outputOperand, loopIndex, tempIndex = 0, operand1Index, operand2Index, amountOfStackElements;
    
    // Set some variables
    outputCurr = &outputPtr[startIndex];
    outputType = outputCurr->type;
    outputOperand = outputCurr->operand;
    ice.stackStart = (uint24_t*)(ice.stackDepth * STACK_SIZE + ice.stack);
    setStackValues(ice.stackStart, ice.stackStart);
    
    // Clean the expr struct
    memset(&expr, 0, sizeof expr);
    
    // Get all the indexes of the expression
    temp = 0;
    amountOfStackElements = 0;
    for (loopIndex = startIndex; loopIndex <= endIndex; loopIndex++) {
        outputCurr = &outputPtr[loopIndex];
        
        // If it's the start of a det(, increment the amount of nested dets
        if (outputCurr->type == TYPE_C_START) {
            temp++;
        }
        // If it's a det(, decrement the amount of nested dets
        if (outputCurr->type == TYPE_FUNCTION && (uint8_t)outputCurr->operand == tDet) {
            temp--;
        }
        
        // If not in a nested det(, push the index
        if (!temp) {
            push(loopIndex);
            amountOfStackElements++;
        }
    }
    
    // It's a single entry
    if (amountOfStackElements == 1) {
        // Expression is only a single number
        if (outputType == TYPE_NUMBER) {
            // This boolean is set, because loops may be optimized when the condition is a number
            expr.outputIsNumber = true;
            expr.outputNumber = outputOperand;
            LD_HL_NUMBER(outputOperand);
        } 
        
        // Expression is only a variable
        else if (outputType == TYPE_VARIABLE) {
            expr.outputIsVariable = true;
            LD_HL_IND_IX_OFF(outputOperand);
        } 
        
        // Expression is only a function without arguments that returns something (getKey, rand)
        else if (outputType == TYPE_FUNCTION_RETURN) {
            insertFunctionReturn(outputOperand, OUTPUT_IN_HL, NO_PUSH);
        }
        
        // It is a det(
        else if (outputType == TYPE_C_START) {
            return parseFunction(getNextIndex());
        }
        
        // It's a string
        else if (outputType == TYPE_STRING || outputType == TYPE_OS_STRING) {
            expr.outputIsString = true;
            LD_HL_STRING(outputOperand);
        }
        
        // Expression is an empty function or operator, i.e. not(, +
        else {
            return E_SYNTAX;
        }
        
        return VALID;
    } else if (amountOfStackElements == 2) {
        getNextIndex();
        outputCurr = &outputPtr[tempIndex = getNextIndex()];
        
        // It should be a function with a single argument, i.e. det(0 / not(A
        if (outputCurr->type != TYPE_FUNCTION) {
            return E_SYNTAX;
        }
        
        return parseFunction(tempIndex);
    }
    
    // 3 or more entries, full expression
    do {
        outputCurr = &outputPtr[loopIndex = getNextIndex()];
        outputType = outputCurr->type;
        
        // Clear this flag
        expr.AnsSetZeroFlagReversed = false;
        
        if (outputType == TYPE_OPERATOR) {
            element_t *outputPrev, *outputPrevPrev;
            // Wait, invalid operator?!
            if (loopIndex < startIndex + 2) {
                return E_SYNTAX;
            }
            
            operand2Index = getIndexOffset(-2);
            operand1Index = getIndexOffset(-3);
            
            outputPrev = &outputPtr[operand2Index];
            outputPrevPrev = &outputPtr[operand1Index];
            
            // Parse the operator with the 2 latest operands of the stack!
            if ((temp = parseOperator(outputPrevPrev, outputPrev, outputCurr)) != VALID) {
                return temp;
            }
        
            // Remove the index of the first and the second argument, the index of the operator will be the chain
            removeIndexFromStack(getCurrentIndex() - 2);
            removeIndexFromStack(getCurrentIndex() - 2);
            
            // Check if it was a command with 2 strings
            if (outputCurr->operand == tAdd && outputPrevPrev->type >= TYPE_STRING && outputPrev->type >= TYPE_STRING) {
                outputCurr->type = TYPE_STRING;
                outputCurr->operand = (outputPrevPrev->operand == TempString2 || outputPrev->operand == TempString1) ? TempString2 : TempString1;
            } else {
                operandDepth = 3;
            }
            tempIndex = loopIndex;
        }
        
        else if (outputType == TYPE_FUNCTION) {
            // Use this to cleanup the function after parsing
            uint8_t amountOfArguments = (uint8_t)(outputCurr->operand >> 8);
            
            if ((temp = parseFunction(loopIndex)) != VALID) {
                return temp;
            }
            
            // Cleanup, only if it's NOT a det(
            if ((uint8_t)outputCurr->operand != tDet) {
                for (temp = 0; temp < amountOfArguments; temp++) {
                    removeIndexFromStack(getCurrentIndex() - 2);
                }
            }
            
            // Check chain push/ans
            operandDepth = 3;
            tempIndex = loopIndex;
        }
        
        // Check if the next or next next operand is either a function operator
        if (operandDepth == 3) {
            outputCurr->type = TYPE_CHAIN_ANS;
        } else if (operandDepth == 1) {
            // We need to push HL since it isn't used in the next operator/function
            (&outputPtr[tempIndex])->type = TYPE_CHAIN_PUSH;
            PushHLDE();
        }
        
        if (operandDepth) {
            operandDepth--;
        }
    } while (loopIndex != endIndex);
    
    return VALID;
}

static uint8_t functionI(unsigned int token, ti_var_t currentProgram) {
    const uint8_t colorTable[16] = {255,24,224,0,248,36,227,97,9,19,230,255,181,107,106,74};    // Thanks Cesium :D

    // Only get the output name, icon or description at the top of your program
    if (!ice.usedCodeAfterHeader) {
        unsigned int offset;
        
        // Get the output name
        if (!ice.gotName) {
            uint8_t a = 0;
            while ((int)(token = __getc()) != EOF && (uint8_t)token != tEnter && a < 9) {
                ice.outName[a++] = (uint8_t)token;
            }
            ice.gotName = true;
            return VALID;
        }

        // Get the icon and description
        else if (!ice.gotIconDescription) {
            uint8_t b = 0;
            // Move header to take place for the icon and description, setup pointer
            memcpy(ice.programData + 600, ice.programData, ice.programSize);
            ice.programPtr = ice.programData;
            
            // Insert "jp <random>" and Cesium header
            *ice.programPtr = OP_JP;
            w24(ice.programPtr + 4, 0x101001);
            ice.programPtr += 7;
            
            // Icon should start with a "
            if ((uint8_t)__getc() != tString) {
                return E_WRONG_ICON;
            }

            // Get hexadecimal
            do {
                uint8_t tok;
                if ((tok = IsHexadecimal(__getc())) == 16) {
                    return E_INVALID_HEX;
                }
                *ice.programPtr++ = colorTable[tok];
            } while (++b);
            
            // Move on to the description
            if ((uint8_t)(token = __getc()) == tString) {
                token = __getc();
            }

            if ((int)token != EOF) {
                
                if ((uint8_t)token != tEnter) {
                    return E_SYNTAX;
                }
                
                // Check if there is a description
                if ((token = __getc()) == tii) {
#ifndef COMPUTER_ICE
                    uint8_t *dataPtr = ti_GetDataPtr(ice.inPrgm);
                    
                    // Grab description
                    while ((int)(token = __getc()) != EOF && (uint8_t)token != tEnter) {
                        unsigned int strLength;
                        const char *dataString;
                        uint8_t tokSize;
                        
                        // Get the token in characters, and copy to the output
                        dataString = ti_GetTokenString(&dataPtr, &tokSize, &strLength);
                        memcpy(ice.programPtr, dataString, strLength);
                        ice.programPtr += strLength;
                        
                        // If it's a 2-byte token, we also need to get the second byte of it
                        if (tokSize == 2) {
                            __getc();
                        }
                    }
#else
                    while ((int)(token = __getc()) != EOF && (uint8_t)token != tEnter) {
                        const uint8_t token2byte[] = { 0x5C, 0x5D, 0x5E, 0x60, 0x61, 0x62, 0x63, 0x7E, 0xBB, 0xAA, 0xEF };
                        *ice.programPtr++ = (uint8_t)token;
                        if (memchr(token2byte, token, sizeof token2byte)) {
                            __getc();
                        }
                    }
#endif
                } else if ((int)token != EOF) {
                    setCurrentOffset(-1, SEEK_CUR, ice.inPrgm);
                }
            }

            // Don't increment the pointer for now, we will do that later :)
            *ice.programPtr = 0;

            // Get the correct offset
            offset = ice.programPtr - ice.programData + 1;

            // Write the right jp offset
            w24(ice.programData + 1, offset + PRGM_START);
            
            // Copy header back
            memcpy(ice.programPtr + 1, ice.programData + 600, ice.programSize);
            
            // If C functions were detected, update the pointers
            if (ice.programSize > 10) {
                w24(ice.programPtr + 2, r24(ice.programPtr + 2) + offset);
                w24(ice.programPtr + 53, r24(ice.programPtr + 53) + offset);
                w24(ice.programPtr + 66, r24(ice.programPtr + 66) + offset);
            }
            ice.programPtr += ice.programSize + 1;
            
            ice.gotIconDescription = true;
            return VALID;
        }
        
        // Don't return and treat as a comment
        else {
            ice.usedCodeAfterHeader = true;
        }
    }

    // Treat it as a comment
    skipLine(currentProgram);

    return VALID;
}

static uint8_t functionIf(unsigned int token, ti_var_t currentProgram) {
    uint8_t res, tempZR, tempC, tempCR, insertJRCZReturn;
    uint8_t *IfStartAddr, *IfElseAddr = NULL;
    uint24_t tempDataOffsetElements, tempDataOffsetElements2;
    
    if ((int)(token = __getc()) != EOF && token != tEnter) {
        // Parse the argument
        if ((res = parseExpression(token, currentProgram)) != VALID) {
            return res;
        }
        
        if (expr.outputIsString) {
            return E_SYNTAX;
        }
        
        //Check if we can optimize stuff :D
        optimizeZeroCarryFlagOutput();
        
        // Backup stuff
        IfStartAddr = ice.programPtr;
        tempDataOffsetElements = ice.dataOffsetElements;
        tempZR = expr.AnsSetZeroFlagReversed;
        tempC  = expr.AnsSetCarryFlag;
        tempCR = expr.AnsSetCarryFlagReversed;
        
        if (tempC) {
            if (tempCR) {
                JP_NC(0);
            } else {
                JP_C(0);
            }
        } else {
            if (tempZR) {
                JP_NZ(0);
            } else {
                JP_Z(0);
            }
        }
        res = parseProgram(currentProgram);
        
        // Needs to be the end of a line
        if (!CheckEOL(currentProgram)) {
            return E_SYNTAX;
        }
        
        // Check if we quit the program with an 'Else'
        if (res == E_ELSE) {
            // Backup stuff
            IfElseAddr = ice.programPtr;
            tempDataOffsetElements2 = ice.dataOffsetElements;
            
            JP(0);
            res = parseProgram(currentProgram);
            if (res != E_END && res != VALID) {
                return res;
            }
            
            // Needs to be the end of a line
            if (!CheckEOL(currentProgram)) {
                return E_SYNTAX;
            }
            
            // Check if we can change the "jp" to a "jr" from the Else code
            if (ice.programPtr - IfElseAddr - 2 < 0x80) {
                // Update all the pointers to the data section
                UpdatePointersToData(tempDataOffsetElements2);

                // And finally insert the "jr", and move the code
                *IfElseAddr++ = OP_JR;
                *IfElseAddr = ice.programPtr - IfElseAddr - 3;
                IfElseAddr++;
                memcpy(IfElseAddr, IfElseAddr + 2, 0x7F);
                ice.programPtr -= 2;
            } else {
                w24(IfElseAddr + 1, ice.programPtr + PRGM_START - ice.programData);
                IfElseAddr += 4;
            }
            
            // Check if we can change the "jp" to a "jr" from the If code
            if (IfElseAddr - IfStartAddr < 0x80 + 4) {
                // Update all the pointers to the data section
                while (ice.dataOffsetElements != tempDataOffsetElements) {
                    ice.dataOffsetStack[tempDataOffsetElements] -= 2;
                }
                // And finally insert the "jr (n)z/c", and move the code
                insertJRCZReturn = 1;
                goto insertJRCZ;
insertJRCZReturn1:
                *IfStartAddr = IfElseAddr - IfStartAddr - 3;
                IfStartAddr++;
                memcpy(IfStartAddr, IfStartAddr + 2, ice.programPtr - IfStartAddr);
                ice.programPtr -= 2;
            } else {
                w24(IfStartAddr + 1, IfElseAddr + PRGM_START - ice.programData);
                IfStartAddr += 4;
            }
        }
        
        // Check if we quit the program with an 'End' or at the end of the input program
        else if (res == E_END || res == VALID) {
            // Check if we can change the "jp" to a "jr"
            if (ice.programPtr - IfStartAddr - 2 < 0x80) {
                // Update all the pointers to the data section
                UpdatePointersToData(tempDataOffsetElements);

                // And finally insert the "jr (n)z/c", and move the code
                insertJRCZReturn = 2;
                goto insertJRCZ;
insertJRCZReturn2:
                *IfStartAddr = ice.programPtr - IfStartAddr - 3;
                IfStartAddr++;
                memcpy(IfStartAddr, IfStartAddr + 2, 0x7F);
                ice.programPtr -= 2;
            } else {
                w24(IfStartAddr + 1, ice.programPtr + PRGM_START - ice.programData);
            }
        } else {
            return res;
        }
        return VALID;
    } else {
        return E_NO_CONDITION;
    }
    
    // Duplicated function opt
insertJRCZ:
    if (tempC) {
        if (tempCR) {
            *IfStartAddr++ = OP_JR_NC;
        } else {
            *IfStartAddr++ = OP_JR_C;
        }
    } else {
        if (tempZR) {
            *IfStartAddr++ = OP_JR_NZ;
        } else {
            *IfStartAddr++ = OP_JR_Z;
        }
    }
    if (insertJRCZReturn == 1) {
        goto insertJRCZReturn1;
    }
    goto insertJRCZReturn2;
}

static uint8_t functionElse(unsigned int token, ti_var_t currentProgram) {
    return E_ELSE;
}

static uint8_t functionEnd(unsigned int token, ti_var_t currentProgram) {
    return E_END;
}

static uint8_t dummyReturn(unsigned int token, ti_var_t currentProgram) {
    return VALID;
}

uint8_t *WhileRepeatCondStart = NULL;

void UpdatePointersToData(uint24_t tempDataOffsetElements) {
    while (ice.dataOffsetElements != tempDataOffsetElements) {
        ice.dataOffsetStack[tempDataOffsetElements] = (uint24_t*)(((uint8_t*)ice.dataOffsetStack[tempDataOffsetElements]) - 2);
        tempDataOffsetElements++;
    }
}

static uint8_t functionWhile(unsigned int token, ti_var_t currentProgram) {
    uint24_t tempDataOffsetElements = ice.dataOffsetElements;
    uint8_t *WhileStartAddr = ice.programPtr, *TempStartAddr = WhileStartAddr, res;
    
    // Basically the same as "Repeat", but jump to condition checking first
    JP(0);
    if ((res = functionRepeat(token, currentProgram)) != VALID) {
        return res;
    }
    
    // Check if we can replace the "jp" with a "jr"
    if (WhileRepeatCondStart - WhileStartAddr + 2 < 0x80) {
        *WhileStartAddr++ = OP_JR;
        *WhileStartAddr++ = WhileRepeatCondStart - TempStartAddr - 4;
        UpdatePointersToData(tempDataOffsetElements);
        memcpy(WhileStartAddr, WhileStartAddr + 2, 0x80);
        ice.programPtr -= 2;
    } else {
        w24(WhileStartAddr + 1, WhileRepeatCondStart + PRGM_START - ice.programData);
    }
    
    return VALID;
}

uint8_t functionRepeat(unsigned int token, ti_var_t currentProgram) {
    uint16_t RepeatCondStart, RepeatProgEnd;
    uint8_t *RepeatCodeStart, *RepeatCondEnd, res;
    
    RepeatCondStart = getCurrentOffset(currentProgram);
    RepeatCodeStart = ice.programPtr;
    
    // Skip the condition for now
    skipLine(currentProgram);
    
    // Parse the code
    if ((res = parseProgram(currentProgram)) != E_END && res != VALID) {
        return res;
    }
    
    // Needs to be the end of a line
    if (!CheckEOL(currentProgram)) {
        return E_SYNTAX;
    }

    // Remind where the "End" is
    RepeatProgEnd = getCurrentOffset(currentProgram);
    WhileRepeatCondStart = ice.programPtr;
    
    // Parse the condition
    setCurrentOffset(RepeatCondStart, SEEK_SET, currentProgram);
    if ((res = parseExpression(__getc(), currentProgram)) != VALID) {
        return res;
    }
    
    if (expr.outputIsString) {
        return E_SYNTAX;
    }
    
    // And set the pointer after the "End"
    setCurrentOffset(RepeatProgEnd, SEEK_SET, currentProgram);
    optimizeZeroCarryFlagOutput();
    RepeatCondEnd = ice.programPtr;

    if (expr.AnsSetCarryFlag) {
        if (expr.AnsSetCarryFlagReversed) {
            JP_NC(0);
        } else {
            JP_C(0);
        }
    } else {
        if (expr.AnsSetZeroFlagReversed) {
            JP_NZ(0);
        } else {
            JP_Z(0);
        }
    }
    
    // Check if we can replace the "jp" with a "jr"
    if (ice.programPtr - RepeatCodeStart < 0x80 + 4 - 2) {
        if (expr.AnsSetCarryFlag) {
            if (expr.AnsSetCarryFlagReversed) {
                *RepeatCondEnd++ = OP_JR_NC;
            } else {
                *RepeatCondEnd++ = OP_JR_C;
            }
        } else {
            if (expr.AnsSetZeroFlagReversed) {
                *RepeatCondEnd++ = OP_JR_NZ;
            } else {
                *RepeatCondEnd++ = OP_JR_Z;
            }
        }
        
        // Insert the jr offset
        *RepeatCondEnd++ = RepeatCodeStart - ice.programPtr + 2;
        ice.programPtr -= 2;
    } else {
        w24(RepeatCondEnd+1, ice.programPtr + PRGM_START - ice.programData);
    }
    return VALID;
}

static uint8_t functionReturn(unsigned int token, ti_var_t currentProgram) {
    uint8_t res;
    
    if ((uint8_t)(token = __getc()) == tEnter) {
        RET();
        ice.lastTokenIsReturn = true;
    } else if (token == tIf) {
        if ((res = parseExpression(__getc(), currentProgram)) != VALID) {
            return res;
        }
        if (expr.outputIsString) {
            return E_SYNTAX;
        }
        
        //Check if we can optimize stuff :D
        optimizeZeroCarryFlagOutput();
        
        if (expr.AnsSetCarryFlag) {
            if (expr.AnsSetCarryFlagReversed) {
                RET_C();
            } else {
                RET_NC();
            }
        } else {
            if (expr.AnsSetZeroFlagReversed) {
                RET_Z();
            } else {
                RET_NZ();
            }
        }
    }
    return VALID;
}

static uint8_t functionDisp(unsigned int token, ti_var_t currentProgram) {
    do {
        uint8_t res;

        if ((uint8_t)(token = __getc()) == tii) {
            if ((int)(token = __getc()) == EOF) {
                ice.tempToken = tEnter;
            } else {
                ice.tempToken = (uint8_t)token;
            }
            CALL(_NewLine);
            goto checkArgument;
        }
        
        // Get the argument, and display it, based on whether it's a string or the outcome of an expression
        expr.inFunction = true;
        if ((res = parseExpression(token, currentProgram)) != VALID) {
            return res;
        }
        if (expr.outputIsString) {
            CALL(_PutS);
        } else {
            if (expr.outputRegister != OutputRegisterHL) {
                EX_DE_HL();
            }
            CALL(_DispHL);
        }
        
checkArgument:
        // Oops, there was a ")" after the expression
        if (ice.tempToken == tRParen) {
            return E_SYNTAX;
        }
    } while (ice.tempToken != tEnter);
    return VALID;
}

static uint8_t functionOutput(unsigned int token, ti_var_t currentProgram) {
    uint8_t res;
    
    // Get the first argument = column
    expr.inFunction = true;
    if ((res = parseExpression(__getc(), currentProgram)) != VALID) {
        return res;
    }
    
    // Return syntax error if the expression was a string or the token after the expression wasn't a comma
    if (expr.outputIsString || ice.tempToken != tComma) {
        return E_SYNTAX;
    }
    
    if (expr.outputIsNumber) {
        *(ice.programPtr - 4) = OP_LD_A;
        ice.programPtr -= 2;
        LD_IMM_A(curCol);
        
        // Get the second argument = row
        expr.inFunction = true;
        if ((res = parseExpression(__getc(), currentProgram)) != VALID) {
            return res;
        }
        if (expr.outputIsString) {
            return E_SYNTAX;
        }
        
        // Yay, we can optimize things!
        if (expr.outputIsNumber) {
            uint16_t outputCoordinates;
            ice.programPtr -= 10;
            outputCoordinates = (*(ice.programPtr + 1) << 8) + *(ice.programPtr + 7);
            LD_SIS_HL(outputCoordinates);
            LD_SIS_IMM_HL(curRow & 0xFFFF);
        } else {
            if (expr.outputIsVariable) {
                *(ice.programPtr - 2) = 0x7E;
            } else if (expr.outputRegister == OutputRegisterHL) {
                LD_A_L();
            } else {
                LD_A_E();
            }
            LD_IMM_A(curRow);
        }
    } else {
        if (expr.outputIsVariable) {
            *(ice.programPtr - 2) = 0x7E;
        } else if (expr.outputRegister == OutputRegisterHL) {
            LD_A_L();
        } else {
            LD_A_E();
        }
        LD_IMM_A(curCol);
        
        // Get the second argument = row
        expr.inFunction = true;
        if ((res = parseExpression(__getc(), currentProgram)) != VALID) {
            return res;
        }
        if (expr.outputIsString) {
            return E_SYNTAX;
        }
        
        if (expr.outputIsVariable) {
            *(ice.programPtr - 2) = 0x7E;
        } else if (expr.outputRegister == OutputRegisterHL) {
            LD_A_L();
        } else {
            LD_A_E();
        }
        LD_IMM_A(curRow);
    }
    
    // Get the third argument = output thing
    if (ice.tempToken == tComma) {
        if ((res = parseExpression(__getc(), currentProgram)) != VALID) {
            return res;
        }
        
        // Call the right function to display it
        if (expr.outputIsString) {
            CALL(_PutS);
        } else {
            if (expr.outputRegister != OutputRegisterHL) {
                EX_DE_HL();
            }
            CALL(_DispHL);
        }
    } else if (ice.tempToken != tEnter) {
        return E_SYNTAX;
    }
    
    return VALID;
}

static uint8_t functionClrHome(unsigned int token, ti_var_t currentProgram) {
    if (ice.modifiedIY) {
        LD_IY_IMM(flags);
    }
    CALL(_HomeUp);
    CALL(_ClrLCDFull);
    ice.modifiedIY = false;
    return VALID;
}

static uint8_t functionFor(unsigned int token, ti_var_t currentProgram) {
    return E_UNIMPLEMENTED;
}

static uint8_t functionPrgm(unsigned int token, ti_var_t currentProgram) {
    return E_UNIMPLEMENTED;
}

static uint8_t functionCustom(unsigned int token, ti_var_t currentProgram) {
    return E_UNIMPLEMENTED;
}

static uint8_t functionLbl(unsigned int token, ti_var_t currentProgram) {
    // Add the label to the stack, and skip the line
    *ice.LblPtr++ = (uint24_t)ice.programPtr;
    *ice.LblPtr++ = (uint24_t)ti_GetDataPtr(currentProgram);
    skipLine(currentProgram);
    return VALID;
}

static uint8_t functionGoto(unsigned int token, ti_var_t currentProgram) {
    // Add the goto to the stack, and skip the line
    *ice.GotoPtr++ = (uint24_t)ice.programPtr;
    *ice.GotoPtr++ = (uint24_t)ti_GetDataPtr(currentProgram);
    JP(0);
    skipLine(currentProgram);
    return VALID;
}

static uint8_t functionPause(unsigned int token, ti_var_t currentProgram) {
    return E_UNIMPLEMENTED;
}

static uint8_t functionInput(unsigned int token, ti_var_t currentProgram) {
    return E_UNIMPLEMENTED;
}

static uint8_t functionBB(unsigned int token, ti_var_t currentProgram) {
    // Asm(
    if ((uint8_t)(token = __getc()) == tAsm) {
        while ((int)(token = __getc()) != EOF && (uint8_t)token != tEnter && (uint8_t)token != tRParen) {
            uint8_t tok1, tok2;
            
            // Get hexadecimal 1
            if ((tok1 = IsHexadecimal(token)) == 16) {
                return E_INVALID_HEX;
            }
            
            // Get hexadecimal 2
            if ((tok2 = IsHexadecimal(__getc())) == 16) {
                return E_INVALID_HEX;
            }
            
            *ice.programPtr++ = (tok1 << 4) + tok2;
        }
        if ((uint8_t)token == tRParen) {
            if ((uint8_t)__getc() != tEnter) {
                return E_SYNTAX;
            }
        }
    } else {
        setCurrentOffset(-2, SEEK_CUR, currentProgram);
        return parseExpression(__getc(), currentProgram);
    }
    return VALID;
}

static uint8_t tokenWrongPlace(unsigned int token, ti_var_t currentProgram) {
    return E_WRONG_PLACE;
}

static uint8_t tokenUnimplemented(unsigned int token, ti_var_t currentProgram) {
    return E_UNIMPLEMENTED;
}

void optimizeZeroCarryFlagOutput(void) {
    if (!expr.AnsSetZeroFlag && !expr.AnsSetCarryFlag) {
        MaybeDEToHL();
        ADD_HL_DE();
        OR_A_A();
        SBC_HL_DE();
    } else {
        ice.programPtr -= expr.ZeroCarryFlagRemoveAmountOfBytes;
    }
    return;
}

void skipLine(ti_var_t currentProgram) {
    int token;
    
    while ((token = __getc()) != EOF && (uint8_t)token != tEnter);
}

uint8_t (*functions[256])(unsigned int, ti_var_t) = {
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
    parseExpression,    //25
    parseExpression,    //26
    tokenUnimplemented, //27
    tokenUnimplemented, //28
    tokenUnimplemented, //29
    tokenUnimplemented, //30
    tokenUnimplemented, //31
    tokenUnimplemented, //32
    parseExpression,    //33
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
    functionBB,         //187
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
    functionElse,       //208
    functionWhile,      //209
    functionRepeat,     //210
    functionFor,        //211
    functionEnd,        //212
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
    parseExpression,    //239
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

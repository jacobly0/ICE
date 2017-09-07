#include "defines.h"
#include "parse.h"

#include "operator.h"
#include "main.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "output.h"
#include "routines.h"
//#include "gfx/gfx_logos.h"

#ifdef COMPUTER_ICE
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(Pause, "src/asm/pause.bin");
#endif

extern uint8_t (*functions[256])(uint24_t token);
const char implementedFunctions[] = {tNot, tMin, tMax, tMean, tSqrt, tDet};
const char implementedFunctions2[] = {tRemainder, tSub, tLength, tToString};
uint8_t outputStack[4096];

uint8_t parseProgram(void) {
    uint24_t token;
    uint8_t ret = VALID;

    // Do things based on the token
    while ((int)(token = _getc(ice.inPrgm)) != EOF) {
        if ((uint8_t)token != tii) {
            ice.usedCodeAfterHeader = true;
        }
        
        ice.lastTokenIsReturn = false;
        ice.currentLine++;

        if ((ret = (*functions[(uint8_t)token])(token)) != VALID) {
            break;
        }
    }

    return ret;
}

/* Static functions */

uint8_t parseExpression(uint24_t token) {
    uint24_t stackElements = 0, outputElements = 0;
    uint24_t loopIndex, temp;
    uint8_t stack[1500], amountOfArgumentsStack[20];
    uint8_t index = 0, a, stackToOutputReturn, mask = TYPE_MASK_U24, tok;
    uint8_t *amountOfArgumentsStackPtr = amountOfArgumentsStack, canUseMask = 2;

    // Setup pointers
    element_t *outputPtr = (element_t*)outputStack;
    element_t *stackPtr  = (element_t*)stack;
    element_t *outputCurr, *outputPrev, *outputPrevPrev;
    element_t *stackCurr, *stackPrev = NULL;
    
    /*
        General explanation stacks:
        - Each entry consists of 5 bytes, the type, the mask and the operand
        - Type: number, variable, function, operator etc
        - Mask: 0 = 8 bits, 1 = 16 bits, 2 = 24 bits
        - The operand is either a 3-byte number or consists of these 3 bytes:
            - The first byte = the operand: function/variable/operator token
            - If it's a function then the second byte is the amount of arguments for that function
            - If it's a getKeyFast, the second byte is the key
            - If it's a 2-byte function, the third byte is the second byte of the function
    */

    while ((int)token != EOF && (tok = (uint8_t)token) != tEnter) {
        outputCurr = &outputPtr[outputElements];
        stackCurr  = &stackPtr[stackElements];
        
        // We can use the unsigned mask * only at the start of the line, or directly after an operator
        if (canUseMask) {
            canUseMask--;
        }

        // Process a number
        if (tok >= t0 && tok <= t9) {
            uint24_t output = token - t0;
            
            while ((uint8_t)(token = _getc(ice.inPrgm)) >= t0 && (uint8_t)token <= t9) {
                output = output * 10 + (uint8_t)token - t0;
            }
            outputCurr->type = TYPE_NUMBER;
            outputCurr->operand = output;
            outputElements++;
            mask = TYPE_MASK_U24;

            // Don't grab a new token
            continue;
        }
        
        // Process a hexadecimal number
        else if (tok == tee) {
            uint24_t output = 0;
            
            while ((tok = IsHexadecimal(token = _getc(ice.inPrgm))) != 16) {
                output = (output << 4) + tok;
            }
            outputCurr->type = TYPE_NUMBER;
            outputCurr->operand = output;
            outputElements++;
            mask = TYPE_MASK_U24;

            // Don't grab a new token
            continue;
        }
        
        // Process a 'negative' number
        else if (tok == tChs) {
            uint24_t output = 0;
            
            while ((uint8_t)(token = _getc(ice.inPrgm)) >= t0 && (uint8_t)token <= t9) {
                output = output * 10 + (uint8_t)token - t0;
            }
            outputCurr->type = TYPE_NUMBER;
            outputCurr->operand = 0-output;
            outputElements++;
            mask = TYPE_MASK_U24;

            // Don't grab a new token
            continue;
        }
        
        // Process an OS list (number)
        else if (tok == tVarLst) {
            outputCurr->type = TYPE_NUMBER;
            outputCurr->operand = ice.OSLists[_getc(ice.inPrgm)];
            outputElements++;
            mask = TYPE_MASK_U24;
        }

        // Process a variable
        else if (tok >= tA && tok <= tTheta) {
            outputCurr->type = TYPE_VARIABLE;
            outputCurr->operand = tok - tA;
            outputElements++;
            mask = TYPE_MASK_U24;
        }
        
        // Process a mask
        else if (tok == tMul && canUseMask) {
            uint8_t a = 0;
            
            while ((uint8_t)(token = _getc(ice.inPrgm)) == tMul) {
                a++;
            }
            
            if (a > 2 || (uint8_t)token != tLBrace) {
                return E_SYNTAX;
            }
            mask = TYPE_MASK_U8 + a;
            
            // Don't grab the { token
            continue;
        }
        
        // Parse an operator
        else if ((index = getIndexOfOperator(tok))) {
            // If the token is ->, move the entire stack to the output, instead of checking the precedence
            if (tok == tStore) {
                // Move entire stack to output
                stackToOutputReturn = 1;
                goto stackToOutput;
            }
            
            // Move the stack to the output as long as it's not empty
            while (stackElements) {
                stackPrev = &stackPtr[stackElements-1];
                outputCurr = &outputPtr[outputElements];
                
                // Move the last entry of the stack to the ouput if it's precedence is greater than the precedence of the current token
                if (stackPrev->type == TYPE_OPERATOR && operatorPrecedence[index - 1] <= operatorPrecedence2[getIndexOfOperator(stackPrev->operand) - 1]) {
                    outputCurr->type = stackPrev->type;
                    outputCurr->mask = stackPrev->mask;
                    outputCurr->operand = stackPrev->operand;
                    stackElements--;
                    outputElements++;
                } else {
                    break;
                }
            }
            
stackToOutputReturn1:
            // Push the operator to the stack
            stackCurr = &stackPtr[stackElements++];
            stackCurr->type = TYPE_OPERATOR;
            stackCurr->operand = token;
            mask = TYPE_MASK_U24;
            canUseMask = 2;
        }
        
        // Push a ( { [
        else if (tok == tLParen || tok == tLBrace || tok == tLBrack) {
            stackCurr->type = TYPE_FUNCTION;
            stackCurr->mask = mask;
            stackCurr->operand = token;
            stackElements++;
            mask = TYPE_MASK_U24;
            canUseMask = 2;
        }
        
        // Gets the address of a variable
        else if (tok == tFromDeg) {
            outputCurr->type = TYPE_NUMBER;
            outputElements++;
            mask = TYPE_MASK_U24;
            tok = (uint8_t)_getc(ice.inPrgm);
            
            // Get the address of the variable
            if (tok >= tA && tok <= tTheta) {
                outputCurr->operand = 0xD13F47 + (tok - tA) * 3;
            } else if (tok == tVarLst) {
                outputCurr->operand = ice.OSLists[_getc(ice.inPrgm)];
            } else if (tok == tVarStrng) {
                outputCurr->operand = ice.OSStrings[_getc(ice.inPrgm)];
            } else {
                return E_SYNTAX;
            }
        }
        
        // Pop a ) } ] ,
        else if (tok == tRParen || tok == tComma || tok == tRBrace || tok == tRBrack) {
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
            
            // Closing tag should match it's open tag
            if (tok != tComma && (stackPrev->operand != tok - 1)) {
                return E_SYNTAX;
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
                outputElements++;
            }
            
            // If the right parenthesis belongs to a function, move the function as well
            if (tok != tComma && stackPrev->operand != tLParen) {
                uint24_t temp2 = stackPrev->operand;
                outputCurr->type = stackPrev->type;
                outputCurr->mask = stackPrev->mask;
                if (tok == tRParen) {
                    temp2 += (*amountOfArgumentsStackPtr--) << 8;
                }
                outputCurr->operand = temp2;
                stackElements--;
                outputElements++;
            }
            
            // Increment the amount of arguments for that function
            if (tok == tComma) {
                (*amountOfArgumentsStackPtr)++;
            }
            
            mask = TYPE_MASK_U24;
        }
        
        // Process a function
        else if (strchr(implementedFunctions, tok) || tok == t2ByteTok || tok == tExtTok) {
            if (tok == t2ByteTok || tok == tExtTok) {
                if (!strchr(implementedFunctions2, tok = (uint8_t)_getc(ice.inPrgm))) {
                    return E_SYNTAX;
                }
                token = token + (tok << 16);
            }
            // We always have at least 1 argument
            *++amountOfArgumentsStackPtr = 1;
            stackCurr->type = TYPE_FUNCTION;
            stackCurr->operand = token;
            stackElements++;
            mask = TYPE_MASK_U24;
            canUseMask = 2;
            
            // Check if it's a C function
            if (tok == tDet) {
                outputCurr->type = TYPE_C_START;
                outputElements++;
            }
        }
        
        // Process a function that returns something (rand, getKey(X))
        else if (tok == tRand || tok == tGetKey) {
            outputCurr->type = TYPE_FUNCTION_RETURN;
            outputCurr->operand = token;
            outputElements++;
            mask = TYPE_MASK_U24;
            
            // Check for fast key input, i.e. getKey(X)
            if (tok == tGetKey) {
                // The next token must be a left parenthesis
                if ((uint8_t)(token = _getc(ice.inPrgm)) != tLParen) {
                    continue;
                }
                
                // The next token must be a number
                if ((uint8_t)(token = _getc(ice.inPrgm)) >= t0 && (uint8_t)token <= t9) {
                    tok = (uint8_t)token - t0;
                    
                    // The next token can be a number, but also right parenthesis or EOF
                    if ((uint8_t)(token = _getc(ice.inPrgm)) >= t0 && (uint8_t)token <= t9) {
                        // Add the direct key to the operand
                        outputCurr->operand = tGetKey + ((tok * 10 + (uint8_t)token - t0) << 8);
                        if ((uint8_t)(token = _getc(ice.inPrgm)) == tStore || (uint8_t)token == tEnter) {
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
            outputElements++;
            mask = TYPE_MASK_U24;
            
            // Get the string until it hits EOF, Enter, " or ->
            while ((int)(token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tString && (uint8_t)token != tStore && (uint8_t)token != tEnter) {
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
            outputCurr->operand = ice.OSStrings[_getc(ice.inPrgm)];
            outputElements++;
            mask = TYPE_MASK_U24;
        }
        
        // Oops, unknown token...
        else {
            return E_UNIMPLEMENTED;
        }
        
        // Yay, fetch the next token, it's great, it's true, I like it
        token = _getc(ice.inPrgm);
    }
    
    // If the expression quits normally, rather than an argument seperator
    ice.tempToken = tEnter;
    
stopParsing:
    // Move entire stack to output
    stackToOutputReturn = 2;
    goto stackToOutput;
stackToOutputReturn2:

    // Remove stupid things like 2+5, and not(1, max(2,3
    for (loopIndex = 1; loopIndex < outputElements; loopIndex++) {
        outputPrevPrev = &outputPtr[loopIndex-2];
        outputPrev = &outputPtr[loopIndex-1];
        outputCurr = &outputPtr[loopIndex];
        index = (uint8_t)(outputCurr->operand >> 8);
        
        // Check if the types are number | number | operator
        if (loopIndex > 1 && outputPrevPrev->type == TYPE_NUMBER && outputPrev->type == TYPE_NUMBER && 
               outputCurr->type == TYPE_OPERATOR && (uint8_t)outputCurr->operand != tStore) {
            // If yes, execute the operator, and store it in the first entry, and remove the other 2
            outputPrevPrev->operand = executeOperator(outputPrevPrev->operand, outputPrev->operand, (uint8_t)outputCurr->operand);
            memcpy(outputPrev, &outputPtr[loopIndex+1], (outputElements-1)*4);
            outputElements -= 2;
            loopIndex--;
            continue;
        }
        
        // Check if the types are number | number | ... | function (not det)
        if (loopIndex >= index && outputCurr->type == TYPE_FUNCTION && (uint8_t)outputCurr->operand != tDet && (uint8_t)outputCurr->operand != tLBrace) {
            uint24_t outputPrevOperand = outputPrev->operand, outputPrevPrevOperand = outputPrevPrev->operand;
            for (a = 1; a <= index; a++) {
                if ((&outputPtr[loopIndex-a])->type != TYPE_NUMBER) {
                    goto DontDeleteFunction;
                }
            }
            // The function has only numbers as argument, so remove them as well :)
            switch (outputCurr->operand) {
                case tNot:
                    temp = !outputPrevOperand;
                    break;
                case tMin:
                    temp = (outputPrevOperand < outputPrevPrevOperand) ? outputPrevOperand : outputPrevPrevOperand;
                    break;
                case tMax:
                    temp = (outputPrevOperand > outputPrevPrevOperand) ? outputPrevOperand : outputPrevPrevOperand;
                    break;
                case tMean:
                    temp = (outputPrevOperand + outputPrevPrevOperand) / 2;
                    break;
                case tSqrt:
                    temp = sqrt(outputPrevOperand);
                    break;
                case tExtTok:
                    if ((uint8_t)(outputPrevOperand >> 16) != tRemainder) {
                        return E_ICE_ERROR;
                    }
                    temp = outputPrevOperand % outputPrevPrevOperand;
                    break;
                default:
                    return E_ICE_ERROR;
            }
            
            // And remove everything
            (&outputPtr[loopIndex - index])->operand = temp;
            memcpy(&outputPtr[loopIndex - index + 1], &outputPtr[loopIndex+1], (outputElements - 1) * 4);
            outputElements -= index;
            loopIndex -= index - 1;
DontDeleteFunction:;
        }
    }
    
    // Check if the expression is valid
    if (!outputElements) {
        return E_SYNTAX;
    }
    
    return parsePostFixFromIndexToIndex(0, outputElements - 1);

    // Duplicated function opt
stackToOutput:
    // Move entire stack to output
    while (stackElements) {
        outputCurr = &outputPtr[outputElements++];
        stackPrev = &stackPtr[--stackElements];
        
        // Don't move the left paren...
        if (stackPrev->type == TYPE_FUNCTION && (uint8_t)stackPrev->operand == tLParen) {
            outputElements--;
            continue;
        }
        
        outputCurr->type = stackPrev->type;
        outputCurr->mask = stackPrev->mask;
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
    uint8_t outputType, temp, operandDepth = 0, AnsDepth = 0;
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
            
            if (AnsDepth > 3) {
                // We need to push HL since it isn't used in the next operator/function
                (&outputPtr[tempIndex])->type = TYPE_CHAIN_PUSH;
                PushHLDE();
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
                AnsDepth = 1;
            }
            tempIndex = loopIndex;
            outputCurr->type = TYPE_CHAIN_ANS;
        }
        
        else if (outputType == TYPE_FUNCTION) {
            // Use this to cleanup the function after parsing
            uint8_t amountOfArguments = (uint8_t)(outputCurr->operand >> 8);
            
            if (AnsDepth > 1+amountOfArguments) {
                // We need to push HL since it isn't used in the next operator/function
                (&outputPtr[tempIndex])->type = TYPE_CHAIN_PUSH;
                PushHLDE();
            }
            
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
            AnsDepth = 1;
            tempIndex = loopIndex;
            outputCurr->type = TYPE_CHAIN_ANS;
        }
        
        if (AnsDepth) {
            AnsDepth++;
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

static uint8_t functionI(uint24_t token) {
    const uint8_t colorTable[16] = {255,24,224,0,248,36,227,97,9,19,230,255,181,107,106,74};    // Thanks Cesium :D

    // Only get the output name, icon or description at the top of your program
    if (!ice.usedCodeAfterHeader) {
        uint24_t offset;
        
        // Get the output name
        if (!ice.gotName) {
            uint8_t a = 0;
            while ((int)(token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tEnter && a < 9) {
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
            if ((uint8_t)_getc(ice.inPrgm) != tString) {
                return E_WRONG_ICON;
            }

            // Get hexadecimal
            do {
                uint8_t tok;
                if ((tok = IsHexadecimal(_getc(ice.inPrgm))) == 16) {
                    return E_INVALID_HEX;
                }
                *ice.programPtr++ = colorTable[tok];
            } while (++b);
            
            // Move on to the description
            if ((uint8_t)(token = _getc(ice.inPrgm)) == tString) {
                token = _getc(ice.inPrgm);
            }

            if ((int)token != EOF) {
                if ((uint8_t)token != tEnter) {
                    return E_SYNTAX;
                }
                
                // Check if there is a description
                if ((uint8_t)(token = _getc(ice.inPrgm)) == tii) {
#ifndef COMPUTER_ICE
                    uint8_t *dataPtr = ti_GetDataPtr(ice.inPrgm);
                    
                    // Grab description
                    while ((int)(token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tEnter) {
                        uint24_t strLength;
                        const char *dataString;
                        uint8_t tokSize;
                        
                        // Get the token in characters, and copy to the output
                        dataString = ti_GetTokenString(&dataPtr, &tokSize, &strLength);
                        memcpy(ice.programPtr, dataString, strLength);
                        ice.programPtr += strLength;
                        
                        // If it's a 2-byte token, we also need to get the second byte of it
                        if (tokSize == 2) {
                            _getc(ice.inPrgm);
                        }
                    }
#else
                    while ((int)(token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tEnter) {
                        const uint8_t token2byte[] = { 0x5C, 0x5D, 0x5E, 0x60, 0x61, 0x62, 0x63, 0x7E, 0xBB, 0xAA, 0xEF };
                        *ice.programPtr++ = (uint8_t)token;
                        if (memchr(token2byte, token, sizeof token2byte)) {
                            _getc(ice.inPrgm);
                        }
                    }
#endif
                } else if ((int)token != EOF) {
                    _seek(-1, SEEK_CUR, ice.inPrgm);
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
    skipLine();

    return VALID;
}

static uint8_t functionIf(uint24_t token) {
    uint8_t res, tempZR, tempC, tempCR, insertJRCZReturn;
    uint8_t *IfStartAddr, *IfElseAddr = NULL;
    uint24_t tempDataOffsetElements, tempDataOffsetElements2;
    
    if ((int)(token = _getc(ice.inPrgm)) != EOF && token != tEnter) {
        // Parse the argument
        if ((res = parseExpression(token)) != VALID) {
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
        res = parseProgram();
        
        // Needs to be the end of a line
        if (!CheckEOL()) {
            return E_SYNTAX;
        }
        
        // Check if we quit the program with an 'Else'
        if (res == E_ELSE) {
            // Backup stuff
            IfElseAddr = ice.programPtr;
            tempDataOffsetElements2 = ice.dataOffsetElements;
            
            JP(0);
            res = parseProgram();
            if (res != E_END && res != VALID) {
                return res;
            }
            
            // Needs to be the end of a line
            if (!CheckEOL()) {
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

static uint8_t functionElse(uint24_t token) {
    return E_ELSE;
}

static uint8_t functionEnd(uint24_t token) {
    return E_END;
}

static uint8_t dummyReturn(uint24_t token) {
    return VALID;
}

uint8_t *WhileRepeatCondStart = NULL;

void UpdatePointersToData(uint24_t tempDataOffsetElements) {
    while (ice.dataOffsetElements != tempDataOffsetElements) {
        ice.dataOffsetStack[tempDataOffsetElements] = (uint24_t*)(((uint8_t*)ice.dataOffsetStack[tempDataOffsetElements]) - 2);
        tempDataOffsetElements++;
    }
}

static uint8_t functionWhile(uint24_t token) {
    uint24_t tempDataOffsetElements = ice.dataOffsetElements;
    uint8_t *WhileStartAddr = ice.programPtr, *TempStartAddr = WhileStartAddr, res;
    
    // Basically the same as "Repeat", but jump to condition checking first
    JP(0);
    if ((res = functionRepeat(token)) != VALID) {
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

uint8_t functionRepeat(uint24_t token) {
    uint24_t tempCurrentLine, tempCurrentLine2;
    uint16_t RepeatCondStart, RepeatProgEnd;
    uint8_t *RepeatCodeStart, *RepeatCondEnd, res;
    
    RepeatCondStart = _tell(ice.inPrgm);
    RepeatCodeStart = ice.programPtr;
    tempCurrentLine = ice.currentLine;
    
    // Skip the condition for now
    skipLine();
    
    // Parse the code
    if ((res = parseProgram()) != E_END && res != VALID) {
        return res;
    }
    
    // Needs to be the end of a line
    if (!CheckEOL()) {
        return E_SYNTAX;
    }

    // Remind where the "End" is
    RepeatProgEnd = _tell(ice.inPrgm);
    WhileRepeatCondStart = ice.programPtr;
    
    // Parse the condition
    _seek(RepeatCondStart, SEEK_SET, ice.inPrgm);
    tempCurrentLine2 = ice.currentLine;
    ice.currentLine = tempCurrentLine;
    if ((res = parseExpression(_getc(ice.inPrgm))) != VALID) {
        return res;
    }
    ice.currentLine = tempCurrentLine2;
    
    if (expr.outputIsString) {
        return E_SYNTAX;
    }
    
    // And set the pointer after the "End"
    _seek(RepeatProgEnd, SEEK_SET, ice.inPrgm);
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

static uint8_t functionReturn(uint24_t token) {
    uint8_t res;
    
    if ((int)(token = _getc(ice.inPrgm)) == EOF || (uint8_t)token == tEnter) {
        RET();
        ice.lastTokenIsReturn = true;
    } else if (token == tIf) {
        if ((res = parseExpression(_getc(ice.inPrgm))) != VALID) {
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

static uint8_t functionDisp(uint24_t token) {
    do {
        uint8_t res;

        if ((uint8_t)(token = _getc(ice.inPrgm)) == tii) {
            if ((int)(token = _getc(ice.inPrgm)) == EOF) {
                ice.tempToken = tEnter;
            } else {
                ice.tempToken = (uint8_t)token;
            }
            CALL(_NewLine);
            goto checkArgument;
        }
        
        // Get the argument, and display it, based on whether it's a string or the outcome of an expression
        expr.inFunction = true;
        if ((res = parseExpression(token)) != VALID) {
            return res;
        }
        if (expr.outputIsString) {
            CALL(_PutS);
        } else {
            MaybeDEToHL();
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

static uint8_t functionOutput(uint24_t token) {
    uint8_t res;
    
    // Get the first argument = column
    expr.inFunction = true;
    if ((res = parseExpression(_getc(ice.inPrgm))) != VALID) {
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
        if ((res = parseExpression(_getc(ice.inPrgm))) != VALID) {
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
        if ((res = parseExpression(_getc(ice.inPrgm))) != VALID) {
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
        if ((res = parseExpression(_getc(ice.inPrgm))) != VALID) {
            return res;
        }
        
        // Call the right function to display it
        if (expr.outputIsString) {
            CALL(_PutS);
        } else {
            MaybeDEToHL();
            CALL(_DispHL);
        }
    } else if (ice.tempToken != tEnter) {
        return E_SYNTAX;
    }
    
    return VALID;
}

static uint8_t functionClrHome(uint24_t token) {
    if (!CheckEOL()) {
        return E_SYNTAX;
    }
    if (ice.modifiedIY) {
        LD_IY_IMM(flags);
    }
    CALL(_HomeUp);
    CALL(_ClrLCDFull);
    ice.modifiedIY = false;
    return VALID;
}

static uint8_t functionFor(uint24_t token) {
    return E_UNIMPLEMENTED;
}

static uint8_t functionPrgm(uint24_t token) {
    return E_UNIMPLEMENTED;
}

static uint8_t functionCustom(uint24_t token) {
    uint8_t tok = (uint8_t)(token = _getc(ice.inPrgm));
    
    // CompilePrgm(
    if ((uint8_t)token == 0x0D) {
        char tempName[9];
        uint8_t a = 0, res;
        ti_var_t tempProg = ice.inPrgm;
        
#ifdef COMPUTER_ICE
        return E_NO_SUBPROG;
#endif
        
        while ((int)(token = _getc(ice.inPrgm)) != EOF && (tok = (uint8_t)token) != tEnter && a < 9) {
            tempName[a++] = (char)tok;
        }
        tempName[a] = 0;
        
        if ((ice.inPrgm = _open(tempName))) {
#ifndef COMPUTER_ICE
            char buf[30];
            displayLoadingBarFrame();
            sprintf(buf, "Compiling program %s...", tempName);
            gfx_PrintStringXY(buf, 1, iceMessageLine);
            
            // Compile it, and close
            res = parseProgram();
            displayLoadingBarFrame();
            ti_Close(ice.inPrgm);
#endif
        } else {
            return E_PROG_NOT_FOUND;
        }
        ice.inPrgm = tempProg;
        return res;
    }
    
    // Call
    else if ((uint8_t)token == 0x0C) {
        // Add the goto to the stack, and skip the line
        *ice.GotoPtr++ = (uint24_t)ice.programPtr;
        *ice.GotoPtr++ = _tell(ice.inPrgm);
        CALL(0);
        skipLine();
        return VALID;
    } else {
        return E_UNIMPLEMENTED;
    }
}

static uint8_t functionLbl(uint24_t token) {
    // Add the label to the stack, and skip the line
    *ice.LblPtr++ = (uint24_t)ice.programPtr;
    *ice.LblPtr++ = _tell(ice.inPrgm);
    skipLine();
    return VALID;
}

static uint8_t functionGoto(uint24_t token) {
    // Add the goto to the stack, and skip the line
    *ice.GotoPtr++ = (uint24_t)ice.programPtr;
    *ice.GotoPtr++ = _tell(ice.inPrgm);
    JP(0);
    skipLine();
    return VALID;
}

static uint8_t functionPause(uint24_t token) {
    if (CheckEOL()) {
        CALL(_GetCSC);
        CP_A(9);
        JR_NZ(-8);
    } else {
        uint8_t res;
        
        _seek(-1, SEEK_CUR, ice.inPrgm);
        if ((res = parseExpression(_getc(ice.inPrgm))) != VALID) {
            return res;
        }
        
        MaybeDEToHL();
        
        // Store the pointer to the call to the stack, to replace later
        ProgramPtrToOffsetStack();
        
        // We need to add the rand routine to the data section
        if (!ice.usedAlreadyPause) {
            ice.PauseAddr = (uintptr_t)ice.programDataPtr;
            memcpy(ice.programDataPtr, PauseData, 20);
            ice.programDataPtr += 20;
            ice.usedAlreadyPause = true;
        }
        
        CALL(ice.PauseAddr);
    }
    return VALID;
}

static uint8_t functionInput(uint24_t token) {
    return E_UNIMPLEMENTED;
}

static uint8_t functionBB(uint24_t token) {
    // Asm(
    if ((uint8_t)(token = _getc(ice.inPrgm)) == tAsm) {
        while ((int)(token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tEnter && (uint8_t)token != tRParen) {
            uint8_t tok1, tok2;
            
            // Get hexadecimal 1
            if ((tok1 = IsHexadecimal(token)) == 16) {
                return E_INVALID_HEX;
            }
            
            // Get hexadecimal 2
            if ((tok2 = IsHexadecimal(_getc(ice.inPrgm))) == 16) {
                return E_INVALID_HEX;
            }
            
            *ice.programPtr++ = (tok1 << 4) + tok2;
        }
        if ((uint8_t)token == tRParen) {
            if ((uint8_t)_getc(ice.inPrgm) != tEnter) {
                return E_SYNTAX;
            }
        }
    } else {
        _seek(-2, SEEK_CUR, ice.inPrgm);
        return parseExpression(_getc(ice.inPrgm));
    }
    return VALID;
}

static uint8_t tokenWrongPlace(uint24_t token) {
    return E_WRONG_PLACE;
}

static uint8_t tokenUnimplemented(uint24_t token) {
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

void skipLine(void) {
    uint24_t token;
    
    while ((int)(token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tEnter);
}

uint8_t (*functions[256])(uint24_t) = {
    tokenUnimplemented, //0
    tokenUnimplemented, //1
    tokenUnimplemented, //2
    tokenUnimplemented, //3
    tokenWrongPlace,    //4
    tokenUnimplemented, //5
    tokenWrongPlace,    //6
    parseExpression,    //7
    tokenWrongPlace,    //8
    parseExpression,    //9
    tokenUnimplemented, //10
    parseExpression,    //11
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
    parseExpression,    //59
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
    parseExpression,    //176
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
    parseExpression,    //240
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

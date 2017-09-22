#include "defines.h"
#include "parse.h"

#include "operator.h"
#include "main.h"
#include "functions.h"
#include "errors.h"
#include "stack.h"
#include "output.h"
#include "routines.h"

#ifdef COMPUTER_ICE
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(Pause, "src/asm/pause.bin");
INCBIN(Input, "src/asm/input.bin");
#endif

extern uint8_t (*functions[256])(int token);
const char implementedFunctions[] = {tNot, tMin, tMax, tMean, tSqrt, tDet, tSum, tSin, tCos};
const uint8_t implementedFunctions2[12] = {tExtTok, tRemainder,
                                           t2ByteTok, tSubStrng,
                                           t2ByteTok, tLength,
                                           tVarOut, tDefineSprite,
                                           tVarOut, tData,
                                           tVarOut, tCopy
                                          };
element_t outputStack[400];
element_t stack[200];
label_t labelStack[100];
label_t gotoStack[50];
variable_t variableStack[85];


uint8_t parseProgram(void) {
    int token;
    uint8_t ret = VALID;

    // Do things based on the token
    while ((token = _getc(ice.inPrgm)) != EOF) {
        if ((uint8_t)token != tii) {
            ice.usedCodeAfterHeader = true;
        }
        
        ice.lastTokenIsReturn = false;
        ice.currentLine++;

        if ((ret = (*functions[token])(token)) != VALID) {
            break;
        }
    }

    return ret;
}

/* Static functions */

uint8_t parseExpression(int token) {
    uint24_t stackElements = 0, outputElements = 0;
    uint24_t loopIndex, temp;
    uint8_t amountOfArgumentsStack[20];
    uint8_t index = 0, a, stackToOutputReturn, mask = TYPE_MASK_U24, tok, storeDepth = 0;
    uint8_t *amountOfArgumentsStackPtr = amountOfArgumentsStack, canUseMask = 2;

    // Setup pointers
    element_t *outputPtr = outputStack;
    element_t *stackPtr  = stack;
    element_t *outputCurr, *outputPrev, *outputPrevPrev;
    element_t *stackCurr, *stackPrev = NULL;
    
    /*
        General explanation output stack and normal stack:
        - Each entry consists of 5 bytes, the type (1), the mask (1) and the operand (3)
        - Type: number, variable, function, operator etc
        - Mask: 0 = 8 bits, 1 = 16 bits, 2 = 24 bits, only used for pointers
        - The operand is either a 3-byte number or consists of these 3 bytes:
            - The first byte = the operand: function/variable/operator token
            - If it's a function then the second byte is the amount of arguments for that function
            - If it's a getKeyFast, the second byte is the key
            - If it's a 2-byte function, the third byte is the second byte of the function
            - If it's a pointer directly after the -> operator, the third byte is 1 to ignore the function
    */

    while (token != EOF && (tok = (uint8_t)token) != tEnter) {
        bool IsA2ByteToken = (tok == t2ByteTok || tok == tExtTok || tok == tVarOut);
        
        outputCurr = &outputPtr[outputElements];
        stackCurr  = &stackPtr[stackElements];
        
        // We can use the unsigned mask * only at the start of the line, or directly after an operator
        if (canUseMask) {
            canUseMask--;
        }
        
        // If there's a pointer directly after an -> operator, we have to ignore it
        if (storeDepth) {
            storeDepth--;
        }

        // Process a number
        if (tok >= t0 && tok <= t9) {
            uint24_t output = token - t0;
            
            while ((uint8_t)(token = _getc(ice.inPrgm)) >= t0 && (uint8_t)token <= t9) {
                output = output * 10 + token - t0;
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
        
        // Process a binary number
        else if (tok == tPi) {
            uint24_t output = 0;
            
            while ((tok = (token = _getc(ice.inPrgm))) >= t0 && tok <= t1) {
                output = (output << 1) + tok - t0;
            }
            outputCurr->type = TYPE_NUMBER;
            outputCurr->operand = output;
            outputElements++;
            mask = TYPE_MASK_U24;

            // Don't grab a new token
            continue;
        }
        
        // Process a 'negative' number or expression
        else if (tok == tChs) {
            if ((token = _getc(ice.inPrgm)) >= t0 && token <= t9) {
                uint24_t output = token - t0;
                
                while ((uint8_t)(token = _getc(ice.inPrgm)) >= t0 && (uint8_t)token <= t9) {
                    output = output * 10 + token - t0;
                }
                outputCurr->type = TYPE_NUMBER;
                outputCurr->operand = 0-output;
                outputElements++;
                mask = TYPE_MASK_U24;
                
                // Don't grab a new token
                continue;
            } else {
                // Pretend as if it's a -1*
                outputCurr->type = TYPE_NUMBER;
                outputCurr->operand = -1;
                outputElements++;
                _seek(-1, SEEK_CUR, ice.inPrgm);
                token = tMul;
                tok = tMul;
                goto tokenIsOperator;
            }
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
            outputCurr->operand = GetVariableOffset(tok);
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
            
            // If the previous token was a ->, remind it, if not, this won't hurt
            storeDepth++;
            
            // Don't grab the { token
            continue;
        }
        
        // Parse an operator
        else if ((index = getIndexOfOperator(tok))) {
            // If the token is ->, move the entire stack to the output, instead of checking the precedence
            if (tok == tStore) {
                storeDepth = 2;
                // Move entire stack to output
                stackToOutputReturn = 1;
                goto stackToOutput;
            }
tokenIsOperator:

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
        
        // Gets the address of a variable
        else if (tok == tFromDeg) {
            outputCurr->type = TYPE_NUMBER;
            outputElements++;
            mask = TYPE_MASK_U24;
            tok = _getc(ice.inPrgm);
            
            // Get the address of the variable
            if (tok >= tA && tok <= tTheta) {
                char offset = GetVariableOffset(tok);
                
                outputCurr->operand = 0xD13F47 + offset;
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
                    outputCurr->mask = stackPrev->mask;
                    outputCurr->operand = stackPrev->operand;
                    stackElements--;
                    outputElements++;
                } else {
                    break;
                }
            }
            
            stackPrev = &stackPtr[stackElements-1];
            
            // Closing tag should match it's open tag
            if (((tok == tRBrace || tok == tRBrack) && ((uint8_t)stackPrev->operand != token - 1)) ||
                 (tok == tRParen && (stackPrev->operand == tLBrace || stackPrev->operand == tLBrack))) {
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
            if (tok == tComma && ((uint8_t)stackPrev->operand == tDet || (uint8_t)stackPrev->operand == tSum)) {
                outputCurr->type = TYPE_ARG_DELIMITER;
                outputElements++;
            }
            
            // If the right parenthesis belongs to a function, move the function as well
            if (tok != tComma && stackPrev->operand != tLParen) {
                outputCurr->type = stackPrev->type;
                outputCurr->mask = stackPrev->mask;
                outputCurr->operand = stackPrev->operand + ((*amountOfArgumentsStackPtr--) << 8);
                stackElements--;
                outputElements++;
            }
            
            // Increment the amount of arguments for that function
            if (tok == tComma) {
                (*amountOfArgumentsStackPtr)++;
                canUseMask = 2;
            }
            
            mask = TYPE_MASK_U24;
        }
        
        // Process a function, ( { [
        else if (strchr(implementedFunctions, tok) || IsA2ByteToken || tok == tLParen || tok == tLBrace || tok == tLBrack) {
            if (IsA2ByteToken) {
                uint8_t temp2 = _getc(ice.inPrgm);
                uint24_t temp3;
                
                for (temp3 = 0; temp3 < sizeof(implementedFunctions2); temp3 += 2) {
                    if (tok == implementedFunctions2[temp3] && temp2 == implementedFunctions2[temp3 + 1]) {
                        goto foundRight2ByteToken;
                    }
                }
                return E_UNIMPLEMENTED;
foundRight2ByteToken:
                token = token + (temp2 << 16);
            }
            // We always have at least 1 argument
            *++amountOfArgumentsStackPtr = 1;
            stackCurr->type = TYPE_FUNCTION;
            stackCurr->mask = mask;
            stackCurr->operand = token + ((tok == tLBrace && storeDepth) << 16);
            stackElements++;
            mask = TYPE_MASK_U24;
            canUseMask = 2;
            
            // Check if it's a C function
            if (tok == tDet || tok == tSum) {
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
                    uint24_t temp2 = token - t0;
                    
                    // The next token can be a number, but also right parenthesis or EOF
                    if ((uint8_t)(token = _getc(ice.inPrgm)) >= t0 && (uint8_t)token <= t9) {
                        // Add the direct key to the operand
                        outputCurr->operand = tGetKey + ((temp2 * 10 + token - t0) << 8);
                        if ((uint8_t)(token = _getc(ice.inPrgm)) == tStore || (uint8_t)token == tEnter) {
                            // Don't grab new token
                            continue;
                        } else if (token != EOF && (uint8_t)token != tRParen) {
                            return E_SYNTAX;
                        }
                    } else if ((uint8_t)token == tRParen || token == EOF || (uint8_t)token == tStore || (uint8_t)token == tEnter) {
                        // Add the direct key to the operand
                        outputCurr->operand = tGetKey + (temp2 << 8);
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
            while ((token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tString && (uint8_t)token != tStore && (uint8_t)token != tEnter) {
                *ice.programDataPtr++ = token;
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
        outputPrevPrev = &outputPtr[loopIndex - 2];
        outputPrev = &outputPtr[loopIndex - 1];
        outputCurr = &outputPtr[loopIndex];
        index = outputCurr->operand >> 8;
        
        // Check if the types are number | number | operator
        if (loopIndex > 1 && outputPrevPrev->type == TYPE_NUMBER && outputPrev->type == TYPE_NUMBER && 
               outputCurr->type == TYPE_OPERATOR && (uint8_t)outputCurr->operand != tStore) {
            // If yes, execute the operator, and store it in the first entry, and remove the other 2
            outputPrevPrev->operand = executeOperator(outputPrevPrev->operand, outputPrev->operand, (uint8_t)outputCurr->operand);
            memcpy(outputPrev, &outputPtr[loopIndex + 1], (outputElements - 1) * sizeof(element_t));
            outputElements -= 2;
            loopIndex--;
            continue;
        }
        
        // Check if the types are number | number | ... | function (no det or pointer)
        if (loopIndex >= index && outputCurr->type == TYPE_FUNCTION && 
                (uint8_t)outputCurr->operand != tDet &&
                (uint8_t)outputCurr->operand != tLBrace &&
                (uint8_t)outputCurr->operand != tSum &&
                (uint8_t)outputCurr->operand != tVarOut) {
            uint24_t outputPrevOperand = outputPrev->operand, outputPrevPrevOperand = outputPrevPrev->operand;
            
            for (a = 1; a <= index; a++) {
                if ((&outputPtr[loopIndex-a])->type != TYPE_NUMBER) {
                    goto DontDeleteFunction;
                }
            }
            // The function has only numbers as argument, so remove them as well :)
            switch ((uint8_t)outputCurr->operand) {
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
                    temp = ((long)outputPrevOperand + (long)outputPrevPrevOperand) / 2;
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
                case tSin:
                    temp = 255*sin((double)outputPrevOperand / 256 * 2 * M_PI);
                    break;
                case tCos:
                    temp = 255*cos((double)outputPrevOperand / 256 * 2 * M_PI);
                    break;
                default:
                    return E_ICE_ERROR;
            }
            
            // And remove everything
            (&outputPtr[loopIndex - index])->operand = temp;
            memcpy(&outputPtr[loopIndex - index + 1], &outputPtr[loopIndex + 1], (outputElements - 1) * sizeof(element_t));
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
    uint24_t outputOperand, loopIndex, tempIndex = 0, amountOfStackElements;
    
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
        
        // If it's the start of a det( or sum(, increment the amount of nested det(/sum(
        if (outputCurr->type == TYPE_C_START) {
            temp++;
        }
        // If it's a det( or sum(, decrement the amount of nested dets
        if (outputCurr->type == TYPE_FUNCTION && ((uint8_t)outputCurr->operand == tDet || (uint8_t)outputCurr->operand == tSum)) {
            temp--;
        }
        
        // If not in a nested det( or sum(, push the index
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
            insertFunctionReturnNoPush(outputOperand, OUTPUT_IN_HL);
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
        element_t *outputPrevPrevPrev;
        
        outputCurr = &outputPtr[loopIndex = getNextIndex()];
        outputPrevPrevPrev = &outputPtr[getIndexOffset(-4)];
        outputType = outputCurr->type;
        
        // Clear this flag
        expr.AnsSetZeroFlagReversed = false;
        
        if (outputType == TYPE_OPERATOR) {
            element_t *outputPrev, *outputPrevPrev;
            
            // Wait, invalid operator?!
            if (loopIndex < startIndex + 2) {
                return E_SYNTAX;
            }
            
            if (AnsDepth > 3 && (uint8_t)outputCurr->operand != tStore) {
                // We need to push HL since it isn't used in the next operator/function
                (&outputPtr[tempIndex])->type = TYPE_CHAIN_PUSH;
                PushHLDE();
                expr.outputRegister = OUTPUT_IN_HL;
            }
            
            // Get the previous entries, -2 is the previous one, -3 is the one before etc
            outputPrev = &outputPtr[getIndexOffset(-2)];
            outputPrevPrev = &outputPtr[getIndexOffset(-3)];
            
            // Parse the operator with the 2 latest operands of the stack!
            if ((temp = parseOperator(outputPrevPrevPrev, outputPrevPrev, outputPrev, outputCurr)) != VALID) {
                return temp;
            }
            
            // Remove the index of the first and the second argument, the index of the operator will be the chain
            removeIndexFromStack(getCurrentIndex() - 2);
            removeIndexFromStack(getCurrentIndex() - 2);
            
            // Check if it was a command with 2 strings, then the output is a string, not Ans
            if ((uint8_t)outputCurr->operand == tAdd && outputPrevPrev->type >= TYPE_STRING && outputPrev->type >= TYPE_STRING) {
                outputCurr->type = TYPE_STRING;
                if (outputPrevPrev->operand == ice.tempStrings[TempString2] || outputPrev->operand == ice.tempStrings[TempString1]) {
                    outputCurr->operand = ice.tempStrings[TempString2];
                } else {
                    outputCurr->operand = ice.tempStrings[TempString1];
                }
            } else {
                AnsDepth = 1;
                outputCurr->type = TYPE_CHAIN_ANS;
            }
            tempIndex = loopIndex;
        }
        
        else if (outputType == TYPE_FUNCTION) {
            // Use this to cleanup the function after parsing
            uint8_t amountOfArguments = outputCurr->operand >> 8;
            uint8_t function2 = outputCurr->operand >> 16;
            
            // Only execute when it's not a pointer directly after a ->
            if (outputCurr->operand != 0x010108) {
                // Check if we need to push Ans
                if (AnsDepth > 1 + amountOfArguments) {
                    // We need to push HL since it isn't used in the next operator/function
                    (&outputPtr[tempIndex])->type = TYPE_CHAIN_PUSH;
                    PushHLDE();
                    expr.outputRegister = OUTPUT_IN_HL;
                }
                
                if ((temp = parseFunction(loopIndex)) != VALID) {
                    return temp;
                }
                
                // Cleanup, if it's not a det(
                if ((uint8_t)outputCurr->operand != tDet && (uint8_t)outputCurr->operand != tSum) {
                    for (temp = 0; temp < amountOfArguments; temp++) {
                        removeIndexFromStack(getCurrentIndex() - 2);
                    }
                }
                
                // I don't care that this will be ignored when it's a pointer, because I know there is a -> directly after
                // If it's a sub(, the output should be a string, not Ans
                if ((uint8_t)outputCurr->operand == t2ByteTok && function2 == tSubStrng) {
                    outputCurr->type = TYPE_STRING;
                    if (outputPrevPrevPrev->operand == ice.tempStrings[TempString1]) {
                        outputCurr->operand = ice.tempStrings[TempString2];
                    } else {
                        outputCurr->operand = ice.tempStrings[TempString1];
                    }
                }
                
                // Check chain push/ans
                else {
                    AnsDepth = 1;
                    tempIndex = loopIndex;
                    outputCurr->type = TYPE_CHAIN_ANS;
                }
            }
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

static uint8_t functionI(int token) {
    const uint8_t colorTable[16] = {255,24,224,0,248,36,227,97,9,19,230,255,181,107,106,74};    // Thanks Cesium :D

    // Only get the output name, icon or description at the top of your program
    if (!ice.usedCodeAfterHeader) {
        uint24_t offset;
        
        // Get the output name
        if (!ice.gotName) {
            uint8_t a = 0;
            while ((token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tEnter && a < 9) {
                ice.outName[a++] = token;
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

            if (token != EOF) {
                if ((uint8_t)token != tEnter) {
                    return E_SYNTAX;
                }
                
                // Check if there is a description
                if ((uint8_t)(token = _getc(ice.inPrgm)) == tii) {
#ifndef COMPUTER_ICE
                    void *dataPtr = ti_GetDataPtr(ice.inPrgm);
                    
                    // Grab description
                    while ((token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tEnter) {
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
                    while ((token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tEnter) {
                        const uint8_t token2byte[] = { 0x5C, 0x5D, 0x5E, 0x60, 0x61, 0x62, 0x63, 0x7E, 0xBB, 0xAA, 0xEF };
                        *ice.programPtr++ = (uint8_t)token;
                        if (memchr(token2byte, token, sizeof token2byte)) {
                            _getc(ice.inPrgm);
                        }
                    }
#endif
                } else if (token != EOF) {
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

static uint8_t functionIf(int token) {
    uint8_t *IfStartAddr, *IfElseAddr = NULL, res;
    uint24_t tempDataOffsetElements, tempDataOffsetElements2;
    
    if ((token = _getc(ice.inPrgm)) != EOF && token != tEnter) {
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
        
        if (expr.AnsSetCarryFlag || expr.AnsSetCarryFlagReversed) {
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
        res = parseProgram();
        
        // Needs to be the end of a line
        if (!CheckEOL()) {
            return E_SYNTAX;
        }
        
        // Check if we quit the program with an 'Else'
        if (res == E_ELSE) {
            bool shortElseCode;
            
            // Backup stuff
            IfElseAddr = ice.programPtr;
            tempDataOffsetElements2 = ice.dataOffsetElements;
            
            JP(0);
            if ((res = parseProgram()) != E_END && res != VALID) {
                return res;
            }
            
            // Needs to be the end of a line
            if (!CheckEOL()) {
                return E_SYNTAX;
            }
            
            shortElseCode = JumpForward(IfElseAddr, ice.programPtr, tempDataOffsetElements2);
            JumpForward(IfStartAddr, IfElseAddr + (shortElseCode ? 2 : 4), tempDataOffsetElements);
        }
        
        // Check if we quit the program with an 'End' or at the end of the input program
        else if (res == E_END || res == VALID) {
            JumpForward(IfStartAddr, ice.programPtr, tempDataOffsetElements);
        } else {
            return res;
        }
        return VALID;
    } else {
        return E_NO_CONDITION;
    }
}

static uint8_t functionElse(int token) {
    return E_ELSE;
}

static uint8_t functionEnd(int token) {
    return E_END;
}

static uint8_t dummyReturn(int token) {
    return VALID;
}

uint8_t JumpForward(uint8_t *startAddr, uint8_t *endAddr, uint24_t tempDataOffsetElements) {
    if (endAddr - startAddr <= 0x80) {
        uint8_t *tempPtr = startAddr;
        uint8_t opcode = *startAddr;
        
        *startAddr++ = opcode - 0xA2 - (opcode == 0xC3 ? 9 : 0);
        *startAddr++ = endAddr - tempPtr - 4;
        
        // Update pointers to data, decrease them all with 2
        while (ice.dataOffsetElements != tempDataOffsetElements) {
            ice.dataOffsetStack[tempDataOffsetElements] = (uint24_t*)(((uint8_t*)ice.dataOffsetStack[tempDataOffsetElements]) - 2);
            tempDataOffsetElements++;
        }
        
        memcpy(startAddr, startAddr + 2, ice.programPtr - startAddr);
        ice.programPtr -= 2;
        return true;
    } else {
        w24(startAddr + 1, ice.programPtr - ice.programData + PRGM_START);
        return false;
    }
}

uint8_t JumpBackwards(uint8_t *startAddr, uint8_t whichOpcode) {
    if (ice.programPtr + 2 - startAddr <= 0x80) {
        uint8_t *tempPtr = ice.programPtr;
        
        *ice.programPtr++ = whichOpcode;
        *ice.programPtr++ = startAddr - 2 - tempPtr;
        return true;
    } else {
        // JR cc to JP cc
        *ice.programPtr++ = whichOpcode + 0xA2 + (whichOpcode == 0x18 ? 9 : 0);
        output(uint24_t, startAddr - ice.programData + PRGM_START);
        return false;
    }
}

uint8_t *WhileRepeatCondStart = NULL;

static uint8_t functionWhile(int token) {
    uint24_t tempDataOffsetElements = ice.dataOffsetElements;
    uint8_t *WhileStartAddr = ice.programPtr, *TempStartAddr = WhileStartAddr, res;
    
    // Basically the same as "Repeat", but jump to condition checking first
    JP(0);
    if ((res = functionRepeat(token)) != VALID) {
        return res;
    }
    
    // Check if we can optimize the JP(0) to JR
    JumpForward(WhileStartAddr, WhileRepeatCondStart, tempDataOffsetElements);

    return VALID;
}

uint8_t functionRepeat(int token) {
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
    
    JumpBackwards(RepeatCodeStart, expr.AnsSetCarryFlag || expr.AnsSetCarryFlagReversed ?
        (expr.AnsSetCarryFlagReversed ? OP_JR_NC : OP_JR_C) :
        (expr.AnsSetZeroFlagReversed  ? OP_JR_NZ : OP_JR_Z));
    return VALID;
}

static uint8_t functionReturn(int token) {
    uint8_t res;
    
    if ((token = _getc(ice.inPrgm)) == EOF || (uint8_t)token == tEnter) {
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
        
        *ice.programPtr++ = (expr.AnsSetCarryFlag || expr.AnsSetCarryFlagReversed ? 
            (expr.AnsSetCarryFlagReversed ? OP_RET_C : OP_RET_NC) :
            (expr.AnsSetZeroFlagReversed ? OP_RET_Z : OP_RET_NZ));
    }
    return VALID;
}

static uint8_t functionDisp(int token) {
    if (ice.modifiedIY) {
        LD_IY_IMM(flags);
        ice.modifiedIY = false;
    }
    do {
        uint8_t res;

        if ((uint8_t)(token = _getc(ice.inPrgm)) == tii) {
            if ((token = _getc(ice.inPrgm)) == EOF) {
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
            AnsToHL();
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

static uint8_t functionOutput(int token) {
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
            // Output coordinates in H and L
            ice.programPtr -= 10;
            LD_SIS_HL((*(ice.programPtr + 1) << 8) + *(ice.programPtr + 7));
            LD_SIS_IMM_HL(curRow & 0xFFFF);
        } else {
            if (expr.outputIsVariable) {
                *(ice.programPtr - 2) = OP_LD_A_HL;
            } else if (expr.outputRegister == OUTPUT_IN_HL) {
                LD_A_L();
            } else if (expr.outputRegister == OUTPUT_IN_DE) {
                LD_A_E();
            }
            LD_IMM_A(curRow);
        }
    } else {
        if (expr.outputIsVariable) {
            *(ice.programPtr - 2) = OP_LD_A_HL;
        } else if (expr.outputRegister == OUTPUT_IN_HL) {
            LD_A_L();
        } else if (expr.outputRegister == OUTPUT_IN_DE) {
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
            *(ice.programPtr - 2) = OP_LD_A_HL;
        } else if (expr.outputRegister == OUTPUT_IN_HL) {
            LD_A_L();
        } else if (expr.outputRegister == OUTPUT_IN_DE) {
            LD_A_E();
        }
        LD_IMM_A(curRow);
    }
    
    // Get the third argument = output thing
    if (ice.tempToken == tComma) {
        if (ice.modifiedIY) {
            LD_IY_IMM(flags);
            ice.modifiedIY = false;
        }
        if ((res = parseExpression(_getc(ice.inPrgm))) != VALID) {
            return res;
        }
        
        // Call the right function to display it
        if (expr.outputIsString) {
            CALL(_PutS);
        } else {
            AnsToHL();
            CALL(_DispHL);
        }
    } else if (ice.tempToken != tEnter) {
        return E_SYNTAX;
    }
    
    return VALID;
}

static uint8_t functionClrHome(int token) {
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

static uint8_t functionFor(int token) {
    bool endPointIsNumber = false, stepIsNumber = false, reversedCond = false, smallCode;
    uint24_t endPointNumber = 0, stepNumber = 0, tempDataOffsetElements;
    uint8_t *endPointExpressionValue = 0, *stepExpression = 0, *jumpToCond, *loopStart;
    uint8_t tok, variable, res;
    
    if ((tok = _getc(ice.inPrgm)) < tA || tok > tTheta) {
        return E_SYNTAX;
    }
    variable = GetVariableOffset(tok);
    expr.inFunction = true;
    if (_getc(ice.inPrgm) != tComma) {
        return E_SYNTAX;
    }
    
    // Get the start value, followed by a comma
    if ((res = parseExpression(_getc(ice.inPrgm))) != VALID) {
        return res;
    }
    if (ice.tempToken != tComma) {
        return E_SYNTAX;
    }
    
    // Load the value in the variable
    if (expr.outputRegister == OUTPUT_IN_HL) {
        LD_IX_OFF_IND_HL(variable);
    } else {
        LD_IX_OFF_IND_DE(variable);
    }
    
    // Get the end value
    expr.inFunction = true;
    if ((res = parseExpression(_getc(ice.inPrgm))) != VALID) {
        return res;
    }
    
    // If the end point is a number, we can optimize things :D
    if (expr.outputIsNumber) {
        endPointIsNumber = true;
        endPointNumber = expr.outputNumber;
        ice.programPtr -= 4 - !expr.outputNumber;
    } else {
        endPointExpressionValue = ice.programPtr;
        MaybeAToHL();
        if (expr.outputRegister == OUTPUT_IN_HL) {
            LD_ADDR_HL(0);
        } else {
            LD_ADDR_DE(0);
        }
    }
    
    // Check if there was a step
    if (ice.tempToken == tComma) {
        expr.inFunction = true;
        
        // Get the step value
        if ((res = parseExpression(_getc(ice.inPrgm))) != VALID) {
            return res;
        }
        if (ice.tempToken == tComma) {
            return E_SYNTAX;
        }
        
        if (expr.outputIsNumber) {
            stepIsNumber = true;
            stepNumber = expr.outputNumber;
            ice.programPtr -= 4 - !expr.outputNumber;
        } else {
            stepExpression = ice.programPtr;
            MaybeAToHL();
            if (expr.outputRegister == OUTPUT_IN_HL) {
                LD_ADDR_HL(0);
            } else {
                LD_ADDR_DE(0);
            }
        }
    } else {
        stepIsNumber = true;
        stepNumber = 1;
    }
    
    jumpToCond = ice.programPtr;
    JP(0);
    tempDataOffsetElements = ice.dataOffsetElements;
    loopStart = ice.programPtr;
    
    // Parse the inner loop
    if ((res = parseProgram()) != E_END && res != VALID) {
        return res;
    }
    
    // Needs to be the end of a line
    if (!CheckEOL()) {
        return E_SYNTAX;
    }
    
    // First add the step to the variable, if the step is 0 we don't need this
    if (!stepIsNumber || stepNumber) {
        // ld hl, (ix+*) \ inc hl/dec hl (x times) \ ld (ix+*), hl
        // ld hl, (ix+*) \ ld de, x \ add hl, de \ ld (ix+*), hl
        
        LD_HL_IND_IX_OFF(variable);
        if (stepIsNumber) {
            uint8_t a = 0;
            
            if (stepNumber < 5) {
                for (a = 0; a < (uint8_t)stepNumber; a++) {
                    INC_HL();
                }
            } else if (stepNumber > 0xFFFFFF - 4) {
                for (a = 0; a < (uint8_t)(0-stepNumber); a++) {
                    DEC_HL();
                }
            } else {
                LD_DE_IMM(stepNumber);
                ADD_HL_DE();
            }
        } else {
            w24(stepExpression + 1, ice.programPtr - ice.programData + PRGM_START + 1);
            LD_DE_IMM(0);
            ADD_HL_DE();
        }
        LD_IX_OFF_IND_HL(variable);
    }
    
    smallCode = JumpForward(jumpToCond, ice.programPtr, tempDataOffsetElements);
    
    // If both the step and the end point are a number, the variable is already in HL
    if (!(endPointIsNumber && stepIsNumber)) {
        LD_HL_IND_IX_OFF(variable);
    }

    if (endPointIsNumber) {
        if (stepNumber < 0x800000) {
            LD_DE_IMM(endPointNumber + 1);
        } else {
            LD_DE_IMM(endPointNumber);
            reversedCond = true;
        }
        OR_A_A();
    } else {
        w24(endPointExpressionValue + 1, ice.programPtr + PRGM_START - ice.programData + 1);
        LD_DE_IMM(0);
        if (stepNumber < 0x800000) {
            SCF();
        } else {
            OR_A_A();
            reversedCond = true;
        }
    }
    SBC_HL_DE();
    
    // Jump back to the loop
    JumpBackwards(loopStart - (smallCode ? 2 : 0), OP_JR_C - (reversedCond ? 8 : 0));
    
    return VALID;
}

static uint8_t functionPrgm(int token) {
    return E_UNIMPLEMENTED;
}

static uint8_t functionCustom(int token) {
    uint8_t tok = _getc(ice.inPrgm);
    
    // DefineSprite(, Data(, Copy(
    if (tok == tDefineSprite || tok == tData || tok == tCopy) {
        _seek(-1, SEEK_CUR, ice.inPrgm);
        return parseExpression(token);
    }
    
    // Call
    else if (tok == tCall) {
        insertGotoLabel();
        CALL(0);
        return VALID;
    } else {
        return E_UNIMPLEMENTED;
    }
}

static uint8_t functionLbl(int token) {
    // Add the label to the stack, and skip the line
    label_t *labelPtr = labelStack;
    label_t *labelCurr = &labelPtr[ice.amountOfLbls++];
    uint8_t a = 0;
    
    // Get the label name
    while ((token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tEnter) {
        labelCurr->name[a++] = token;
    }
    labelCurr->name[a] = 0;
    labelCurr->addr = (uint24_t)ice.programPtr;
    return VALID;
}

static uint8_t functionGoto(int token) {
    insertGotoLabel();
    JP(0);
    return VALID;
}

void insertGotoLabel(void) {
    // Add the label to the stack, and skip the line
    label_t *gotoPtr = gotoStack;
    label_t *gotoCurr = &gotoPtr[ice.amountOfGotos++];
    uint8_t a = 0;
    int token;
    
    while ((token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tEnter) {
        gotoCurr->name[a++] = token;
    }
    gotoCurr->name[a] = 0;
    gotoCurr->addr = (uint24_t)ice.programPtr;
}

static uint8_t functionPause(int token) {
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
        
        AnsToHL();
        
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

static uint8_t functionInput(int token) {
    uint8_t tok;
    
    if ((tok = _getc(ice.inPrgm)) < tA || tok > tTheta || !CheckEOL()) {
        return E_SYNTAX;
    }
    LD_A(GetVariableOffset(tok));
    
    // Copy the Input routine to the data section
    if (!ice.usedAlreadyInput) {
        ice.InputAddr = (uintptr_t)ice.programDataPtr;
        memcpy(ice.programDataPtr, InputData, 66);
        ice.programDataPtr += 66;
        ice.usedAlreadyInput = true;
    }
    
    // Set which var we need to store to
    ProgramPtrToOffsetStack();
    LD_ADDR_A(ice.InputAddr + 61);
    
    // Call the right routine
    ProgramPtrToOffsetStack();
    CALL(ice.InputAddr);
    
    return VALID;
}

static uint8_t functionBB(int token) {
    // Asm(
    if ((uint8_t)(token = _getc(ice.inPrgm)) == tAsm) {
        while ((token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tEnter && (uint8_t)token != tRParen) {
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
            if (!CheckEOL()) {
                return E_SYNTAX;
            }
        }
        return VALID;
    }
    
    // AsmComp(
    else if ((uint8_t)token == tAsmComp) {
        char tempName[9];
        uint8_t a = 0, res;
        ti_var_t tempProg = ice.inPrgm;
        
#ifdef COMPUTER_ICE
        return E_NO_SUBPROG;
#else
        while ((token = _getc(ice.inPrgm)) != EOF && (uint8_t)token != tEnter && a < 9) {
            tempName[a++] = token;
        }
        tempName[a] = 0;
        
        if ((ice.inPrgm = _open(tempName))) {
            char buf[30];
            
            displayLoadingBarFrame();
            sprintf(buf, "Compiling program %s...", tempName);
            gfx_PrintStringXY(buf, 1, iceMessageLine);
            
            // Compile it, and close
            res = parseProgram();
            displayLoadingBarFrame();
            ti_Close(ice.inPrgm);
            gfx_PrintStringXY("Return from subprogram...", 1, iceMessageLine);
        } else {
            return E_PROG_NOT_FOUND;
        }
        ice.inPrgm = tempProg;
        return res;
#endif
    } else {
        _seek(-1, SEEK_CUR, ice.inPrgm);
        return parseExpression(token);
    }
}

static uint8_t tokenWrongPlace(int token) {
    return E_WRONG_PLACE;
}

static uint8_t tokenUnimplemented(int token) {
    return E_UNIMPLEMENTED;
}

void optimizeZeroCarryFlagOutput(void) {
    if (!expr.AnsSetZeroFlag && !expr.AnsSetCarryFlag) {
        if (expr.outputRegister == OUTPUT_IN_HL) {
            ADD_HL_DE();
            OR_A_SBC_HL_DE();
        } else if (expr.outputRegister == OUTPUT_IN_DE) {
            SCF();
            SBC_HL_HL();
            ADD_HL_DE();
            expr.AnsSetCarryFlagReversed = true;
        } else {
            OR_A_A();
        }
    } else {
        ice.programPtr -= expr.ZeroCarryFlagRemoveAmountOfBytes;
    }
    return;
}

void skipLine(void) {
    while (!CheckEOL());
}

uint8_t (*functions[256])(int) = {
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
    parseExpression,    //130
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
    parseExpression,    //172
    parseExpression,    //173
    tokenUnimplemented, //174
    tokenUnimplemented, //175
    parseExpression,    //176
    tokenUnimplemented, //177
    tokenUnimplemented, //178
    parseExpression,    //179
    tokenUnimplemented, //180
    tokenUnimplemented, //181
    parseExpression,    //182
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
    parseExpression,    //194
    tokenUnimplemented, //195
    parseExpression,    //196
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

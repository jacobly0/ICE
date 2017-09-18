#include "defines.h"
#include "functions.h"

#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "main.h"
#include "output.h"
#include "operator.h"
#include "routines.h"

#ifdef COMPUTER_ICE
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(Sqrt, "src/asm/sqrt.bin");
INCBIN(Mean, "src/asm/mean.bin");
INCBIN(Sincos, "src/asm/sincos.bin");
#endif

/* First byte:  bit 7  : returns something in A
                bit 6  : unimplemented
                bit 5  : returns something in HL (16 bits)
                bit 4  : deprecated
                bit 2-0: amount of arguments needed
   Second byte: bit 7  : first argument is small
                bit 6  : second argument is small
                bit 5  : third argument is small
                ...
*/

const uint8_t GraphxArgs[] = {
    RET_NONE | 0, ARG_NORM,    // Begin
    RET_NONE | 0, ARG_NORM,    // End
    RET_A    | 1, SMALL_1,     // SetColor
    RET_NONE | 0, ARG_NORM,    // SetDefaultPalette
    UN       | 3, ARG_NORM,    // SetPalette
    RET_NONE | 1, SMALL_1,     // FillScreen
    RET_NONE | 2, SMALL_2,     // SetPixel
    RET_A    | 2, SMALL_2,     // GetPixel
    RET_A    | 0, ARG_NORM,    // GetDraw
    RET_NONE | 1, SMALL_1,     // SetDraw
    RET_NONE | 0, ARG_NORM,    // SwapDraw
    RET_NONE | 1, SMALL_1,     // Blit
    RET_NONE | 3, SMALL_123,   // BlitLines
    RET_NONE | 5, SMALL_13,    // BlitArea
    RET_NONE | 1, SMALL_1,     // PrintChar
    RET_NONE | 2, SMALL_2,     // PrintInt
    RET_NONE | 2, SMALL_2,     // PrintUInt
    RET_NONE | 1, ARG_NORM,    // PrintString
    RET_NONE | 3, ARG_NORM,    // PrintStringXY
    RET_NONE | 2, ARG_NORM,    // SetTextXY
    RET_A    | 1, SMALL_1,     // SetTextBGColor
    RET_A    | 1, SMALL_1,     // SetTextFGColor
    RET_A    | 1, SMALL_1,     // SetTextTransparentColor
    UN       | 0, ARG_NORM,    // SetCustomFontData
    UN       | 0, ARG_NORM,    // SetCustomFontSpacing
    RET_NONE | 1, SMALL_1,     // SetMonoSpaceFont
    RET_HL   | 1, ARG_NORM,    // GetStringWidth
    RET_HL   | 1, SMALL_1,     // GetCharWidth
    RET_HL   | 0, ARG_NORM,    // GetTextX
    RET_HL   | 0, ARG_NORM,    // GetTextY
    RET_NONE | 4, ARG_NORM,    // Line
    RET_NONE | 3, ARG_NORM,    // HorizLine
    RET_NONE | 3, ARG_NORM,    // VertLine
    RET_NONE | 3, ARG_NORM,    // Circle
    RET_NONE | 3, ARG_NORM,    // FillCircle
    RET_NONE | 4, ARG_NORM,    // Rectangle
    RET_NONE | 4, ARG_NORM,    // FillRectangle
    RET_NONE | 4, SMALL_14,    // Line_NoClip
    RET_NONE | 3, SMALL_2,     // HorizLine_NoClip
    RET_NONE | 3, SMALL_2,     // VertLine_NoClip
    RET_NONE | 3, SMALL_2,     // FillCircle_NoClip
    RET_NONE | 3, SMALL_14,    // Rectangle_NoClip
    RET_NONE | 4, SMALL_14,    // FillRectangle_NoClip
    RET_NONE | 4, ARG_NORM,    // SetClipRegion
    RET_A    | 1, ARG_NORM,    // GetClipRegion
    RET_NONE | 1, SMALL_1,     // ShiftDown
    RET_NONE | 1, SMALL_1,     // ShiftUp
    RET_NONE | 1, SMALL_1,     // ShiftLeft
    RET_NONE | 1, SMALL_1,     // ShiftRight
    UN       | 0, ARG_NORM,    // Tilemap
    UN       | 0, ARG_NORM,    // Tilemap_NoClip
    UN       | 0, ARG_NORM,    // TransparentTilemap
    UN       | 0, ARG_NORM,    // TransparentTilemap_NoClip
    UN       | 0, ARG_NORM,    // TilePtr
    UN       | 0, ARG_NORM,    // TilePtrMapped
    UN       | 0, ARG_NORM,    // LZDecompress
    UN       | 0, ARG_NORM,    // AllocSprite
    RET_NONE | 3, ARG_NORM,    // Sprite
    RET_NONE | 3, ARG_NORM,    // TransparentSprite
    RET_NONE | 3, SMALL_3,     // Sprite_NoClip
    RET_NONE | 3, SMALL_3,     // TransparentSprite_NoClip
    RET_HL   | 3, ARG_NORM,    // GetSprite
    RET_NONE | 5, SMALL_45,    // ScaledSprite_NoClip
    RET_NONE | 5, SMALL_45,    // ScaledTransparentSprite_NoClip
    RET_HL   | 2, ARG_NORM,    // FlipSpriteY
    RET_HL   | 2, ARG_NORM,    // FlipSpriteX
    RET_HL   | 2, ARG_NORM,    // RotateSpriteC
    RET_HL   | 2, ARG_NORM,    // RotateSpriteCC
    RET_HL   | 2, ARG_NORM,    // RotateSpriteHalf
    RET_NONE | 2, ARG_NORM,    // Polygon
    RET_NONE | 2, ARG_NORM,    // Polygon_NoClip
    RET_NONE | 6, ARG_NORM,    // FillTriangle
    RET_NONE | 6, ARG_NORM,    // FillTriangle_NoClip
    UN       | 0, ARG_NORM,    // LZDecompressSprite
    RET_NONE | 2, SMALL_12,    // SetTextScale
    RET_A    | 1, SMALL_1,     // SetTransparentColor
    RET_NONE | 0, ARG_NORM,    // ZeroScreen
    RET_NONE | 1, SMALL_1,     // SetTextConfig
    RET_HL   | 1, SMALL_1,     // GetSpriteChar
    RET_HLs  | 2, SMALL_2,     // Lighten
    RET_HLs  | 2, SMALL_2,     // Darken
    RET_A    | 1, SMALL_1,     // SetFontHeight
    RET_HL   | 2, ARG_NORM,    // ScaleSprite
    RET_NONE | 3, SMALL_12,    // FloodFill
    RET_NONE | 3, ARG_NORM,    // RLETSprite
    RET_NONE | 3, SMALL_3,     // RLETSprite_NoClip
    UN       | 2, ARG_NORM,    // ConvertFromRLETSprite
    UN       | 2, ARG_NORM,    // ConvertToRLETSprite
    UN       | 2, ARG_NORM,    // ConvertToNewRLETSprite
    RET_HL   | 4, SMALL_34,    // RotateScaleSprite
    RET_A    | 5, SMALL_345,   // RotatedScaledTransparentSprite_NoClip
    RET_A    | 5, SMALL_345,   // RotatedScaledSprite_NoClip
};

const uint8_t FileiocArgs[] = {
    RET_NONE | 0, ARG_NORM,    // CloseAll
    RET_A    | 2, ARG_NORM,    // Open
    RET_A    | 3, SMALL_3,     // OpenVar
    RET_NONE | 1, SMALL_1,     // Close
    RET_HL   | 4, SMALL_4,     // Write
    RET_HL   | 4, SMALL_4,     // Read
    RET_HL   | 1, SMALL_1,     // GetChar
    RET_HL   | 2, SMALL_12,    // PutChar
    RET_HL   | 1, ARG_NORM,    // Delete
    RET_HL   | 2, SMALL_2,     // DeleteVar
    RET_HL   | 3, SMALL_23,    // Seek
    RET_HL   | 2, SMALL_2,     // Resize
    RET_HL   | 1, SMALL_1,     // IsArchived
    RET_NONE | 2, SMALL_12,    // SetArchiveStatus
    RET_HL   | 1, SMALL_1,     // Tell
    RET_HL   | 1, SMALL_1,     // Rewind
    RET_HL   | 1, SMALL_1,     // GetSize
    RET_HL   | 1, SMALL_1,     // GetTokenString
    RET_HL   | 1, SMALL_1,     // GetDataPtr
    UN       | 0, ARG_NORM,    // Detect
    UN       | 0, ARG_NORM,    // DetectVar
};

extern uint8_t outputStack[4096];

uint8_t parseFunction(uint24_t index) {
    element_t *outputPtr = (element_t*)outputStack, *outputPrev, *outputCurr, *outputPrevPrev;
    uint8_t function, function2, amountOfArguments, temp, a, outputPrevType, res;
    uint24_t output, endIndex, startIndex, outputPrevOperand;
    
    outputPrevPrev    = &outputPtr[getIndexOffset(-3)];
    outputPrev        = &outputPtr[getIndexOffset(-2)];
    output            = (&outputPtr[index])->operand;
    outputCurr        = &outputPtr[getIndexOffset(-1)];
    function          = output;
    function2         = output >> 16;
    amountOfArguments = output >> 8;
    
    outputPrevOperand     = outputPrev->operand;
    outputPrevType        = outputPrev->type;
    
    expr.outputRegister2 = OUTPUT_IN_HL;
    expr.AnsSetZeroFlag = expr.AnsSetZeroFlagReversed = expr.AnsSetCarryFlag = expr.AnsSetCarryFlagReversed = false;
    
    // not(
    if (function == tNot) {
        if ((res = parseFunction1Arg(index, OUTPUT_IN_HL_DE, amountOfArguments)) != VALID) {
            return res;
        }
        if (expr.outputRegister == OUTPUT_IN_HL) {
            LD_DE_IMM(-1);
        } else {
            LD_HL_IMM(-1);
        }
        ADD_HL_DE();
        SBC_HL_HL_INC_HL();
        expr.AnsSetCarryFlag = true;
        expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
    }
    
    // sqrt(
    else if (function == tSqrt) {
        if ((res = parseFunction1Arg(index, OUTPUT_IN_HL, amountOfArguments)) != VALID) {
            return res;
        }
        
        ProgramPtrToOffsetStack();
        if (!ice.usedAlreadySqrt) {
            ice.SqrtAddr = (uintptr_t)ice.programDataPtr;
            memcpy(ice.programDataPtr, SqrtData, 43);
            ice.programDataPtr += 43;
            ice.usedAlreadySqrt = true;
        }
        CALL(ice.SqrtAddr);
        expr.outputRegister2 = OUTPUT_IN_DE;
        ice.modifiedIY = true;
    }
    
    // sin(, cos(
    else if (function == tSin || function == tCos) {
        if ((res = parseFunction1Arg(index, OUTPUT_IN_HL, amountOfArguments)) != VALID) {
            return res;
        }
        
        if (!ice.usedAlreadySinCos) {
            ice.SinCosAddr = (uintptr_t)ice.programDataPtr;
            memcpy(ice.programDataPtr, SinCosData, 95);
            ice.programDataPtr += 95;
            ice.usedAlreadySinCos = true;
        }
        ProgramPtrToOffsetStack();
        LD_DE_IMM(ice.SinCosAddr + 30);
        
        ProgramPtrToOffsetStack();
        CALL(ice.SinCosAddr + (function == tSin ? 4 : 0));
        
        expr.outputRegister2 = OUTPUT_IN_DE;
    }
    
    // min(
    else if (function == tMin) {
        if ((res = parseFunction2Args(index, OUTPUT_IN_DE, amountOfArguments, false)) != VALID) {
            return res;
        }
        OR_A_SBC_HL_DE();
        ADD_HL_DE();
        JR_C(1);
        EX_DE_HL();
    }
    
    // max(
    else if (function == tMax) {
        if ((res = parseFunction2Args(index, OUTPUT_IN_DE, amountOfArguments, false)) != VALID) {
            return res;
        }
        OR_A_SBC_HL_DE();
        ADD_HL_DE();
        JR_NC(1);
        EX_DE_HL();
    }
    
    // mean(
    else if (function == tMean) {
        if ((res = parseFunction2Args(index, OUTPUT_IN_DE, amountOfArguments, false)) != VALID) {
            return res;
        }
        ProgramPtrToOffsetStack();
        if (!ice.usedAlreadyMean) {
            ice.MeanAddr = (uintptr_t)ice.programDataPtr;
            memcpy(ice.programDataPtr, MeanData, 19);
            ice.programDataPtr += 19;
            ice.usedAlreadyMean = true;
        }
        CALL(ice.MeanAddr);
        ice.modifiedIY = true;
    }
    
    // remainder(
    else if (function2 == tRemainder) {
        if (outputPrev->type == TYPE_NUMBER && outputPrev->operand <= 256 && !((uint8_t)outputPrev->operand & (uint8_t)(outputPrev->operand - 1))) {
            if (outputPrevPrev->type == TYPE_VARIABLE) {
                LD_A_IND_IX_OFF(outputPrevPrev->operand);
            } else if (outputPrevPrev->type == TYPE_FUNCTION_RETURN) {
                insertFunctionReturnNoPush(outputPrevPrev->operand, OUTPUT_IN_HL);
                LD_A_L();
            } else if (outputPrevPrev->type == TYPE_CHAIN_ANS) {
                if (expr.outputRegister == OUTPUT_IN_HL) {
                    LD_A_L();
                } else {
                    LD_A_E();
                }
            } else {
                return E_ICE_ERROR;
            }
            if (outputPrev->operand == 256) {
                OR_A_A();
            } else {
                AND_A(outputPrev->operand - 1);
            }
            SBC_HL_HL();
            LD_L_A();
        } else {
            if ((res = parseFunction2Args(index, OUTPUT_IN_BC, amountOfArguments, true)) != VALID) {
                return res;
            }
            CALL(__idvrmu);
        }
    }
    
    // sub(
    else if (function == t2ByteTok && function2 == tSubStrng) {
        element_t *outputPrevPrevPrev = &outputPtr[getIndexOffset(-4)];
        uint24_t outputPrevPrevPrevOperand = outputPrevPrevPrev->operand;
        
        // First argument should be a string
        if (outputPrevPrevPrev->type < TYPE_STRING) {
            return E_SYNTAX;
        }
        
        // Parse last 2 argument
        if ((res = parseFunction2Args(index, OUTPUT_IN_BC, amountOfArguments - 1, true)) != VALID) {
            return res;
        }
        
        // Get the string into DE
        if (outputPrevPrevPrev->type == TYPE_STRING && 
              outputPrevPrevPrevOperand != ice.tempStrings[TempString1] && outputPrevPrevPrevOperand != ice.tempStrings[TempString2]) {
            ProgramPtrToOffsetStack();
        }
        LD_DE_IMM(outputPrevPrevPrevOperand);
        
        // Add the offset to the string, and copy!
        ADD_HL_DE();
        if (outputPrevPrevPrevOperand == ice.tempStrings[TempString1]) {
            LD_DE_IMM(ice.tempStrings[TempString2]);
        } else {
            LD_DE_IMM(ice.tempStrings[TempString1]);
        }
        LDIR();
        EX_DE_HL();
        LD_HL_VAL(0);
    }
    
    // length(
    else if (function2 == tLength) {
        if (amountOfArguments != 1) {
            return E_ARGUMENTS;
        }
        
        // Should be a string
        if (outputPrev->type < TYPE_STRING) {
            return E_SYNTAX;
        }
        
        // Get the length
        LD_HL_STRING(outputPrev->operand);
        PUSH_HL();
        CALL(__strlen);
        POP_BC();
    }
    
    // Here are coming the special functions, with abnormal arguments
    
    // Data(
    else if (function == tVarOut && function2 == tData) {
        element_t *outputTemp;
        uint24_t startIndex = -1 - amountOfArguments;
        uint8_t a;
        
        ProgramPtrToOffsetStack();
        LD_HL_IMM((uint24_t)ice.programDataPtr);
        
        for (a = 0; a < amountOfArguments; a++) {
            outputTemp = &outputPtr[getIndexOffset(startIndex + a)];
            if (outputTemp->type != TYPE_NUMBER) {
                return E_SYNTAX;
            }
            *ice.programDataPtr++ = outputTemp->operand;
        }
    }
    
    // Copy(
    else if (function2 == tCopy) {
        return E_UNIMPLEMENTED;
    }
    
    // DefineSprite(
    else if (function2 == tDefineSprite) {
        element_t *outputPrevPrevPrev = &outputPtr[getIndexOffset(-4)];
        
        ProgramPtrToOffsetStack();
        
        if (amountOfArguments == 2) {
            if (outputPrevPrev->type != TYPE_NUMBER || outputPrevType != TYPE_NUMBER) {
                return E_SYNTAX;
            }
            
            LD_HL_IMM((uint24_t)ice.programDataPtr);
            ice.programDataPtr += outputPrevPrev->operand * outputPrev->operand;
        } else if (amountOfArguments == 3) {
            uint8_t *prevDataPtr = (uint8_t*)outputPrev->operand, *prevDataPtr2 = prevDataPtr;
            
            if(outputPrevPrevPrev->type != TYPE_NUMBER || outputPrevPrev->type != TYPE_NUMBER || outputPrev->type != TYPE_STRING) {
                return E_SYNTAX;
            }
            
            // Replace the hexadecimal string to hexadecimal bytes
            LD_HL_IMM((uint24_t)prevDataPtr);
            while (prevDataPtr != ice.programDataPtr - 1) {
                uint8_t tok1, tok2;
                
                if ((tok1 = IsHexadecimal(*prevDataPtr++)) == 16 || (tok2 = IsHexadecimal(*prevDataPtr++)) == 16) {
                    return E_SYNTAX;
                }
                *prevDataPtr2++ = (tok1 << 4) + tok2;
            }
            
            ice.programDataPtr = prevDataPtr2;
        } else {
            return E_ARGUMENTS;
        }
    }
    
    // {
    else if (function == tLBrace) {
        if (amountOfArguments != 1) {
            return E_ARGUMENTS;
        }
        if (outputPrevType == TYPE_NUMBER) {
            if (outputCurr->mask == TYPE_MASK_U8) {
                LD_A_ADDR(outputPrevOperand);
            } else if (outputCurr->mask == TYPE_MASK_U16) {
                LD_HL_ADDR(outputPrevOperand);
                EX_S_DE_HL();
                expr.outputRegister2 = OUTPUT_IN_DE;
            } else {
                LD_HL_ADDR(outputPrevOperand);
            }
        } else if (outputPrevType == TYPE_VARIABLE) {
            LD_HL_IND_IX_OFF(outputPrevOperand);
            if (outputCurr->mask == TYPE_MASK_U8) {
                LD_A_HL();
            } else if (outputCurr->mask == TYPE_MASK_U16) {
                LD_HL_HL();
                EX_S_DE_HL();
                expr.outputRegister2 = OUTPUT_IN_DE;
            } else {
                LD_HL_HL();
            }
        } else if (outputPrevType == TYPE_FUNCTION_RETURN) {
            insertFunctionReturnNoPush(outputPrevOperand, OUTPUT_IN_HL);
            if (outputCurr->mask == TYPE_MASK_U8) {
                LD_A_HL();
            } else if (outputCurr->mask == TYPE_MASK_U16) {
                LD_HL_HL();
                EX_S_DE_HL();
                expr.outputRegister2 = OUTPUT_IN_DE;
            } else {
                LD_HL_HL();
            }
        } else if (outputPrevType == TYPE_CHAIN_ANS) {
            if (outputCurr->mask == TYPE_MASK_U8) {
                if (expr.outputRegister == OUTPUT_IN_HL) {
                    LD_A_HL();
                } else {
                    LD_A_DE();
                }
            } else if (outputCurr->mask == TYPE_MASK_U16) {
                MaybeDEToHL();
                LD_HL_HL();
                EX_S_DE_HL();
                expr.outputRegister2 = OUTPUT_IN_DE;
            } else {
                MaybeDEToHL();
                LD_HL_HL();
            }
        } else {
            return E_SYNTAX;
        }
        if (outputCurr->mask == TYPE_MASK_U8) {
            OR_A_A();
            SBC_HL_HL();
            LD_L_A();
            expr.AnsSetZeroFlag = true;
            expr.ZeroCarryFlagRemoveAmountOfBytes = 3;
        }
    }
    
    // det(, sum(
    else if (function == tDet || function == tSum) {
        endIndex = index;
        startIndex = index;
        
        // Get all the arguments
        for (a = 0; a < amountOfArguments; a++) {
            uint24_t *tempP1, *tempP2;
            
            temp = 0;
            while (1) {
                outputPrev = &outputPtr[--startIndex];
                outputPrevType = outputPrev->type;
                
                if (outputPrevType == TYPE_C_START) {
                    if (!temp) {
                        break;
                    }
                    temp--;
                }
                
                if (outputPrevType == TYPE_FUNCTION && ((uint8_t)outputPrevOperand == tDet || (uint8_t)outputPrevOperand == tSum)) {
                    temp++;
                }
                
                if (outputPrevType == TYPE_ARG_DELIMITER && !temp) {
                    break;
                }
            }
            
            // Check if it's the first argument or not
            if ((outputPrevType == TYPE_ARG_DELIMITER && a == amountOfArguments - 1) ||
                (outputPrevType == TYPE_C_START && a != amountOfArguments - 1)) {
                return E_ARGUMENTS;
            }
            
            // Setup a new stack
            tempP1 = getStackVar(0);
            tempP2 = getStackVar(1);
            ice.stackDepth++;
            
            // And finally grab the argument, and return if an error occured
            if ((temp = parsePostFixFromIndexToIndex(startIndex + 1, endIndex - 1)) != VALID) {
                return temp;
            }
            
            ice.stackDepth--;
            
            // And restore the stack
            setStackValues(tempP1, tempP2);
            
            // Push the argument
            PushHLDE();
            
            endIndex = startIndex--;
        }
        
        // Invalid first argument of det(
        if (!expr.outputIsNumber) {
            return E_SYNTAX;
        }
        
        // Wow, unknown C function?
        if (expr.outputNumber >= (function == tDet ? AMOUNT_OF_GRAPHX_FUNCTIONS : AMOUNT_OF_FILEIOC_FUNCTIONS)) {
            return E_UNKNOWN_C;
        }
        
        // Lel, we need to remove the last argument (ld hl, XXXXXX) + the push
        ice.programPtr -= 4 + (expr.outputNumber > 0);
        
        // Get the amount of arguments, and call the function
        if (function == tDet) {
            temp = GraphxArgs[expr.outputNumber * 2];
            CALL(ice.GraphxRoutinesStack[expr.outputNumber]);
        } else {
            temp = FileiocArgs[expr.outputNumber * 2];
            CALL(ice.FileiocRoutinesStack[expr.outputNumber]);
        }
        
        // Check if unimplemented function
        if (temp & UN) {
            return E_UNIMPLEMENTED;
        }
        
        // Check if deprecated function
        if (temp & DEPR) {
            return E_DEPRECATED;
        }
        
        // Check the right amount of arguments
        if ((temp & 7) != amountOfArguments - 1) {
            return E_ARGUMENTS;
        }
        
        // And pop the arguments
        for (a = 1; a < amountOfArguments; a++) {
            POP_BC();
        }
        
        // Check if the output is 16-bits OR in A
        if (temp & RET_A) {
            OR_A_A();
            SBC_HL_HL();
            LD_L_A();
        } else if (temp & RET_HLs) {
            EX_S_DE_HL();
            expr.outputRegister2 = OUTPUT_IN_DE;
        }
    }
    
    expr.outputRegister = expr.outputRegister2;
    
    return VALID;
}

uint8_t parseFunction1Arg(uint24_t index, uint8_t outputRegister1, uint8_t amountOfArguments) {
    element_t *outputPtr = (element_t*)outputStack, *outputPrev;
    uint24_t outputOperand;
    uint8_t outputPrevType;
    
    outputPrev = &outputPtr[getIndexOffset(-2)];
    outputPrevType = outputPrev->type;
    outputOperand = outputPrev->operand;
    
    if (amountOfArguments != 1) {
        return E_ARGUMENTS;
    }
    
    if (outputPrevType == TYPE_VARIABLE) {
        LD_HL_IND_IX_OFF(outputOperand);
    } else if (outputPrevType == TYPE_FUNCTION_RETURN) {
        insertFunctionReturnNoPush(outputOperand, OUTPUT_IN_HL);
    } else if (outputPrevType == TYPE_CHAIN_ANS) {
        if (outputRegister1 == OUTPUT_IN_HL) {
            MaybeDEToHL();
            expr.outputRegister = OUTPUT_IN_HL;
        }
    } else {
        return E_SYNTAX;
    }
    
    return VALID;
}

uint8_t parseFunction2Args(uint24_t index, uint8_t outputRegister2, uint8_t amountOfArguments, bool orderDoesMatter) {
    element_t *outputPtr = (element_t*)outputStack, *outputPrev, *outputPrevPrev;
    uint8_t outputPrevType, outputPrevPrevType;
    uint24_t outputPrevOperand, outputPrevPrevOperand;
    
    outputPrev            = &outputPtr[getIndexOffset(-2)];
    outputPrevPrev        = &outputPtr[getIndexOffset(-3)];
    outputPrevType        = outputPrev->type;
    outputPrevPrevType    = outputPrevPrev->type;
    outputPrevOperand     = outputPrev->operand;
    outputPrevPrevOperand = outputPrevPrev->operand;
    
    if (amountOfArguments != 2) {
        return E_ARGUMENTS;
    }
    
    if (outputPrevPrevType == TYPE_NUMBER) {
        if (outputPrevType == TYPE_VARIABLE) {
            LD_HL_NUMBER(outputPrevPrevOperand);
            if (outputRegister2 == OUTPUT_IN_DE) {
                LD_DE_IND_IX_OFF(outputPrevOperand);
            } else {
                LD_BC_IND_IX_OFF(outputPrevOperand);
            }
        } else if (outputPrevType == TYPE_FUNCTION_RETURN) {
            if (orderDoesMatter) {
                insertFunctionReturnNoPush(outputPrevOperand, outputRegister2);
                LD_HL_NUMBER(outputPrevPrevOperand);
            } else {
                insertFunctionReturnNoPush(outputPrevOperand, OUTPUT_IN_HL);
                LD_DE_IMM(outputPrevPrevOperand);
            }
        } else if (outputPrevType == TYPE_CHAIN_ANS) {
            if (orderDoesMatter) {
                if (outputRegister2 == OUTPUT_IN_DE) {
                    MaybeHLToDE();
                } else {
                    PushHLDE();
                    POP_BC();
                }
                LD_HL_NUMBER(outputPrevPrevOperand);
            } else {
                if (expr.outputRegister == OUTPUT_IN_HL) {
                    LD_DE_IMM(outputPrevPrevOperand);
                } else {
                    LD_HL_NUMBER(outputPrevPrevOperand);
                }
            }
        } else {
            return E_ICE_ERROR;
        }
    } else if (outputPrevPrevType == TYPE_VARIABLE) {
        if (outputPrevType == TYPE_NUMBER) {
            if (orderDoesMatter) {
                LD_HL_IND_IX_OFF(outputPrevPrevOperand);
                if (outputRegister2 == OUTPUT_IN_DE) {
                    LD_DE_IMM(outputPrevOperand);
                } else {
                    LD_BC_IMM(outputPrevOperand);
                }
            } else {
                LD_HL_NUMBER(outputPrevPrevOperand);
                LD_DE_IND_IX_OFF(outputPrevOperand);
            }
        } else if (outputPrevType == TYPE_VARIABLE) {
            LD_HL_IND_IX_OFF(outputPrevPrevOperand);
            if (outputRegister2 == OUTPUT_IN_DE) {
                LD_DE_IND_IX_OFF(outputPrevOperand);
            } else {
                LD_BC_IND_IX_OFF(outputPrevOperand);
            }
        } else if (outputPrevType == TYPE_FUNCTION_RETURN) {
            if (orderDoesMatter) {
                insertFunctionReturnNoPush(outputPrevOperand, outputRegister2);
                LD_HL_IND_IX_OFF(outputPrevPrevOperand);
            } else {
                insertFunctionReturnNoPush(outputPrevOperand, OUTPUT_IN_HL);
                LD_DE_IND_IX_OFF(outputPrevPrevOperand);
            }
        } else if (outputPrevType == TYPE_CHAIN_ANS) {
            if (orderDoesMatter) {
                if (outputRegister2 == OUTPUT_IN_DE) {
                    MaybeHLToDE();
                } else {
                    PushHLDE();
                    POP_BC();
                }
                LD_HL_IND_IX_OFF(outputPrevPrevOperand);
            } else {
                if (expr.outputRegister == OUTPUT_IN_HL) {
                    LD_DE_IND_IX_OFF(outputPrevPrevOperand);
                } else {
                    LD_HL_IND_IX_OFF(outputPrevPrevOperand);
                }
            }
        } else {
            return E_ICE_ERROR;
        }
    } else if (outputPrevPrevType == TYPE_FUNCTION_RETURN) {
        if (outputPrevType == TYPE_NUMBER) {
            insertFunctionReturnNoPush(outputPrevPrevOperand, OUTPUT_IN_HL);
            if (outputRegister2 == OUTPUT_IN_DE) {
                LD_DE_IMM(outputPrevOperand);
            } else {
                LD_BC_IMM(outputPrevOperand);
            }
        } else if (outputPrevType == TYPE_VARIABLE) {
            insertFunctionReturnNoPush(outputPrevPrevOperand, OUTPUT_IN_HL);
            if (outputRegister2 == OUTPUT_IN_DE) {
                LD_DE_IND_IX_OFF(outputPrevOperand);
            } else {
                LD_BC_IND_IX_OFF(outputPrevOperand);
            }
        } else if (outputPrevType == TYPE_FUNCTION_RETURN) {
            insertFunctionReturnNoPush(outputPrevOperand, outputRegister2);
            insertFunctionReturnNoPush(outputPrevPrevOperand, OUTPUT_IN_HL);
        } else if (outputPrevType == TYPE_CHAIN_ANS) {
            if (orderDoesMatter) {
                PushHLDE();
                insertFunctionReturnNoPush(outputPrevPrevOperand, OUTPUT_IN_HL);
                if (outputRegister2 == OUTPUT_IN_DE) {
                    POP_DE();
                } else {
                    POP_BC();
                }
            } else {
                if (expr.outputRegister == OUTPUT_IN_HL) {
                    PUSH_HL();
                    insertFunctionReturnNoPush(outputPrevPrevOperand, OUTPUT_IN_HL);
                    POP_DE();
                } else {
                    insertFunctionReturn(outputPrevPrevOperand, OUTPUT_IN_HL, NEED_PUSH);
                }
            }
        } else {
            return E_ICE_ERROR;
        }
    } else if (outputPrevPrevType == TYPE_CHAIN_ANS) {
        if (outputPrevType == TYPE_NUMBER) {
            if (orderDoesMatter) {
                MaybeDEToHL();
                if (outputRegister2 == OUTPUT_IN_DE) {
                    LD_DE_IMM(outputPrevOperand);
                } else {
                    LD_BC_IMM(outputPrevOperand);
                }
            } else {
                if (expr.outputRegister == OUTPUT_IN_HL) {
                    LD_DE_IMM(outputPrevOperand);
                } else {
                    LD_HL_NUMBER(outputPrevOperand);
                }
            }
        } else if (outputPrevType == TYPE_VARIABLE) {
            if (orderDoesMatter) {
                MaybeDEToHL();
                if (outputRegister2 == OUTPUT_IN_DE) {
                    LD_DE_IND_IX_OFF(outputPrevOperand);
                } else {
                    LD_BC_IND_IX_OFF(outputPrevOperand);
                }
            } else {
                if (expr.outputRegister == OUTPUT_IN_HL) {
                    LD_DE_IND_IX_OFF(outputPrevOperand);
                } else {
                    LD_HL_IND_IX_OFF(outputPrevOperand);
                }
            }
        } else if (outputPrevType == TYPE_FUNCTION_RETURN) {
            if (orderDoesMatter) {
                PushHLDE();
                insertFunctionReturnNoPush(outputPrevOperand, outputRegister2);
                POP_HL();
            } else {
                if (expr.outputRegister == OUTPUT_IN_HL) {
                    PUSH_HL();
                    insertFunctionReturnNoPush(outputPrevOperand, OUTPUT_IN_HL);
                    POP_DE();
                } else {
                    insertFunctionReturn(outputPrevOperand, OUTPUT_IN_HL, NEED_PUSH);
                }
            }
        } else {
            return E_ICE_ERROR;
        }
    } else if (outputPrevPrevType == TYPE_CHAIN_PUSH) {
        if (outputPrevType != TYPE_CHAIN_ANS) {
            return E_ICE_ERROR;
        }
        if (orderDoesMatter) {
            if (outputRegister2 == OUTPUT_IN_DE) {
                MaybeHLToDE();
            } else {
                PushHLDE();
                POP_BC();
            }
            POP_HL();
        } else {
            if (expr.outputRegister == OUTPUT_IN_HL) {
                POP_DE();
            } else {
                POP_HL();
            }
        }
    } else {
        return E_SYNTAX;
    }
    
    return VALID;
}

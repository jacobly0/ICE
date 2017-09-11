#include "defines.h"
#include "functions.h"

#include "errors.h"
#include "stack.h"
#include "parse.h"
#include "main.h"
#include "output.h"
#include "operator.h"
#include "routines.h"
//#include "gfx/gfx_logos.h"

#ifdef COMPUTER_ICE
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(Sqrt, "src/asm/sqrt.bin");
INCBIN(Mean, "src/asm/mean.bin");
#endif

/* First byte:  bit 7  : returns something in A
                bit 6  : unimplemented
                bit 5  : returns something in HL(s)
                bit 4  : extra bit
                bit 2-0: amount of arguments needed
   Second byte: bit 7  : first argument is small
                bit 6  : second argument is small
                bit 5  : third argument is small
                ...
*/

#define AMOUNT_OF_C_FUNCTIONS 92

const uint8_t CArguments[] = {
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
    RET_NONE | 1, ARG_NORM,    // GetStringWidth
    RET_NONE | 1, SMALL_1,     // GetCharWidth
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
    UN       | 0, ARG_NORM,    // GetClipRegion
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
    UN       | 0, ARG_NORM,    // GetSprite
    RET_NONE | 5, SMALL_45,    // ScaledSprite_NoClip
    RET_NONE | 5, SMALL_45,    // ScaledTransparentSprite_NoClip
    UN       | 0, ARG_NORM,    // FlipSpriteY
    UN       | 0, ARG_NORM,    // FlipSpriteX
    UN       | 0, ARG_NORM,    // RotateSpriteC
    UN       | 0, ARG_NORM,    // RotateSpriteCC
    UN       | 0, ARG_NORM,    // RotateSpriteHalf
    UN       | 0, ARG_NORM,    // Polygon
    UN       | 0, ARG_NORM,    // Polygon_NoClip
    RET_NONE | 6, ARG_NORM,    // FillTriangle
    RET_NONE | 6, ARG_NORM,    // FillTriangle_NoClip
    UN       | 0, ARG_NORM,    // LZDecompressSprite
    RET_NONE | 2, SMALL_12,    // SetTextScale
    RET_A    | 1, SMALL_1,     // SetTransparentColor
    RET_NONE | 0, ARG_NORM,    // ZeroScreen
    RET_NONE | 1, SMALL_1,     // SetTextConfig
    UN       | 0, ARG_NORM,    // GetSpriteChar
    RET_HL   | 2, SMALL_2,     // Lighten
    RET_HL   | 2, SMALL_2,     // Darken
    RET_A    | 1, SMALL_1,     // SetFontHeight
    UN       | 0, ARG_NORM,    // ScaleSprite
    RET_NONE | 3, SMALL_12,    // FloodFill
    RET_NONE | 3, ARG_NORM,    // RLETSprite
    RET_NONE | 3, SMALL_3,     // RLETSprite_NoClip
    UN       | 2, ARG_NORM,    // ConvertFromRLETSprite
    UN       | 2, ARG_NORM,    // ConvertToRLETSprite
    UN       | 2, ARG_NORM,    // ConvertToNewRLETSprite
    UN       | 4, ARG_NORM,    // RotateScaleSprite
    UN       | 4, ARG_NORM,    // RotatedScaledTransparentSprite_NoClip
    UN       | 4, ARG_NORM     // RotatedScaledSprite_NoClip
};

extern uint8_t outputStack[4096];

uint8_t parseFunction(uint24_t index) {
    element_t *outputPtr = (element_t*)outputStack, *outputPrev, *outputCurr;
    uint8_t function, function2, amountOfArguments, temp, a, outputPrevType, res;
    uint24_t output, endIndex, startIndex, outputPrevOperand;
    
    outputPrev        = &outputPtr[getIndexOffset(-2)];
    output            = (&outputPtr[index])->operand;
    outputCurr        = &outputPtr[getIndexOffset(-1)];
    function          = (uint8_t)output;
    function2         = (uint8_t)(output >> 16);
    amountOfArguments = (uint8_t)(output >> 8);
    
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
        SBC_HL_HL();
        INC_HL();
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
            memcpy(ice.programDataPtr, SqrtData, 44);
            ice.programDataPtr += 44;
            ice.usedAlreadySqrt = true;
        }
        CALL(ice.SqrtAddr);
        expr.outputRegister2 = OUTPUT_IN_DE;
        ice.modifiedIY = true;
    }
    
    // min(
    else if (function == tMin) {
        if ((res = parseFunction2Args(index, OUTPUT_IN_DE, amountOfArguments, false)) != VALID) {
            return res;
        }
        OR_A_A();
        SBC_HL_DE();
        ADD_HL_DE();
        JR_C(1);
        EX_DE_HL();
    }
    
    // max(
    else if (function == tMax) {
        if ((res = parseFunction2Args(index, OUTPUT_IN_DE, amountOfArguments, false)) != VALID) {
            return res;
        }
        OR_A_A();
        SBC_HL_DE();
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
        if ((res = parseFunction2Args(index, OUTPUT_IN_BC, amountOfArguments, true)) != VALID) {
            return res;
        }
        CALL(__idvrmu);
    }
    
    // sub(
    else if (function2 == tSubStrng) {
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
            insertFunctionReturn(outputPrevOperand, OUTPUT_IN_HL, NO_PUSH);
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
    
    // det(
    else if (function == tDet) {
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
                
                if (outputPrevType == TYPE_FUNCTION && (uint8_t)outputPrevOperand == tDet) {
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
        if (expr.outputNumber >= AMOUNT_OF_C_FUNCTIONS) {
            return E_UNKNOWN_C;
        }
        
        // Lel, we need to remove the last argument (ld hl, XXXXXX) + the push
        ice.programPtr -= 4 + (expr.outputNumber > 0);
        
        temp = CArguments[expr.outputNumber * 2];
        
        // Check if unimplemented function
        if (temp & UN) {
            return E_NOT_IMPLEMENTED;
        }
        
        // Check if deprecated function
        if (temp & DEPR) {
            return E_DEPRECATED;
        }
        
        // Check the right amount of arguments
        if ((temp & 7) != amountOfArguments - 1) {
            return E_ARGUMENTS;
        }
        
        // If it's Begin, push gfx_Bpp8 as well
        if (!expr.outputNumber) {
            LD_L(0x27);
            PUSH_HL();
        }
        
        // Call the function
        CALL(ice.CRoutinesStack[expr.outputNumber]*4 + 0xD00000);
        
        // If it's Begin, pop gfx_Bpp8 as well
        if (!expr.outputNumber) {
            POP_BC();
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
        } else if (temp & RET_HL) {
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
        insertFunctionReturn(outputOperand, OUTPUT_IN_HL, NO_PUSH);
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
                insertFunctionReturn(outputPrevOperand, outputRegister2, NO_PUSH);
                LD_HL_NUMBER(outputPrevPrevOperand);
            } else {
                insertFunctionReturn(outputPrevOperand, OUTPUT_IN_HL, NO_PUSH);
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
                insertFunctionReturn(outputPrevOperand, outputRegister2, NO_PUSH);
                LD_HL_IND_IX_OFF(outputPrevPrevOperand);
            } else {
                insertFunctionReturn(outputPrevOperand, OUTPUT_IN_HL, NO_PUSH);
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
            insertFunctionReturn(outputPrevPrevOperand, OUTPUT_IN_HL, NO_PUSH);
            if (outputRegister2 == OUTPUT_IN_DE) {
                LD_DE_IMM(outputPrevOperand);
            } else {
                LD_BC_IMM(outputPrevOperand);
            }
        } else if (outputPrevType == TYPE_VARIABLE) {
            insertFunctionReturn(outputPrevPrevOperand, OUTPUT_IN_HL, NO_PUSH);
            if (outputRegister2 == OUTPUT_IN_DE) {
                LD_DE_IND_IX_OFF(outputPrevOperand);
            } else {
                LD_BC_IND_IX_OFF(outputPrevOperand);
            }
        } else if (outputPrevType == TYPE_FUNCTION_RETURN) {
            insertFunctionReturn(outputPrevOperand, outputRegister2, NO_PUSH);
            insertFunctionReturn(outputPrevPrevOperand, OUTPUT_IN_HL, NO_PUSH);
        } else if (outputPrevType == TYPE_CHAIN_ANS) {
            if (orderDoesMatter) {
                PushHLDE();
                insertFunctionReturn(outputPrevPrevOperand, OUTPUT_IN_HL, NO_PUSH);
                if (outputRegister2 == OUTPUT_IN_DE) {
                    POP_DE();
                } else {
                    POP_BC();
                }
            } else {
                if (expr.outputRegister == OUTPUT_IN_HL) {
                    PUSH_HL();
                    insertFunctionReturn(outputPrevPrevOperand, OUTPUT_IN_HL, NO_PUSH);
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
                insertFunctionReturn(outputPrevOperand, outputRegister2, NO_PUSH);
                POP_HL();
            } else {
                if (expr.outputRegister == OUTPUT_IN_HL) {
                    PUSH_HL();
                    insertFunctionReturn(outputPrevOperand, OUTPUT_IN_HL, NO_PUSH);
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

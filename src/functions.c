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
#include "functions.h"

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
    RET_NONE | 3, SMALL_12     // FloodFill
};

uint8_t parseFunction(uint24_t index) {
    const uint8_t *outputStack = (uint8_t*)0xD62C00;
    element_t *outputPtr = (element_t*)outputStack;
    element_t *outputPrev, *outputPrevPrev;
    uint8_t function, amountOfArguments, tempType, temp, a;
    uint24_t output, endIndex, startIndex;
    uint24_t *tempP1, *tempP2;
    
    outputPrev        = &outputPtr[getIndexOffset(-2)];
    outputPrevPrev    = &outputPtr[getIndexOffset(-3)];
    output            = (&outputPtr[index])->operand;
    function          = (uint8_t)output;
    amountOfArguments = (uint8_t)(output >> 8);
    
    expr.outputRegister2 = OutputRegisterHL;
    
    tempType = outputPrev->type;
    switch (function) {
        case tNot:
            if (tempType == TYPE_VARIABLE) {
                LD_HL_IND_IX_OFF(outputPrev->operand);
            } else if (tempType == TYPE_FUNCTION_RETURN) {
                insertFunctionReturn(outputPrev->operand, OUTPUT_IN_HL, NO_PUSH);
            } else if (tempType != TYPE_CHAIN_ANS) {
                return E_SYNTAX;
            }
            if (expr.outputRegister == OutputRegisterHL) {
                LD_DE_IMM(-1);
            } else {
                LD_HL_IMM(-1);
            }
            ADD_HL_DE();
            SBC_HL_HL();
            INC_HL();
            break;
        case tMin:
        case tMax:
        case tMean:
            switch (outputPrevPrev->type) {
                case TYPE_NUMBER:
                    if (tempType == TYPE_VARIABLE) {
                        LD_HL_IND_IX_OFF(outputPrev->operand);
                    } else if (tempType == TYPE_FUNCTION_RETURN) {
                        insertFunctionReturn(outputPrev->operand, OUTPUT_IN_HL, NO_PUSH);
                    } else if (tempType != TYPE_CHAIN_ANS) {
                        return E_SYNTAX;
                    }
                    
                    if (expr.outputRegister == OutputRegisterHL) {
                        LD_DE_IMM(outputPrevPrev->operand);
                    } else {
                        LD_HL_IMM(outputPrevPrev->operand);
                    }
                    break;
                case TYPE_VARIABLE:
                    if (tempType == TYPE_NUMBER) {
                        LD_HL_NUMBER(outputPrev->operand);
                    } else if (tempType == TYPE_VARIABLE) {
                        LD_HL_IND_IX_OFF(outputPrev->operand);
                        if (outputPrev->operand == outputPrevPrev->operand) {
                            return VALID;
                        }
                    } else if (tempType == TYPE_FUNCTION_RETURN) {
                        insertFunctionReturn(outputPrev->operand, OUTPUT_IN_HL, NO_PUSH);
                    } else if (tempType != TYPE_CHAIN_ANS) {
                        return E_SYNTAX;
                    }
                    
                    if (expr.outputRegister == OutputRegisterHL) {
                        LD_DE_IND_IX_OFF(outputPrevPrev->operand);
                    } else {
                        LD_HL_IND_IX_OFF(outputPrevPrev->operand);
                    }
                    break;
                case TYPE_FUNCTION_RETURN:
                    if (tempType == TYPE_NUMBER) {
                        insertFunctionReturn(outputPrevPrev->operand, OUTPUT_IN_HL, NO_PUSH);
                        LD_DE_IMM(outputPrev->operand);
                    } else if (tempType == TYPE_VARIABLE) {
                        insertFunctionReturn(outputPrevPrev->operand, OUTPUT_IN_HL, NO_PUSH);
                    } else if (tempType == TYPE_FUNCTION) {
                        insertFunctionReturn(outputPrev->operand, OUTPUT_IN_DE, NO_PUSH);
                        insertFunctionReturn(outputPrevPrev->operand, OUTPUT_IN_HL, NEED_PUSH);
                    } else if (tempType == TYPE_CHAIN_ANS) {
                        if (expr.outputRegister == OutputRegisterHL) {
                            insertFunctionReturn(outputPrevPrev->operand, OUTPUT_IN_DE, NEED_PUSH);
                        } else {
                            insertFunctionReturn(outputPrevPrev->operand, OUTPUT_IN_HL, NEED_PUSH);
                        }
                    }
                    break;
                case TYPE_CHAIN_ANS:
                    switch (tempType) {
                        case TYPE_NUMBER:
                            if (expr.outputRegister == OutputRegisterHL) {
                                LD_DE_IMM(outputPrev->operand);
                            } else {
                                LD_HL_NUMBER(outputPrev->operand);
                            }
                        case TYPE_VARIABLE:
                            if (expr.outputRegister == OutputRegisterHL) {
                                LD_DE_IND_IX_OFF(outputPrev->operand);
                            } else {
                                LD_HL_IND_IX_OFF(outputPrev->operand);
                            }
                        case TYPE_FUNCTION_RETURN:
                            if (expr.outputRegister == OutputRegisterHL) {
                                PUSH_HL();
                            } else {
                                PUSH_DE();
                            }
                            insertFunctionReturn(outputPrev->operand, OUTPUT_IN_HL, NO_PUSH);
                            POP_DE();
                            break;
                    }
                    break;
                case TYPE_CHAIN_PUSH:
                    if (tempType != TYPE_CHAIN_ANS) {
                        return E_SYNTAX;
                    }
                    
                    if (expr.outputRegister == OutputRegisterHL) {
                        POP_DE();
                    } else {
                        POP_HL();
                    }
            }
            
            if (function == tMean) {
                ProgramPtrToOffsetStack();
        
                // We need to add the mean routine to the data section
                if (!ice.usedAlreadyMean) {
                    ice.MeanAddr = (uint24_t)ice.programDataPtr;
                    memcpy(ice.programDataPtr, MeanRoutine, 24);
                    ice.programDataPtr += 24;
                    ice.usedAlreadyMean = true;
                }
                CALL(ice.MeanAddr);
            } else {
                OR_A_A();
                SBC_HL_DE();
                ADD_HL_DE();
                if (function == tMin) {
                    JR_C(1);
                } else {
                    JR_NC(1);
                }
                EX_DE_HL();
            }
            break;
        case tSqrt:
            if (tempType == TYPE_VARIABLE) {
                LD_HL_IND_IX_OFF(outputPrev->operand);
            } else if (tempType == TYPE_FUNCTION_RETURN) {
                insertFunctionReturn(outputPrev->operand, OUTPUT_IN_HL, NO_PUSH);
            } else if (tempType != TYPE_CHAIN_ANS) {
                return E_SYNTAX;
            }
            
            ProgramPtrToOffsetStack();
        
            // We need to add the sqrt routine to the data section
            if (!ice.usedAlreadySqrt) {
                ice.SqrtAddr = (uint24_t)ice.programDataPtr;
                memcpy(ice.programDataPtr, SqrtRoutine, 44);
                ice.programDataPtr += 44;
                ice.usedAlreadySqrt = true;
            }
            CALL(ice.SqrtAddr);
            expr.outputRegister2 = OutputRegisterDE;
            ice.modifiedIY = true;
            break;
        case tDet:
            endIndex = index;
            startIndex = index;
            
            // Get all the arguments
            dbg_Debugger();
            for (a = 0; a < amountOfArguments; a++) {
                temp = 0;
                while (1) {
                    outputPrev = &outputPtr[--startIndex];
                    tempType = outputPrev->type;
                    
                    if (tempType == TYPE_C_START) {
                        if (!temp) {
                            break;
                        }
                        temp--;
                    }
                    
                    if (tempType == TYPE_FUNCTION && (uint8_t)outputPrev->operand == tDet) {
                        temp++;
                    }
                    
                    if (tempType == TYPE_ARG_DELIMITER && !temp) {
                        break;
                    }
                }
                
                // Check if it's the first argument or not
                if ((tempType == TYPE_ARG_DELIMITER && a == amountOfArguments - 1) ||
                    (tempType == TYPE_C_START && a != amountOfArguments - 1)) {
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
                
                // And remove the stack
                setStackVar(tempP1, 0);
                setStackVar(tempP2, 1);
                
                // Push the argument
                if (expr.outputRegister == OutputRegisterHL) {
                    PUSH_HL();
                } else {
                    PUSH_DE();
                }
                
                endIndex = startIndex--;
            }
            
            // Invalid first argument of det(
            if (!expr.outputIsNumber) {
                return E_SYNTAX;
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
            
            // Call the function
            CALL(ice.CRoutinesStack[expr.outputNumber]*4 + 0xD00000);
            
            // And pop the arguments
            for (a = 1; a < amountOfArguments; a++) {
                POP_BC();
            }
            
            // Check if the output is 16-bits OR in A
            if (temp & RET_A) {
                OR_A_A();
                SBC_HL_HL();
                LD_L_A();
            }
            
            else if (temp & RET_HL) {
                EX_S_DE_HL();
                expr.outputRegister = OutputRegisterDE;
            }
            break;
        default:
            return E_ICE_ERROR;
    }
    
    expr.outputRegister = expr.outputRegister2;
    
    return VALID;
}

#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include <stdint.h>

#ifndef COMPUTER_ICE
#define w24(x, y) (*(uint24_t*)(x) = y)
#define r24(x) (*(uint24_t*)(x))
#else
void w24(void *x, uint32_t val);
uint32_t r24(void *x);
#endif

#ifndef COMPUTER_ICE
#include <fileioc.h>
#define resetFileOrigin(x) ti_Rewind(x)
#else
#include <stdio.h>
typedef uint32_t uint24_t;
typedef FILE* ti_var_t;
#define resetFileOrigin(x) fseek(x, 0x4A, SEEK_SET)
#endif

#define STACK_SIZE 25

typedef struct {
    char     outName[9];                                    // Output variable name
    char     inName[9];                                     // Input variable name
    
    uint8_t  nestedBlocks;                                  // Amount of nested If/Repeat/While
    uint8_t  *programData;                                  // Address of the program
    uint8_t  programDataData[40000];                        // Address of the program data
    uint8_t  *programPtr;                                   // Pointer to the program
    uint8_t  *programPtrBackup;                             // Same as above
    uint8_t  *programDataPtr;                               // Pointer to the program data
    uint8_t  messageIndex;                                  // Used for displaying messages during compiling
    uint8_t  amountOfCRoutinesUsed;                         // Used for the relocation of C functions at the beginning of the program
    uint8_t  CRoutinesStack[100];                           // The address of the relocation jumps
    uint8_t  tempToken;                                     // Used for functions, i.e. For(, where an argument can stop with either a comma or a parentheses
    uint8_t  stackDepth;                                    // Used for compiling arguments of C functions
    
    uint24_t *dataOffsetStack[500];                         // Stack of the address to point to the data, which needs to be changed after compiling
    uint24_t dataOffsetElements;                            // Amount of stack elements of above
    uint24_t currentLine;                                   // The amount of parsed lines, useful for displaying it when an error occurs
    uint24_t programSize;                                   // Size of the output program
    uint24_t outputElements;                                // 
    uint24_t *stack[STACK_SIZE*5];                          // Stacks for compiling arguments
    uint24_t *stackStart;                                   // Start of the stack
    
    ti_var_t inPrgm;                                        // Used for getting tokens
    ti_var_t outPrgm;                                       // Used for writing bytes
    
    bool     gotName;                                       // Already got the output name
    bool     gotIconDescription;                            // Already got the icon + description
    bool     usedCodeAfterHeader;                           // An icon/description can't be placed after some code
    bool     lastTokenIsReturn;                             // Last token is a "Return", so we can omit our "ret" :)
    bool     modifiedIY;                                    // Some routines modify IY, and some routines needs it
    
    bool     usedAlreadyRand;                               // Only once the "rand" routine in the program data
    uint24_t randAddr;                                      // Address of the "rand" routine in the program data
    
    bool     usedAlreadyGetKeyFast;                         // Only once the "getKey(X)" routine in the program data
    uint24_t getKeyFastAddr;                                // Address of the "getKey(X)" routine in the program data
    
    bool     usedAlreadySqrt;                               // Only once the "sqrt(" routine in the program data
    uint24_t SqrtAddr;                                      // Address of the "sqrt(" routine in the program data
    
    bool     usedAlreadyMean;                               // Only once the "mean(" routine in the program data
    uint24_t MeanAddr;                                      // Address of the "mean(" routine in the program data
#ifdef COMPUTER_ICE
    int      programLength;                                 // Size of input program
#endif
} ice_t;

typedef struct {
    bool     inString;
    bool     inFunction;
    bool     outputIsNumber;
    bool     outputIsString;
    bool     AnsSetZeroFlag;
    bool     AnsSetZeroFlagReversed;
    bool     AnsSetCarryFlag;
    bool     AnsSetCarryFlagReversed;
    
    uint8_t  ZeroCarryFlagRemoveAmountOfBytes;
    uint8_t  outputRegister;
    uint8_t  outputRegister2;
    
    uint24_t outputNumber;
} expr_t;

#define MESSAGE_HEIGHT       10
#define iceMessageLine       ((ice.messageIndex += MESSAGE_HEIGHT) + 3)
#define iceMessageNewLine()  do { (iceMessageLine); } while(0);
    
#define OutputRegisterHL 0
#define OutputRegisterDE 1

extern ice_t ice;
extern expr_t expr;

void preScanProgram(ti_var_t);
void ProgramPtrToOffsetStack(void);
void displayLoadingBarFrame(void);
void displayLoadingBar(ti_var_t);
unsigned int getNextToken(ti_var_t);

#ifndef COMPUTER_ICE
void CHeaderData(void);
void CProgramHeaderData(void);
void OrData(void);
void AndData(void);
void XorData(void);
void RandData(void);
void KeypadData(void);
void StringConcatenateData(void);
void MeanData(void);
void SqrtData(void);
#endif

void setCurrentOffset(int, int, ti_var_t);
unsigned int getCurrentOffset(ti_var_t);

#endif

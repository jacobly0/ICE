#ifndef MAIN_H
#define MAIN_H

#define AMOUNT_OF_GRAPHX_FUNCTIONS 92
#define AMOUNT_OF_FILEIOC_FUNCTIONS 21
#define STACK_SIZE 250
#define SIZEOF_KEYPAD_DATA 18
#define SIZEOF_RAND_DATA   54
#define SIZEOF_SQRT_DATA   43
#define SIZEOF_SINCOS_DATA 99
#define SIZEOF_MEAN_DATA   19
#define SIZEOF_OR_DATA     10
#define SIZEOF_AND_DATA    11
#define SIZEOF_XOR_DATA    13

#define tDefineSprite 0x0A
#define tCall  0x0B
#define tData  0x0C
#define tCopy  0x0D
#define tAlloc 0x0E

typedef struct {
    char     outName[9];                                    // Output variable name
    
    uint8_t  nestedBlocks;                                  // Amount of nested If/Repeat/While
    uint8_t  *programData;                                  // Address of the program
    uint8_t  programDataData[40000];                        // Address of the program data
    uint8_t  *programPtr;                                   // Pointer to the program
    uint8_t  *programPtrBackup;                             // Same as above
    uint8_t  *programDataPtr;                               // Pointer to the program data
    uint8_t  messageIndex;                                  // Used for displaying messages during compiling
    uint8_t  amountOfGraphxRoutinesUsed;                    // Used for the relocation of C functions at the beginning of the program - GRAPHX
    uint8_t  amountOfFileiocRoutinesUsed;                   // Used for the relocation of C functions at the beginning of the program - FILEIOC
    uint8_t  tempToken;                                     // Used for functions, i.e. For(, where an argument can stop with either a comma or a parentheses
    uint8_t  stackDepth;                                    // Used for compiling arguments of C functions
    uint8_t  amountOfOSLocationsUsed;                       // Used for the amount of OS lists and strings that are used
    uint8_t  amountOfLbls;                                  // Amount of Lbl's
    uint8_t  amountOfGotos;                                 // Amount of Goto's
    uint8_t  amountOfVariablesUsed;                         // Amount of used variables (max 85)
    
    uint24_t *dataOffsetStack[500];                         // Stack of the address to point to the data, which needs to be changed after compiling
    uint24_t dataOffsetElements;                            // Amount of stack elements of above
    uint24_t currentLine;                                   // The amount of parsed lines, useful for displaying it when an error occurs
    uint24_t programSize;                                   // Size of the output program
    uint24_t *stack[STACK_SIZE*5];                          // Stacks for compiling arguments
    uint24_t *stackStart;                                   // Start of the stack
    uint24_t LblStack[200];                                 // Label stack
    uint24_t *LblPtr;                                       // Pointer to the label stack
    uint24_t GotoStack[200];                                // Goto stack
    uint24_t *GotoPtr;                                      // Pointer to the goto stack
    uint24_t OSLists[6];                                    // Used to allocate OS lists
    uint24_t OSStrings[10];                                 // Used to allocate OS string
    uint24_t tempStrings[2];                                // Used to allocate temp strings
    uint24_t GraphxRoutinesStack[AMOUNT_OF_GRAPHX_FUNCTIONS];   // The address of the relocation jumps of the GRAPHX lib
    uint24_t FileiocRoutinesStack[AMOUNT_OF_FILEIOC_FUNCTIONS]; // The address of the relocation jumps of the FILEIOC lib
    uint24_t programLength;                                 // Size of input program
    uint24_t freeMemoryPtr;                                 // Pointer to safe RAM (after the OS variables)
    
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
    
    bool     usedAlreadyInput;                              // Only once the "Input" routine in the program data
    uint24_t InputAddr;                                     // Address of the "Input" routine in the program data
    
    bool     usedAlreadyPause;                              // Only once the "Pause " routine in the program data
    uint24_t PauseAddr;                                     // Address of the "Pause " routine in the program data
    
    bool     usedAlreadySinCos;                             // Only once the "sin(" or "cos(" routine in the program data
    uint24_t SinCosAddr;                                    // Address of the "sin(" or "cos(" routine in the program data
} ice_t;

typedef struct {
    bool     inString;
    bool     inFunction;
    bool     outputIsNumber;
    bool     outputIsVariable;
    bool     outputIsString;
    bool     AnsSetZeroFlag;
    bool     AnsSetZeroFlagReversed;
    bool     AnsSetCarryFlag;
    bool     AnsSetCarryFlagReversed;
    
    uint8_t  ZeroCarryFlagRemoveAmountOfBytes;
    uint8_t  outputRegister;
    uint8_t  outputReturnRegister;
    
    uint24_t outputNumber;
} expr_t;

typedef struct {
    char     name[10];
    uint24_t addr;
    uint24_t dataOffsetElements;
    uint8_t  LblGotoElements;
} label_t;

typedef struct {
    uint8_t offset;
    char    name[10];
} variable_t;

#define MESSAGE_HEIGHT       10
#define iceMessageLine       ((ice.messageIndex += MESSAGE_HEIGHT) + 3)
#define iceMessageNewLine()  do { (iceMessageLine); } while(0);

extern ice_t ice;
extern expr_t expr;
extern variable_t variable;

void preScanProgram(void);

#ifndef COMPUTER_ICE
void CHeaderData(void);
void GraphxHeader(void);
void FileiocHeaderData(void);
void CProgramHeaderData(void);
void OrData(void);
void AndData(void);
void XorData(void);
void RandData(void);
void KeypadData(void);
void StringStoData(void);
void InputData(void);
void SinCosData(void);
void PrgmData(void);
void StringConcatenateData(void);
void MeanData(void);
void SqrtData(void);
void PauseData(void);
#endif

#endif

#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <fileioc.h>

typedef struct {
    char     outName[9];
    char     inName[9];
	
    uint8_t  nestedBlocks;
    uint8_t  amountOfUsedCRoutines;
    uint8_t  headerData[800];
    uint8_t  *programData;
    uint8_t  programDataData[40000];
    uint8_t  *headerPtr;
    uint8_t  *programPtr;
    uint8_t  *programDataPtr;
    uint8_t  messageIndex;
    
    uint24_t dataOffsetStack[500];
    uint24_t *dataOffsetPtr;
	
    ti_var_t inPrgm;
    ti_var_t outPrgm;
	
    bool     gotName;
    bool     gotIconDescription;
    bool     usedCodeAfterHeader;
    bool     usedCFunctions;
    bool     exprOutputIsNumber;
    bool     lastTokenIsReturn;
    
    bool     usedAlreadyRand;
    uint24_t randAddr;
    
    bool     usedAlreadyGetKeyFast;
    uint24_t getKeyFastAddr;
} ice_t;

extern ice_t ice;

void CHeaderData(void);
void CProgramHeader(void);
void MultWithNumber(uint24_t number, uint24_t *programPtr);
void AndData(void);
void XorData(void);
void OrData(void);
void RandRoutine(void);

#endif


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
	
    ti_var_t inPrgm;
    ti_var_t outPrgm;
	
    bool     gotName;
    bool     gotIconDescription;
    bool     usedCodeAfterHeader;
    bool     exprOutputIsNumber;
    bool     lastTokenIsReturn;
} ice_t;

extern ice_t ice;

void CHeaderData(void);
void CProgramHeader(void);

#endif


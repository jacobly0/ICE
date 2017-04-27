#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <fileioc.h>

typedef struct {
    char outName[9];
    char inName[9];
	char *headerData;
	char *programData;
	char *programDataData;
    ti_var_t inPrgm;
    ti_var_t outPrgm;
    bool gotName;
    bool gotIconDescrip;
	bool usedCodeAfterHeader;
	uint8_t nestedBlocks;
} ice_t;

extern ice_t ice;

#endif


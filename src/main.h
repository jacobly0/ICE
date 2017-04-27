#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <fileioc.h>

typedef struct {
    char outName[9];
    char inName[9];
    ti_var_t inPrgm;
    ti_var_t outPrgm;
    bool gotName;
    bool gotIconDescrip;
} ice_t;

extern ice_t ice;

#endif


#ifndef DEFINES_H
#define DEFINES_H

#include <stdbool.h>
#include <stdint.h>
#include <tice.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(COMPUTER_ICE) && !defined(SC)

#include <fileioc.h>
#include <graphx.h>
#include <keypadc.h>
#include <debug.h>
#define w24(x, y) (*(uint24_t*)(x) = y)
#define r24(x) (*(uint24_t*)(x))
#define _rewind(x)   ti_Rewind(x)
#define _close(x)    ti_Close(x)
#define _open(x)     ti_OpenVar(x, "r", TI_PRGM_TYPE)
#define _new(x)      ti_OpenVar(x, "w", TI_PPRGM_TYPE)
#define _tell(x)     ti_Tell(x)
#define _seek(x,y,z) ti_Seek(x,y,z)
#define _getc(x)     getNextToken(x)

#else
    
#define _getc(x)     getNextToken(x)
#define TI_PRGM_TYPE 5
#define M_PI         3.14159265358979323846

void w24(void *x, uint32_t val);
uint32_t r24(void *x);

#ifdef COMPUTER_ICE

typedef FILE* ti_var_t;
typedef uint32_t uint24_t;

#define _rewind(x)   fseek(x, 0x4A, SEEK_SET)
#define _open(x)     fopen(x, "r")
#define _tell(x)     ftell(x)
#define _seek(x,y,z) fseek(z,x,y);

void export_program(const char *name, uint8_t *data, size_t size);

#else
    
// Lmao
typedef bool ti_var_t;
typedef uint32_t uint24_t;
    
#define _rewind(x)   ice.progInputPtr = 0
#define _open(x)     ice_open(x)
#define _tell(x)     ice.progInputPtr
#define _close(x)    ice_close()
#define _seek(x,y,z) \
    do { \
        if (y == SEEK_SET) { \
            ice.progInputPtr = x; \
        } else if (y == SEEK_CUR) { \
            ice.progInputPtr += x; \
        } \
    } while (0)
        

bool ice_open(char tempName[9]);
void ice_open_first_prog(void);
void ice_close(void);

// Uh oh...
int main(int argc, char **argv);
    
#endif

#endif

#endif
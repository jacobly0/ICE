#ifndef ROUTINES_H
#define ROUTINES_H

#include <stdbool.h>
#include <stdint.h>

#ifndef COMPUTER_ICE
#include <fileioc.h>
#else
#include <stdio.h>
typedef uint32_t uint24_t;
typedef FILE* ti_var_t;
#endif

void ProgramPtrToOffsetStack(void);
void displayLoadingBarFrame(void);
void displayLoadingBar(void);
uint24_t getNextToken(void);
void MaybeDEToHL(void);
void MaybeHLToDE(void);
void PushHLDE(void);
uint8_t IsHexadecimal(uint24_t);
bool CheckEOL(void);
void setCurrentOffset(uint24_t, uint24_t);
uint24_t getCurrentOffset(void);

#endif

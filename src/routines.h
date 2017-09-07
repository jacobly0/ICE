#ifndef ROUTINES_H
#define ROUTINES_H

void ProgramPtrToOffsetStack(void);
void displayLoadingBarFrame(void);
void displayLoadingBar(ti_var_t);
uint24_t getNextToken(ti_var_t);
void MaybeDEToHL(void);
void MaybeHLToDE(void);
void PushHLDE(void);
uint8_t IsHexadecimal(uint24_t);
bool CheckEOL(void);
void setCurrentOffset(uint24_t, uint24_t);
uint24_t getCurrentOffset(void);

#endif

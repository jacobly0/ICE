#ifndef ROUTINES_H
#define ROUTINES_H

void ProgramPtrToOffsetStack(void);
void displayLoadingBarFrame(void);
void displayLoadingBar(ti_var_t);
uint24_t getNextToken(ti_var_t);
void MaybeDEToHL(void);
void MaybeHLToDE(void);
void PushHLDE(void);
uint8_t IsHexadecimal(int);
bool CheckEOL(void);
uint8_t GetVariableOffset(uint8_t);

#endif

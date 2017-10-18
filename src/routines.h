#ifndef ROUTINES_H
#define ROUTINES_H

void ProgramPtrToOffsetStack(void);
void ProgramDataPtrToOffsetStack(void);
void displayLoadingBarFrame(void);
void SeekMinus1(void);
void displayLoadingBar(void);
int getNextToken(void);
void AnsToHL(void);
void AnsToDE(void);
void displayMessageLineScroll(char*);
void MaybeAToHL(void);
void MaybeLDIYFlags(void);
void CallRoutine(bool*, uint24_t*, const uint8_t*, uint8_t);
uint8_t SquishHexadecimals(uint8_t*);
int grabString(uint8_t**, bool);
void PushHLDE(void);
uint8_t IsHexadecimal(int);
bool CheckEOL(void);
uint8_t GetVariableOffset(uint8_t);

#endif

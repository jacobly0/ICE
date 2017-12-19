#ifndef ROUTINES_H
#define ROUTINES_H

void ProgramPtrToOffsetStack(void);
void ProgramDataPtrToOffsetStack(void);
void displayLoadingBarFrame(void);
void SeekMinus1(void);
void displayLoadingBar(void);
void ClearAnsFlags(void);
void LoadRegValue(uint8_t, uint24_t);
void LoadRegVariable(uint8_t, uint8_t);
void ChangeRegValue(uint24_t, uint24_t, uint8_t opcodes[7]);
void ResetAllRegs(void);
void ResetReg(uint8_t);
void RegChangeHLDE(void);
void PushHLDE(void);
void AnsToHL(void);
void AnsToDE(void);
void AnsToBC(void);
void displayMessageLineScroll(char*);
void MaybeAToHL(void);
void MaybeLDIYFlags(void);
void CallRoutine(bool*, uint24_t*, const uint8_t*, uint8_t);
uint8_t SquishHexadecimals(uint8_t*);
uint8_t IsHexadecimal(int);
uint8_t GetVariableOffset(uint8_t);
bool CheckEOL(void);
int getNextToken(void);
int grabString(uint8_t**, bool);

#endif

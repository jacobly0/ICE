#ifndef ERRORS_H
#define ERRORS_H

#define E_UNIMPLEMENTED    0
#define E_WRONG_PLACE      1
#define E_NO_CONDITION     2
#define E_ELSE             3
#define E_END              4
#define E_EXTRA_RPAREN     5
#define E_SYNTAX           6
#define E_WRONG_ICON       7
#define E_INVALID_HEX      9
#define E_ICE_ERROR        9
#define E_ARGUMENTS        10
#define E_DEPRECATED       11
#define E_UNKNOWN_C        12
#define E_PROG_NOT_FOUND   13
#define E_NO_SUBPROG       14
#define E_NO_FUNC_ALLOW    15

#define E_VALID            254
#define VALID              255

void displayLabelError(char *label);
void displayError(uint8_t);
void PrintChar(char);

#endif

#ifndef ERRORS_H
#define ERRORS_H

#define E_UNIMPLEMENTED    0
#define E_WRONG_PLACE      1
#define E_NO_CONDITION     2
#define E_NO_NESTED_BLOCK  3
#define E_EXTRA_RPAREN     4
#define VALID              255

void displayError(unsigned int index);

#endif


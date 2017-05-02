#ifndef ERRORS_H
#define ERRORS_H

#define E_UNIMPLEMENTED    0
#define E_WRONG_PLACE      1
#define E_NO_CONDITION     2
#define E_NO_NESTED_BLOCK  3
#define E_EXTRA_RPAREN     4
#define E_SYNTAX           5
#define E_WRONG_ICON       6
#define E_INVALID_ICON     7
#define E_ELSE_END         8
#define VALID              255

void displayError(unsigned int index);

#endif


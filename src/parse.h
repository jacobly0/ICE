#ifndef PARSE_H
#define PARSE_H

void parseProgram(void);

extern const void (*functions[256])();
void parseExpression();

#endif


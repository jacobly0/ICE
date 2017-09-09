#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define RET_A         (1<<7)
#define RET_HL        (1<<5)
#define RET_NONE      (0)
#define UN            (1<<6)
#define DEPR          (1<<4)
#define ARG_NORM      (0)
#define SMALL_1       (1<<7)
#define SMALL_2       (1<<6)
#define SMALL_3       (1<<5)
#define SMALL_4       (1<<4)
#define SMALL_5       (1<<3)
#define SMALL_12      (SMALL_1 | SMALL_2)
#define SMALL_123     (SMALL_1 | SMALL_2 | SMALL_3)
#define SMALL_13      (SMALL_1 | SMALL_3)
#define SMALL_23      (SMALL_2 | SMALL_3)
#define SMALL_14      (SMALL_1 | SMALL_4)
#define SMALL_45      (SMALL_4 | SMALL_5)

uint8_t parseFunction(uint24_t);
uint8_t parseFunction1Arg(uint24_t, uint8_t, uint8_t);
uint8_t parseFunction2Args(uint24_t, uint8_t, uint8_t, bool);

#endif

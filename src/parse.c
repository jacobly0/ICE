#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fileioc.h>

#include "parse.h"
#include "errors.h"
#include "main.h"

void parseProgram(void) {
    unsigned int token;

    while ((token = ti_GetC(ice.inPrgm)) != EOF && token != tElse && token != tEnd) {
        (*functions[(uint8_t)token])();
    }
}

/* Static functions */

static void functionI() {
    unsigned int token;

    while ((token = ti_GetC(ice.inPrgm)) != EOF && token != tEnter);
}

static void functionPrgm() {
}

static void functionCustom() {
}

static void functionIf() {
	unsigned int token;
	
	token = ti_GetC(ice.inPrgm);
	parseExpression();
}

static void dummyReturn() {
	return;
}

static void functionWhile() {
}

static void functionRepeat() {
}

static void functionFor() {
}

static void functionReturn() {
}

static void functionLbl() {
}

static void functionGoto() {
}

static void functionPause() {
}

static void functionInput() {
}

static void functionDisp() {
}

static void functionOutput() {
}

static void functionClrHome() {
}

static void parseExpression() {
}

const void (*functions[256])() = {
    nonExistingToken, //0
    nonExistingToken, //1
    nonExistingToken, //2
    nonExistingToken, //3
    tokenWrongPlace,  //4
    nonExistingToken, //5
    nonExistingToken, //6
    nonExistingToken, //7
    parseExpression,  //8
    parseExpression,  //9
    nonExistingToken, //10
    nonExistingToken, //11
    nonExistingToken, //12
    nonExistingToken, //13
    nonExistingToken, //14
    nonExistingToken, //15
    parseExpression,  //16
    parseExpression,  //17
    nonExistingToken, //18
    nonExistingToken, //19
    nonExistingToken, //20
    nonExistingToken, //21
    nonExistingToken, //22
    nonExistingToken, //23
    nonExistingToken, //24
    nonExistingToken, //25
    nonExistingToken, //26
    nonExistingToken, //27
    nonExistingToken, //28
    nonExistingToken, //29
    nonExistingToken, //30
    nonExistingToken, //31
    nonExistingToken, //32
    nonExistingToken, //33
    nonExistingToken, //34
    nonExistingToken, //35
    nonExistingToken, //36
    nonExistingToken, //37
    nonExistingToken, //38
    nonExistingToken, //39
    nonExistingToken, //40
    nonExistingToken, //41
    parseExpression,  //42
    nonExistingToken, //43
    functionI,        //44
    nonExistingToken, //45
    nonExistingToken, //46
    nonExistingToken, //47
    parseExpression,  //48
    parseExpression,  //49
    parseExpression,  //50
    parseExpression,  //51
    parseExpression,  //52
    parseExpression,  //53
    parseExpression,  //54
    parseExpression,  //55
    parseExpression,  //56
    parseExpression,  //57
    nonExistingToken, //58
    nonExistingToken, //59
    tokenWrongPlace,  //60
    tokenWrongPlace,  //61
    nonExistingToken, //62
    dummyReturn,      //63
    tokenWrongPlace,  //64
    parseExpression,  //65
    parseExpression,  //66
    parseExpression,  //67
    parseExpression,  //68
    parseExpression,  //69
    parseExpression,  //70
    parseExpression,  //71
    parseExpression,  //72
    parseExpression,  //73
    parseExpression,  //74
    parseExpression,  //75
    parseExpression,  //76
    parseExpression,  //77
    parseExpression,  //78
    parseExpression,  //79
    parseExpression,  //80
    parseExpression,  //81
    parseExpression,  //82
    parseExpression,  //83
    parseExpression,  //84
    parseExpression,  //85
    parseExpression,  //86
    parseExpression,  //87
    parseExpression,  //88
    parseExpression,  //89
    parseExpression,  //90
    parseExpression,  //91
    nonExistingToken, //92
    parseExpression,  //93
    nonExistingToken, //94
    functionPrgm,     //95
    nonExistingToken, //96
    nonExistingToken, //97
    functionCustom,   //98
    nonExistingToken, //99
    nonExistingToken, //100
    nonExistingToken, //101
    nonExistingToken, //102
    nonExistingToken, //103
    nonExistingToken, //104
    nonExistingToken, //105
    tokenWrongPlace,  //106
    tokenWrongPlace,  //107
    tokenWrongPlace,  //108
    tokenWrongPlace,  //109
    tokenWrongPlace,  //110
    tokenWrongPlace,  //111
    tokenWrongPlace,  //112
    tokenWrongPlace,  //113
    nonExistingToken, //114
    nonExistingToken, //115
    nonExistingToken, //116
    nonExistingToken, //117
    nonExistingToken, //118
    nonExistingToken, //119
    nonExistingToken, //120
    nonExistingToken, //121
    nonExistingToken, //122
    nonExistingToken, //123
    nonExistingToken, //124
    nonExistingToken, //125
    nonExistingToken, //126
    nonExistingToken, //127
    nonExistingToken, //128
    nonExistingToken, //129
    tokenWrongPlace,  //130
    tokenWrongPlace,  //131
    nonExistingToken, //132
    nonExistingToken, //133
    nonExistingToken, //134
    nonExistingToken, //135
    nonExistingToken, //136
    nonExistingToken, //137
    nonExistingToken, //138
    nonExistingToken, //139
    nonExistingToken, //140
    nonExistingToken, //141
    nonExistingToken, //142
    nonExistingToken, //143
    nonExistingToken, //144
    nonExistingToken, //145
    nonExistingToken, //146
    nonExistingToken, //147
    nonExistingToken, //148
    nonExistingToken, //149
    nonExistingToken, //150
    nonExistingToken, //151
    nonExistingToken, //152
    nonExistingToken, //153
    nonExistingToken, //154
    nonExistingToken, //155
    nonExistingToken, //156
    nonExistingToken, //157
    nonExistingToken, //158
    nonExistingToken, //159
    nonExistingToken, //160
    nonExistingToken, //161
    nonExistingToken, //162
    nonExistingToken, //163
    nonExistingToken, //164
    nonExistingToken, //165
    nonExistingToken, //166
    nonExistingToken, //167
    nonExistingToken, //168
    nonExistingToken, //169
    nonExistingToken, //170
    parseExpression,  //171
    nonExistingToken, //172
    parseExpression,  //173
    nonExistingToken, //174
    nonExistingToken, //175
    nonExistingToken, //176
    nonExistingToken, //177
    nonExistingToken, //178
    parseExpression,  //179
    nonExistingToken, //180
    nonExistingToken, //181
    nonExistingToken, //182
    nonExistingToken, //183
    parseExpression,  //184
    nonExistingToken, //185
    nonExistingToken, //186
    nonExistingToken, //187
    parseExpression,  //188
    nonExistingToken, //189
    nonExistingToken, //190
    nonExistingToken, //191
    nonExistingToken, //192
    nonExistingToken, //193
    nonExistingToken, //194
    nonExistingToken, //195
    nonExistingToken, //196
    nonExistingToken, //197
    nonExistingToken, //198
    nonExistingToken, //199
    nonExistingToken, //200
    nonExistingToken, //201
    nonExistingToken, //202
    nonExistingToken, //203
    nonExistingToken, //204
    nonExistingToken, //205
    functionIf,       //206
    nonExistingToken, //207
    dummyReturn,      //208
    functionWhile,    //209
    functionRepeat,   //210
    functionFor,      //211
    dummyReturn,      //212
    functionReturn,   //213
    functionLbl,      //214
    functionGoto,     //215
    functionPause,    //216
    nonExistingToken, //217
    nonExistingToken, //218
    nonExistingToken, //219
    functionInput,    //220
    nonExistingToken, //221
    functionDisp,     //222
    nonExistingToken, //223
    functionOutput,   //224
    functionClrHome,  //225
    nonExistingToken, //226
    nonExistingToken, //227
    nonExistingToken, //228
    nonExistingToken, //229
    nonExistingToken, //230
    nonExistingToken, //231
    nonExistingToken, //232
    nonExistingToken, //233
    nonExistingToken, //234
    nonExistingToken, //235
    nonExistingToken, //236
    nonExistingToken, //237
    nonExistingToken, //238
    nonExistingToken, //239
    nonExistingToken, //240
    nonExistingToken, //241
    nonExistingToken, //242
    nonExistingToken, //243
    nonExistingToken, //244
    nonExistingToken, //245
    nonExistingToken, //246
    nonExistingToken, //247
    nonExistingToken, //248
    nonExistingToken, //249
    nonExistingToken, //250
    nonExistingToken, //251
    nonExistingToken, //252
    nonExistingToken, //253
    nonExistingToken, //254
    nonExistingToken  //255
};
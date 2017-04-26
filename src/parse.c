void parseProgram() {
    uint8_t (*functions[256])() = {
        nonExistingToken, //0
        nonExistingToken, //1
        nonExistingToken, //2
        nonExistingToken, //3
        nonExistingToken, //4
        nonExistingToken, //5
        nonExistingToken, //6
        nonExistingToken, //7
        nonExistingToken, //8
        nonExistingToken, //9
        nonExistingToken, //10
        nonExistingToken, //11
        nonExistingToken, //12
        nonExistingToken, //13
        nonExistingToken, //14
        nonExistingToken, //15
        nonExistingToken, //16
        nonExistingToken, //17
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
        nonExistingToken, //42
        nonExistingToken, //43
        functionI,        //44
        nonExistingToken, //45
        nonExistingToken, //46
        nonExistingToken, //47
        nonExistingToken, //48
        nonExistingToken, //49
        nonExistingToken, //50
        nonExistingToken, //51
        nonExistingToken, //52
        nonExistingToken, //53
        nonExistingToken, //54
        nonExistingToken, //55
        nonExistingToken, //56
        nonExistingToken, //57
        nonExistingToken, //58
        nonExistingToken, //59
        nonExistingToken, //60
        nonExistingToken, //61
        nonExistingToken, //62
        nonExistingToken, //63
        nonExistingToken, //64
        nonExistingToken, //65
        nonExistingToken, //66
        nonExistingToken, //67
        nonExistingToken, //68
        nonExistingToken, //69
        nonExistingToken, //70
        nonExistingToken, //71
        nonExistingToken, //72
        nonExistingToken, //73
        nonExistingToken, //74
        nonExistingToken, //75
        nonExistingToken, //76
        nonExistingToken, //77
        nonExistingToken, //78
        nonExistingToken, //79
        nonExistingToken, //80
        nonExistingToken, //81
        nonExistingToken, //82
        nonExistingToken, //83
        nonExistingToken, //84
        nonExistingToken, //85
        nonExistingToken, //86
        nonExistingToken, //87
        nonExistingToken, //88
        nonExistingToken, //89
        nonExistingToken, //90
        nonExistingToken, //91
        nonExistingToken, //92
        nonExistingToken, //93
        nonExistingToken, //94
        nonExistingToken, //95
        nonExistingToken, //96
        nonExistingToken, //97
        nonExistingToken, //98
        nonExistingToken, //99
        nonExistingToken, //100
        nonExistingToken, //101
        nonExistingToken, //102
        nonExistingToken, //103
        nonExistingToken, //104
        nonExistingToken, //105
        nonExistingToken, //106
        nonExistingToken, //107
        nonExistingToken, //108
        nonExistingToken, //109
        nonExistingToken, //110
        nonExistingToken, //111
        nonExistingToken, //112
        nonExistingToken, //113
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
        nonExistingToken, //130
        nonExistingToken, //131
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
        nonExistingToken, //171
        nonExistingToken, //172
        nonExistingToken, //173
        nonExistingToken, //174
        nonExistingToken, //175
        nonExistingToken, //176
        nonExistingToken, //177
        nonExistingToken, //178
        nonExistingToken, //179
        nonExistingToken, //180
        nonExistingToken, //181
        nonExistingToken, //182
        nonExistingToken, //183
        nonExistingToken, //184
        nonExistingToken, //185
        nonExistingToken, //186
        nonExistingToken, //187
        nonExistingToken, //188
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
        nonExistingToken, //206
        nonExistingToken, //207
        nonExistingToken, //208
        nonExistingToken, //209
        nonExistingToken, //210
        nonExistingToken, //211
        nonExistingToken, //212
        nonExistingToken, //213
        nonExistingToken, //214
        nonExistingToken, //215
        nonExistingToken, //216
        nonExistingToken, //217
        nonExistingToken, //218
        nonExistingToken, //219
        nonExistingToken, //220
        nonExistingToken, //221
        nonExistingToken, //222
        nonExistingToken, //223
        nonExistingToken, //224
        nonExistingToken, //225
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
    }
    
    while ((token = ti_GetC(inputProgram)) + 1) {
        (*functions[token & 0xFF
    }
}

void functionI() {
    while ((token = ti_GetC(inputProgram)) + 1 && token != 0x3F);
}

void functionPrgm() {
}

void functionCustom() {
}

void functionIf() {
}

void dummyReturn() {
}

void functionWhile() {
}

void functionRepeat() {
}

void functionFor() {
}

void functionReturn() {
}

void funtionLbl() {
}

void functionGoto() {
}

void functionPause() {
}

void functionInput() {
}

void functionDisp() {
}

void functionOutput() {
}

void functionClrHome() {
}

void parseExpression() {
}
#include "defines.h"
#include "main.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

uint24_t tempProgInputPtr;
ice_t ice;
reg_t reg;
prescan_t prescan;

bool ice_open(char tempName[9]) {
    // Load the data and reset the pointer
    bool ret = EM_ASM_({
        return compiler.openProg($0, $1, $2, $3);
    }, tempName, ice.progInputData, &ice.programLength, ice.progInputPtr);

    if (ret) {
        ice.progInputPtr = 0;
    }

    return ret;
}

// This function is called first when clicking on 'Compile'
void ice_open_first_prog(void) {
    char *fake_argv[0];

    memset(&ice, 0, sizeof(ice_t));
    memset(&reg, 0, sizeof(reg_t));
    memset(&prescan, 0, sizeof(prescan_t));
    EM_ASM_({
        compiler.openFirstProg($0, $1);
    }, ice.progInputData, &ice.programLength);

    ice.progInputPtr = 0;

    // Call the main function to start compiling!
    main(0, fake_argv);
}

void ice_close(void) {
    EM_ASM_({
        compiler.closeProg($0, $1, $2);
    }, ice.progInputData, &ice.programLength, &ice.progInputPtr);
}

void ice_error(char *error, uint24_t currentLine) {
    EM_ASM_({
        compiler.error($0, $1);
    }, error, currentLine);
}

void ice_export(uint8_t *outputPtr, uint24_t size) {
    EM_ASM_({
        compiler.exportProgram($0, $1, $2);
    }, ice.outName, outputPtr, size);
}

#endif

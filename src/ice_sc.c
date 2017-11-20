#include "defines.h"
#include "main.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#ifdef SC

#include <emscripten.h>

uint24_t tempProgInputPtr;
ice_t ice;

bool ice_open(char tempName[9]) {
    // Load the data and reset the pointer
    bool ret = EM_ASM_({
        return ice_open_js($0, $1, $2);
    }, tempName, ice.progInputData, &ice.programLength);
    
    tempProgInputPtr = ice.progInputPtr;
    ice.progInputPtr = 0;
    
    return ret;
}

// This function is called first when clicking on 'Compile'
void ice_open_first_prog(void) {
    char *fake_argv[0];
    
    memset(&ice, 0, sizeof(ice_t));
    EM_ASM_({
        ice_open_first_prog_js($0, $1);
    }, ice.progInputData, &ice.programLength);
    
    ice.progInputPtr = 0;
    
    // Call the main function to start compiling!
    main(0, fake_argv);
}

void ice_close(void) {
    EM_ASM_({
        ice_close_js($0, $1);
    }, ice.progInputData, &ice.programLength);
}

void ice_error(char *error, uint24_t currentLine) {
    EM_ASM_({
        ice_display_error($0, $1);
    }, error, currentLine);
}

void ice_export(uint8_t *outputPtr, uint24_t size) {
    EM_ASM_({
        ice_export_program($0, $1, $2);
    }, ice.outName, outputPtr, size);
}

#endif
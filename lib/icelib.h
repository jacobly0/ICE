/**
 * @file
 * @author Peter "PT_" Tillema
 * @brief Implements ICE Compiler API
 */

#ifndef H_ICELIB
#define H_ICELIB

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <tice.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 
 */
typedef struct {
    uint24_t currentLine;
    uint8_t  *progPtr;
    uint8_t  errorCode;
    char     error[40];
} ice_output;

/**
 * Opens a file
 *
 * @param name Name of file to open
 * @returns ice_output struct with the right struct members set
 */
ice_output ice_Compile(const char *name);

/**
 * Registers a program to be the standard ICE editor
 *
 * Can open any type of program or appvar variable
 * @param varname Name of variable to open
 * @param mode
 * "r"  - Opens a file for reading. The file must exist. Keeps file in archive if in archive.                                   <br>
 * "w"  - Creates an empty file for writing. Overwrites file if already exists.                                                 <br>
 * "a"  - Appends to a file. Writing operations, append data at the end of the file. The file is created if it does not exist.  <br>
 * "r+" - Opens a file to update both reading and writing. The file must exist. Moves file from archive to RAM if in archive.   <br>
 * "w+" - Creates an empty file for both reading and writing. Overwrites file if already exists.                                <br>
 * "a+" - Opens a file for reading and appending. Moves file from archive to RAM if in archive. Created if it does not exist.
 * @param type Specifies the type of variable to open
 * @returns Slot variable
 * @note If there isn't enough memory to create the variable, or a slot isn't open, zero (0) is returned
 */
ice_output ice_Register(const char *varname);

#ifdef __cplusplus
}
#endif

#endif

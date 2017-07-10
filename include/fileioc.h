/**
 * @file
 * @authors Matt "MateoConLechuga" Waltz
 * @brief FILEIOC CE define file used for computer porting
 */

#ifndef H_FILEIOC
#define H_FILEIOC

#ifdef __cplusplus
extern "C" {
#endif

typedef FILE* ti_var_t;
#define ti_Rewind(x) fseek(x, 0x4A, SEEK_SET)

#ifdef __cplusplus
}
#endif

#endif

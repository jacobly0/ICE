#ifndef OUTPUT_H
#define OUTPUT_H

#define PRGM_START 0xD1A881
#define _GetCSC    0x02014C
#define __imuls    0x000154
#define __idvrmu   0x000144

#define OP_ADD_HL_DE  0x19
#define OP_INC_HL     0x23
#define OP_ADD_HL_HL  0x29
#define OP_DEC_HL     0x2B
#define OP_CCF        0x3F
#define OP_LD_C_A     0x4F
#define OP_LD_E_A     0x5F
#define OP_LD_L_A     0x6F
#define OP_OR_A_A     0xB7
#define OP_JP         0xC3
#define OP_RET        0xC9
#define OP_CALL       0xCD
#define OP_POP_BC     0xC1
#define OP_PUSH_BC    0xC5
#define OP_POP_DE     0xD1
#define OP_PUSH_DE    0xD5
#define OP_POP_HL     0xE1
#define OP_PUSH_HL    0xE5
#define OP_EX_DE_HL   0xEB

#define output(type, value)   do { *(type*)ice.programPtr = (value); ice.programPtr += sizeof(type); } while (0)

#define LD_BC_IND_IX_OFF(off) do { output(uint16_t, 0x07DD); output(uint8_t, off * 3); } while (0)
#define LD_DE_IND_IX_OFF(off) do { output(uint16_t, 0x17DD); output(uint8_t, off * 3); } while (0)
#define LD_HL_IND_IX_OFF(off) do { output(uint16_t, 0x27DD); output(uint8_t, off * 3); } while (0)
#define LD_IX_OFF_IND_HL(off) do { output(uint16_t, 0x2FDD); output(uint8_t, off * 3); } while (0)

#define LD_A_IMM(val)         do { output(uint8_t, 0x3E); output(uint8_t, val); } while (0)
#define LD_BC_IMM(val)        do { output(uint8_t, 0x01); output(uint24_t, val); } while (0)
#define LD_DE_IMM(val)        do { output(uint8_t, 0x11); output(uint24_t, val); } while (0)
#define LD_HL_IMM(val)        do { output(uint8_t, 0x21); output(uint24_t, val); } while (0)
    
#define LD_C_A()              do { output(uint8_t, OP_LD_C_A); } while (0)
#define LD_E_A()              do { output(uint8_t, OP_LD_E_A); } while (0)
#define LD_L_A()              do { output(uint8_t, OP_LD_L_A); } while (0)

#define INC_HL()              do { output(uint8_t, OP_INC_HL); } while (0)
#define DEC_HL()              do { output(uint8_t, OP_DEC_HL); } while (0)
#define ADD_HL_DE()           do { output(uint8_t, OP_ADD_HL_DE); } while (0)
#define ADD_HL_HL()           do { output(uint8_t, OP_ADD_HL_HL); } while (0)
#define SBC_HL_DE()           do { output(uint16_t, 0x52ED); } while (0)
#define SBC_HL_HL()           do { output(uint16_t, 0x62ED); } while (0)
    
#define PUSH_BC()             do { output(uint8_t, OP_PUSH_BC); } while (0)
#define PUSH_DE()             do { output(uint8_t, OP_PUSH_DE); } while (0)
#define PUSH_HL()             do { output(uint8_t, OP_PUSH_HL); } while (0)
#define POP_BC()              do { output(uint8_t, OP_POP_BC); } while (0)
#define POP_DE()              do { output(uint8_t, OP_POP_DE); } while (0)
#define POP_HL()              do { output(uint8_t, OP_POP_HL); } while (0)

#define EX_DE_HL()            do { output(uint8_t, OP_EX_DE_HL); } while (0)

#define RET()                 do { output(uint8_t, OP_RET); } while (0)
#define JP(val)               do { output(uint8_t, OP_JP); output(uint24_t, val); } while (0)
#define CALL(val)             do { output(uint8_t, OP_CALL); output(uint24_t, val); } while (0)
#define CALL_IMULS()          do { CALL(0x000154); } while (0)

#define OR_A_A()              do { output(uint8_t, OP_OR_A_A); } while (0)
    
#define CCF()                 do { output(uint8_t, OP_CCF); } while (0)

#endif


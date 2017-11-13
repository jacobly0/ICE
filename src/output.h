#ifndef OUTPUT_H
#define OUTPUT_H

#include "main.h"

#define PRGM_START        0xD1A882
#define flags             0xD00080
#define curRow            0xD00595
#define curCol            0xD00596
#define OP1               0xD005F8
#define pixelShadow       0xD031F6
                       
#define _GetCSC           0x02014C
#define _Mov9ToOP1        0x020320
#define _PutS             0x0207C0
#define _NewLine          0x0207F0
#define _ClrLCDFull       0x020808
#define _HomeUp           0x020828
#define _RunIndicOff      0x020848
#define _ParseInp         0x020F00
#define _DrawStatusBar    0x021A3C
#define _os_GetCSC        0x021D3C
#define _SetHLUTo0        0x021D8C
#define _DispHL           0x021EE0

#define __strcat          0x0000C0
#define __strcpy          0x0000CC
#define __strlen          0x0000D4
#define __iand            0x000134
#define __idvrmu          0x000144
#define __imul_b          0x000150
#define __imuls           0x000154
#define __ineg            0x000160
#define __ior             0x000168
#define __ixor            0x000198

#define OP_LD_BC      0x01
#define OP_LD_B       0x06
#define OP_LD_C       0x0E
#define OP_DJNZ       0x10
#define OP_LD_DE      0x11
#define OP_INC_DE     0x13
#define OP_INC_D      0x14
#define OP_DEC_D      0x15
#define OP_LD_D       0x16
#define OP_JR         0x18
#define OP_ADD_HL_DE  0x19
#define OP_LD_A_DE    0x1A
#define OP_DEC_DE     0x1B
#define OP_LD_E       0x1E
#define OP_JR_NZ      0x20
#define OP_LD_HL      0x21
#define OP_INC_HL     0x23
#define OP_INC_H      0x24
#define OP_DEC_H      0x25
#define OP_LD_H       0x26
#define OP_JR_Z       0x28
#define OP_ADD_HL_HL  0x29
#define OP_DEC_HL     0x2B
#define OP_LD_L       0x2E
#define OP_JR_NC      0x30
#define OP_LD_IMM_A   0x32
#define OP_SCF        0x37
#define OP_JR_C       0x38
#define OP_INC_A      0x3C
#define OP_DEC_A      0x3D
#define OP_LD_A       0x3E
#define OP_CCF        0x3F
#define OP_LD_B_A     0x47
#define OP_LD_C_A     0x4F
#define OP_LD_D_A     0x57
#define OP_LD_E_A     0x5F
#define OP_LD_L_A     0x6F
#define OP_LD_HL_D    0x72
#define OP_LD_HL_E    0x73
#define OP_LD_HL_A    0x77
#define OP_LD_A_B     0x78
#define OP_LD_A_D     0x7A
#define OP_LD_A_E     0x7B
#define OP_LD_A_L     0x7D
#define OP_LD_A_HL    0x7E
#define OP_ADD_A_L    0x85
#define OP_ADD_A_A    0x87
#define OP_SUB_A_D    0x92
#define OP_SBC_A_A    0x9F
#define OP_AND_A_L    0xA5
#define OP_XOR_A_L    0xAD
#define OP_XOR_A_A    0xAF
#define OP_OR_A_C     0xB1
#define OP_OR_A_D     0xB2
#define OP_OR_A_A     0xB7
#define OP_RET_NZ     0xC0
#define OP_POP_BC     0xC1
#define OP_JP_NZ      0xC2
#define OP_JP         0xC3
#define OP_PUSH_BC    0xC5
#define OP_ADD_A      0xC6
#define OP_RET_Z      0xC8
#define OP_RET        0xC9
#define OP_CALL       0xCD
#define OP_JP_Z       0xCA
#define OP_RET_NC     0xD0
#define OP_POP_DE     0xD1
#define OP_JP_NC      0xD2
#define OP_PUSH_DE    0xD5
#define OP_SUB_A      0xD6
#define OP_RET_C      0xD8
#define OP_JP_C       0xDA
#define OP_POP_HL     0xE1
#define OP_EX_SP_HL   0xE3
#define OP_PUSH_HL    0xE5
#define OP_AND_A      0xE6
#define OP_EX_DE_HL   0xEB
#define OP_XOR_A      0xEE
#define OP_OR_A       0xF6
#define OP_CP_A       0xFE

#ifndef COMPUTER_ICE
#define output(type, value) \
    do { \
        *(type*)ice.programPtr = (value); \
        ice.programPtr += sizeof(type); \
    } while (0)
#else
#define output(type, value) \
    do { \
        if (sizeof(type) == sizeof(uint24_t)) { \
            w24(ice.programPtr, (value)); \
            ice.programPtr += 3; \
        } else { \
            *(type*)ice.programPtr = (value); \
            ice.programPtr += sizeof(type); \
        } \
    } while (0)
#endif

#define LD_A_IND_IX_OFF(off)  do { LoadRegVariable(REGISTER_A, off); } while (0)
#define LD_BC_IND_IX_OFF(off) do { LoadRegVariable(REGISTER_BC, off); } while (0)
#define LD_DE_IND_IX_OFF(off) do { LoadRegVariable(REGISTER_DE, off); } while (0)
#define LD_HL_IND_IX_OFF(off) do { LoadRegVariable(REGISTER_HL, off); } while (0)
#define LD_IY_IND_IX_OFF(off) do { output(uint16_t, 0x31DD); output(uint8_t, off); } while (0)
#define LD_IX_OFF_IND_DE(off) do { output(uint16_t, 0x1FDD); output(uint8_t, off); reg.DEIsVariable = true; reg.DEVariable = off; } while (0)
#define LD_IX_OFF_IND_HL(off) do { output(uint16_t, 0x2FDD); output(uint8_t, off); reg.HLIsVariable = true; reg.HLVariable = off; } while (0)
#define LD_IX_IMM(val)        do { output(uint16_t, 0x21DD); output(uint24_t, val); } while (0)
#define LD_IY_IMM(val)        do { output(uint16_t, 0x21FD); output(uint24_t, val); } while (0)
#define LEA_HL_IY_OFF(off)    do { output(uint16_t, 0x23ED); output(uint8_t, off); ResetReg(REGISTER_HL); } while (0)

#define LD_BC_IMM(val)        do { LoadRegValue(REGISTER_BC, val); } while (0)
#define LD_DE_IMM(val)        do { LoadRegValue(REGISTER_DE, val); } while (0)
#define LD_HL_IMM(val)        do { LoadRegValue(REGISTER_HL, val); } while (0)
#define LD_IMM_A(val)         do { output(uint8_t, OP_LD_IMM_A); output(uint24_t, val); } while (0)
#define LD_SIS_IMM_HL(val)    do { output(uint16_t, 0x2240); output(uint16_t, val); } while (0)
#define LD_SIS_HL(val)        do { output(uint16_t, 0x2140); output(uint16_t, val); } while (0)
#define LD_HL_ADDR(val)       do { output(uint8_t, 0x2A); output(uint24_t, val); ResetReg(REGISTER_HL); } while (0)
#define LD_ADDR_HL(val)       do { output(uint8_t, 0x22); output(uint24_t, val); } while (0)
#define LD_ADDR_DE(val)       do { output(uint16_t, 0x53ED); output(uint24_t, val); } while (0)
#define LD_A_ADDR(val)        do { output(uint8_t, 0x3A); output(uint24_t, val); ResetReg(REGISTER_A); } while (0)
#define LD_ADDR_A(val)        do { output(uint8_t, 0x32); output(uint24_t, val); } while (0)
#define LD_HL_HL()            do { output(uint16_t, 0x27ED); ResetReg(REGISTER_HL); } while (0)
#define LD_HL_DE()            do { output(uint16_t, 0x1FED); ResetReg(REGISTER_HL); } while (0)
#define LD_HL_VAL(val)        do { output(uint16_t, 0x36 + val * 256); } while (0)

#define LD_A(val)             do { output(uint16_t, val * 256 + OP_LD_A); reg.AIsNumber = true; reg.AIsVariable = false; reg.AValue = val; } while (0)
#define LD_B(val)             do { output(uint8_t, OP_LD_B); output(uint8_t, val); ResetReg(REGISTER_BC); } while (0)
#define LD_C(val)             do { output(uint8_t, OP_LD_C); output(uint8_t, val); ResetReg(REGISTER_BC); } while (0)
#define LD_H(val)             do { output(uint8_t, OP_LD_H); output(uint8_t, val); ResetReg(REGISTER_HL); } while (0)
#define LD_L(val)             do { output(uint8_t, OP_LD_L); output(uint8_t, val); ResetReg(REGISTER_HL); } while (0)
#define LD_HL_A()             do { output(uint8_t, OP_LD_HL_A); } while (0)
#define LD_HL_D()             do { output(uint8_t, OP_LD_HL_D); } while (0)
#define LD_HL_E()             do { output(uint8_t, OP_LD_HL_E); } while (0)
#define LD_A_DE()             do { output(uint8_t, OP_LD_A_DE); ResetReg(REGISTER_A); } while (0)
#define LD_A_HL()             do { output(uint8_t, OP_LD_A_HL); ResetReg(REGISTER_A); } while (0)
#define LD_B_A()              do { output(uint8_t, OP_LD_B_A); ResetReg(REGISTER_BC); } while (0)
#define LD_C_A()              do { output(uint8_t, OP_LD_C_A); ResetReg(REGISTER_BC); } while (0)
#define LD_D_A()              do { output(uint8_t, OP_LD_D_A); ResetReg(REGISTER_DE); } while (0)
#define LD_E_A()              do { output(uint8_t, OP_LD_E_A); ResetReg(REGISTER_DE); } while (0)
#define LD_L_A()              do { output(uint8_t, OP_LD_L_A); ResetReg(REGISTER_HL); } while (0)
#define LD_A_B()              do { output(uint8_t, OP_LD_A_B); ResetReg(REGISTER_A); } while (0)
#define LD_A_D()              do { output(uint8_t, OP_LD_A_D); ResetReg(REGISTER_A); } while (0)
#define LD_A_E()              do { output(uint8_t, OP_LD_A_E); ResetReg(REGISTER_A); } while (0)
#define LD_A_L()              do { output(uint8_t, OP_LD_A_L); ResetReg(REGISTER_A); } while (0)
#define INC_H()               do { output(uint8_t, OP_INC_H); ResetReg(REGISTER_HL); } while (0)
#define DEC_H()               do { output(uint8_t, OP_DEC_H); ResetReg(REGISTER_HL); } while (0)
#define INC_D()               do { output(uint8_t, OP_INC_D); ResetReg(REGISTER_HL); } while (0)
#define DEC_D()               do { output(uint8_t, OP_DEC_D); ResetReg(REGISTER_HL); } while (0)

#define INC_DE()              do { output(uint8_t, OP_INC_DE); ResetReg(REGISTER_DE); } while (0)
#define INC_HL()              do { output(uint8_t, OP_INC_HL); ResetReg(REGISTER_HL); } while (0)
#define DEC_DE()              do { output(uint8_t, OP_DEC_DE); ResetReg(REGISTER_DE); } while (0)
#define DEC_HL()              do { output(uint8_t, OP_DEC_HL); ResetReg(REGISTER_HL); } while (0)
#define ADD_HL_DE()           do { output(uint8_t, OP_ADD_HL_DE); ResetReg(REGISTER_HL); } while (0)
#define ADD_HL_HL()           do { output(uint8_t, OP_ADD_HL_HL); ResetReg(REGISTER_HL); } while (0)
#define SBC_HL_DE()           do { output(uint16_t, 0x52ED); ResetReg(REGISTER_HL); } while (0)
#define SBC_HL_HL()           do { output(uint16_t, 0x62ED); ResetReg(REGISTER_HL); } while (0)
#define MLT_HL()              do { output(uint16_t, 0x6CED); ResetReg(REGISTER_HL); } while (0)
    
#define OR_A_SBC_HL_DE()      do { output(uint24_t, 0x52EDB7); ResetReg(REGISTER_HL); } while (0)
#define SBC_HL_HL_INC_HL()    do { output(uint24_t, 0x2362ED); ResetReg(REGISTER_HL); } while (0)
    
#define PUSH_BC()             do { output(uint8_t, OP_PUSH_BC); } while (0)
#define PUSH_DE()             do { output(uint8_t, OP_PUSH_DE); } while (0)
#define PUSH_HL()             do { output(uint8_t, OP_PUSH_HL); } while (0)
#define POP_BC()              do { output(uint8_t, OP_POP_BC); ResetReg(REGISTER_BC); } while (0)
#define POP_DE()              do { output(uint8_t, OP_POP_DE); ResetReg(REGISTER_DE); } while (0)
#define POP_HL()              do { output(uint8_t, OP_POP_HL); ResetReg(REGISTER_HL); } while (0)

#define EX_DE_HL()            do { output(uint8_t, OP_EX_DE_HL); RegChangeHLDE(); } while (0)
#define EX_S_DE_HL()          do { output(uint16_t, 0xEB52); RegChangeHLDE(); } while (0)
#define EX_SP_HL()            do { output(uint8_t, OP_EX_SP_HL); ResetReg(REGISTER_HL); } while (0)

#define RET()                 do { output(uint8_t, OP_RET); } while (0)
#define RET_Z()               do { output(uint8_t, OP_RET_Z); } while (0)
#define RET_NZ()              do { output(uint8_t, OP_RET_NZ); } while (0)
#define RET_C()               do { output(uint8_t, OP_RET_C); } while (0)
#define RET_NC()              do { output(uint8_t, OP_RET_NC); } while (0)
#define JP(addr)              do { output(uint8_t, OP_JP); output(uint24_t, addr); } while (0)
#define JP_Z(addr)            do { output(uint8_t, OP_JP_Z); output(uint24_t, addr); } while (0)
#define JP_NZ(addr)           do { output(uint8_t, OP_JP_NZ); output(uint24_t, addr); } while (0)
#define JP_C(addr)            do { output(uint8_t, OP_JP_C); output(uint24_t, addr); } while (0)
#define JP_NC(addr)           do { output(uint8_t, OP_JP_NC); output(uint24_t, addr); } while (0)
#define CALL(addr)            do { output(uint8_t, OP_CALL); output(uint24_t, addr); } while (0)
#define JR_NZ(off)            do { output(uint8_t, OP_JR_NZ); output(uint8_t, off); } while (0)
#define JR_Z(off)             do { output(uint8_t, OP_JR_Z); output(uint8_t, off); } while (0)
#define JR_NC(off)            do { output(uint8_t, OP_JR_NC); output(uint8_t, off); } while (0)
#define JR_C(off)             do { output(uint8_t, OP_JR_C); output(uint8_t, off); } while (0)
#define DJNZ(off)             do { output(uint8_t, OP_DJNZ); output(uint8_t, off); ResetReg(REGISTER_BC); } while (0)
    
#define LDIR()                do { output(uint16_t, 0xB0ED); reg.BCIsNumber = true; reg.BCIsVariable = false; reg.BCValue = 0; ResetReg(REGISTER_HL); ResetReg(REGISTER_DE); } while (0)
#define LDDR()                do { output(uint16_t, 0xB8ED); reg.BCIsNumber = true; reg.BCIsVariable = false; reg.BCValue = 0; ResetReg(REGISTER_HL); ResetReg(REGISTER_DE); } while (0)

#define OR_A_A()              do { output(uint8_t, OP_OR_A_A); } while (0)
#define OR_A_C()              do { output(uint8_t, OP_OR_A_C); ResetReg(REGISTER_A); } while (0)
#define XOR_A_A()             do { output(uint8_t, OP_XOR_A_A); reg.AIsNumber = true; reg.AIsVariable = false; reg.AValue = 0; } while (0)
#define AND_A(val)            do { output(uint8_t, OP_AND_A); output(uint8_t, val); ResetReg(REGISTER_A); } while (0)
#define CP_A(val)             do { output(uint8_t, OP_CP_A); output(uint8_t, val); } while (0)
#define CCF()                 do { output(uint8_t, OP_CCF); } while (0)
#define SCF()                 do { output(uint8_t, OP_SCF); } while (0)
#define SRL_A()               do { output(uint16_t, 0x3FCB); ResetReg(REGISTER_A); } while (0)
#define SUB_A(val)            do { output(uint8_t, OP_SUB_A); output(uint8_t, val); ResetReg(REGISTER_A); } while (0)
#define SUB_A_D()             do { output(uint8_t, OP_SUB_A_D); ResetReg(REGISTER_A); } while (0)
#define ADD_A_A()             do { output(uint8_t, OP_ADD_A_A); ResetReg(REGISTER_A); } while (0)
#define ADD_A_L()             do { output(uint8_t, OP_ADD_A_L); ResetReg(REGISTER_A); } while (0)
#define ADD_A(val)            do { output(uint8_t, OP_ADD_A); output(uint8_t, val); ResetReg(REGISTER_A); } while (0)
#define SBC_A_A()             do { output(uint8_t, OP_SBC_A_A); ResetReg(REGISTER_A); } while (0)
#define INC_A()               do { output(uint8_t, OP_INC_A); ResetReg(REGISTER_A); } while (0)
#define DEC_A()               do { output(uint8_t, OP_DEC_A); ResetReg(REGISTER_A); } while (0)
#define XOR_A(val)            do { output(uint8_t, OP_XOR_A); output(uint8_t, val); ResetReg(REGISTER_A); } while (0)
#define OR_A(val)             do { output(uint8_t, OP_OR_A); output(uint8_t, val); ResetReg(REGISTER_A); } while (0)

#endif

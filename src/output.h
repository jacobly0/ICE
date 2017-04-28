#ifndef OUTPUT_H
#define OUTPUT_H

#define output(type, value)   do { *(type*)ice.programPtr = (value); ice.programPtr += sizeof(type); } while (0)

#define LD_DE_IND_IX_OFF(off) do { output(uint16_t, 0x1FDD); output(uint8_t, off); } while (0)
#define LD_HL_IND_IX_OFF(off) do { output(uint16_t, 0x2FDD); output(uint8_t, off); } while (0)
#define LD_A_IMM(imm)         do { output(uint8_t, 0x3E); output(uint8_t, imm); } while (0)
#define LD_DE_IMM(val)        do { output(uint8_t, 0x11); output(uint24_t, val); } while (0)
#define LD_HL_IMM(val)        do { output(uint8_t, 0x21); output(uint24_t, val); } while (0)
#define INC_HL()              do { output(uint8_t, 0x23); } while (0)
#define ADD_HL_DE()           do { output(uint8_t, 0x19); } while (0)
#define ADD_HL_HL()           do { output(uint8_t, 0x29); } while (0)
#define POP_DE()              do { output(uint8_t, 0xD1); } while (0)
#define POP_HL()              do { output(uint8_t, 0xE1); } while (0)
    
#endif
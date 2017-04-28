#ifndef OUTPUT_H
#define OUTPUT_H

#define output(type, value) *(type*)ice.programPtr = (value); ice.programPtr += sizeof(type)

#define LD_HL_IND_IX_OFF(off) do { output(uint16_t, 0x2FDD); output(uint8_t, off); } while (0)
#define LD_IX_OFF_IND_HL(off) do { output(uint16_t, 0x27DD); output(uint8_t, off); } while (0)
#define LD_A_IMM(imm)         do { output(uint8_t, 0x3E); output(uint8_t, imm); } while (0)
#define LD_HL_IMM(val)        do { output(uint8_t, 0x21); output(uint24_t, val); } while (0)
#define INC_HL()              do { output(uint8_t, 0x23);  } while (0)
    
#endif
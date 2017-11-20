#include <stdint.h>

#ifdef SC
#include "defines.h"

const uint8_t AndData[] = {0x19, 0x3F, 0x30, 0x04, 0x1B, 0x2B, 0xED, 0x52, 0xED, 0x62, 0x23};
const uint8_t CheaderData[] = {0x21, 0xD6, 0xA8, 0xD1, 0xCD, 0x20, 0x03, 0x02, 0x3E, 0x15, 0x32, 0xF8, 0x05, 0xD0, 0xCD, 0x0C, 0x05, 0x02, 0x38, 0x24, 0xCD, 0x98, 0x1F, 0x02, 0x20, 0x0E, 0xCD, 0x28, 0x06, 0x02, 0xCD, 0x48, 0x14, 0x02, 0xCD, 0xC4, 0x05, 0x02, 0x18, 0xE6, 0xEB, 0x11, 0x09, 0x00, 0x00, 0x19, 0x5E, 0x19, 0x23, 0x23, 0x23, 0x11, 0xED, 0xA8, 0xD1, 0xE9, 0xCD, 0x14, 0x08, 0x02, 0xCD, 0x28, 0x08, 0x02, 0x21, 0xD2, 0xA8, 0xD1, 0xCD, 0xC0, 0x07, 0x02, 0xCD, 0xF0, 0x07, 0x02, 0xC3, 0xC0, 0x07, 0x02, 0x4E, 0x65, 0x65, 0x64, 0x20, 0x4C, 0x69, 0x62, 0x4C, 0x6F, 0x61, 0x64, 0x00, 0x74, 0x69, 0x6E, 0x79, 0x2E, 0x63, 0x63, 0x2F, 0x63, 0x6C, 0x69, 0x62, 0x73, 0x00, 0xC0, 0x47, 0x52, 0x41, 0x50, 0x48, 0x58, 0x00, 0x07};
const uint8_t EditorData[] = {0xFD, 0x21, 0x80, 0x00, 0xD0, 0xCD, 0x3C, 0x02, 0x3E, 0x15, 0x32, 0xF8, 0x05, 0xD0, 0xCD, 0x0C, 0x05, 0x02, 0x38, 0x24, 0xCD, 0x98, 0x1F, 0x02, 0x20, 0x0E, 0xCD, 0x28, 0x06, 0x02, 0xCD, 0x48, 0x14, 0x02, 0xCD, 0xC4, 0x05, 0x02, 0x18, 0xE6, 0xEB, 0x11, 0x09, 0x00, 0x00, 0x19, 0x5E, 0x19, 0x23, 0x23, 0x23, 0x11, 0xED, 0xA8, 0xD1, 0xE9, 0xCD, 0x14, 0x08, 0x02, 0xCD, 0x28, 0x08, 0x02, 0x21, 0xD2, 0xA8, 0xD1, 0xCD, 0xC0, 0x07, 0x02, 0xCD, 0xF0, 0x07, 0x02, 0xC3, 0xC0, 0x07, 0x02, 0x4E, 0x65, 0x65, 0x64, 0x20, 0x4C, 0x69, 0x62, 0x4C, 0x6F, 0x61, 0x64, 0x00, 0x74, 0x69, 0x6E, 0x79, 0x2E, 0x63, 0x63, 0x2F, 0x63, 0x6C, 0x69, 0x62, 0x73, 0x00, 0xC0, 0x47, 0x52, 0x41, 0x50, 0x48, 0x58, 0x00, 0x07, 0x06, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0xF4, 0x70, 0x30, 0x00, 0xE0, 0x25, 0x66, 0x00, 0x10, 0x00, 0x00, 0x00, 0x88, 0x02, 0x66, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0x6F, 0x67, 0x72, 0xB8, 0x3A, 0x66, 0x00, 0xE0, 0x25, 0x66, 0x00, 0xE0, 0x25, 0x66, 0x00, 0x01, 0x00, 0x00, 0x00, 0xE8, 0x25, 0x66, 0x00, 0x00};
const uint8_t FileiocheaderData[] = {0xC0, 0x46, 0x49, 0x4C, 0x45, 0x49, 0x4F, 0x43, 0x00, 0x03};
const uint8_t InputData[] = {0xCD, 0x14, 0x08, 0x02, 0xCD, 0x28, 0x08, 0x02, 0xAF, 0x32, 0x79, 0x08, 0xD0, 0x32, 0x99, 0x05, 0xD0, 0xFD, 0x46, 0x09, 0xFD, 0x4E, 0x1C, 0xFD, 0xCB, 0x1C, 0xB6, 0xFD, 0xCB, 0x09, 0xFE, 0xC5, 0xCD, 0x20, 0x13, 0x02, 0xC1, 0xCB, 0xA0, 0xFD, 0x70, 0x09, 0xFD, 0x71, 0x1C, 0x2A, 0x4E, 0x24, 0xD0, 0xCD, 0xE8, 0x0A, 0x02, 0xCD, 0x0C, 0x05, 0x02, 0x14, 0x08, 0x02, 0xCD, 0x28, 0x08, 0x02, 0x21, 0xD2, 0xA8, 0xD1, 0xCD, 0xC0, 0x07, 0x02, 0xCD, 0xF0, 0x07, 0x02, 0xC3, 0xC0, 0x07, 0x02, 0x4E, 0x65, 0x65, 0x64, 0x20, 0x4C, 0x69, 0x62, 0x4C, 0x6F};
const uint8_t KeypadData[] = {0xF3, 0x21, 0x00, 0x02, 0xF5, 0x74, 0xAF, 0xBE, 0x20, 0xFD, 0x68, 0x7E, 0xED, 0x62, 0xA1, 0xC8, 0x2C, 0xC9};
const uint8_t LoadspriteData[] = {0xCD, 0x20, 0x03, 0x02, 0x3E, 0x15, 0x32, 0xF8, 0x05, 0xD0, 0xCD, 0x0C, 0x05, 0x02, 0x3F, 0xED, 0x62, 0xD0, 0xCD, 0x98, 0x1F, 0x02, 0xDC, 0x48, 0x14, 0x02, 0x21, 0x00, 0x00, 0x00, 0x19, 0xC9};
const uint8_t LoadtilemapData[] = {0xCD, 0x20, 0x03, 0x02, 0x3E, 0x15, 0x32, 0xF8, 0x05, 0xD0, 0xCD, 0x0C, 0x05, 0x02, 0x3F, 0xED, 0x62, 0xD0, 0xCD, 0x98, 0x1F, 0x02, 0xDC, 0x48, 0x14, 0x02, 0x21, 0x00, 0x00, 0x00, 0x19, 0x4E, 0x23, 0x46, 0x2B, 0xED, 0x4C, 0x03, 0x03, 0x11, 0x00, 0x00, 0x00, 0xD5, 0x3E, 0x00, 0xEB, 0xED, 0x1F, 0x23, 0x23, 0x23, 0xEB, 0x09, 0x3D, 0x20, 0xF5, 0xE1, 0xC9};
const uint8_t MallocData[] = {0x11, 0x00, 0x00, 0x00, 0x19, 0x22, 0x00, 0x00, 0x00, 0x01, 0xC5, 0x3E, 0xD1, 0xB7, 0xED, 0x42, 0xED, 0x62, 0xD0, 0xEB, 0xC9};
const uint8_t MeanData[] = {0xFD, 0x21, 0x00, 0x00, 0x00, 0xFD, 0x39, 0x19, 0xE5, 0xFD, 0xCB, 0xFF, 0x1E, 0xE1, 0xCB, 0x1C, 0xCB, 0x1D, 0xC9};
const uint8_t Mult_with_numberData[] = {0xFD, 0x21, 0x00, 0x00, 0x00, 0xFD, 0x39, 0xFD, 0x27, 0x03, 0xFD, 0x17, 0x03, 0x06, 0x00, 0x3E, 0xCB, 0x1D, 0xC9, 0xEB, 0xC9, 0x02, 0xDC, 0x48, 0x14, 0x02, 0x21, 0x00, 0x00, 0x00, 0x19, 0x4E, 0x23, 0x46, 0x2B, 0xED, 0x4C, 0x03, 0x03, 0x11, 0x00, 0x00, 0x00, 0xD5, 0x3E, 0x00, 0xEB, 0xED, 0x1F, 0x23, 0x23, 0x23, 0xEB, 0x09, 0x3D, 0x20, 0xF5, 0xE1, 0xC9, 0x02, 0xCD, 0x28, 0x08, 0x02, 0x21, 0xD2, 0xA8, 0xD1, 0xCD, 0xC0, 0x07, 0x02, 0xCD, 0xF0, 0x07, 0x02, 0xC3, 0xC0, 0x07, 0x02, 0x4E, 0x65, 0x65, 0x64, 0x20, 0x4C, 0x69, 0x62, 0x4C, 0x6F, 0x61, 0x64, 0x00, 0x74, 0x69, 0x6E, 0x79, 0x2E, 0x63, 0x63, 0x2F, 0x63, 0x6C, 0x69, 0x62, 0x73, 0x00, 0xC0, 0x47, 0x52, 0x41, 0x50, 0x48, 0x58, 0x00, 0x07, 0x06, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0xF4, 0x70, 0x30, 0x00, 0xE0, 0x25, 0x66, 0x00, 0x10, 0x00, 0x00, 0x00, 0x88, 0x02, 0x66, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0x6F, 0x67, 0x72, 0xB8, 0x3A, 0x66, 0x00, 0xE0, 0x25, 0x66, 0x00, 0xE0, 0x25, 0x66, 0x00, 0x01, 0x00, 0x00, 0x00, 0xE8, 0x25, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x65, 0x78, 0x65, 0x00, 0x00, 0x00, 0x01, 0x00, 0xAC, 0x02, 0x66, 0x00, 0x01, 0x00, 0x00, 0x00, 0xB0, 0x3A};
const uint8_t OrData[] = {0xB7, 0xED, 0x5A, 0x3F, 0x28, 0x01, 0xB7, 0xED, 0x62, 0x23};
const uint8_t PauseData[] = {0xF3, 0x2B, 0x0E, 0x6E, 0x06, 0x20, 0x10, 0xFE, 0x0D, 0x20, 0xF9, 0xB7, 0x11, 0xFF, 0xFF, 0xFF, 0x19, 0x38, 0xEF, 0xC9};
const uint8_t PrgmData[] = {0xCD, 0x98, 0x07, 0x02, 0xFD, 0xCB, 0x08, 0xCE, 0xCD, 0x00, 0x0F, 0x02, 0xCD, 0x9C, 0x07, 0x02, 0xFD, 0xCB, 0x08, 0x8E};
const uint8_t RandData[] = {0xDD, 0x27, 0x51, 0xDD, 0x17, 0x54, 0x44, 0x4D, 0x29, 0xCB, 0x13, 0xCB, 0x12, 0x29, 0xCB, 0x13, 0xCB, 0x12, 0x2C, 0x09, 0xDD, 0x2F, 0x51, 0xED, 0x5A, 0xDD, 0x2F, 0x54, 0xEB, 0xDD, 0x27, 0x57, 0xDD, 0x07, 0x5A, 0x29, 0xCB, 0x11, 0xCB, 0x10, 0xDD, 0x0F, 0x5A, 0x9F, 0xE6, 0xC5, 0xAD, 0x6F, 0xDD, 0x2F, 0x57, 0xEB, 0x09, 0xC9};
const uint8_t SincosData[] = {0x7D, 0xC6, 0x40, 0x6F, 0x7D, 0x47, 0xE6, 0x3F, 0xCB, 0x70, 0x28, 0x03, 0xC6, 0xBF, 0x2F, 0x11, 0x00, 0x00, 0x00, 0xED, 0x62, 0x6F, 0xEB, 0x19, 0xCB, 0x78, 0x5E, 0xC8, 0xED, 0x62, 0xED, 0x52, 0xEB, 0xC9, 0x00, 0x06, 0x0C, 0x12, 0x18, 0x1F, 0x25, 0x2B, 0x31, 0x37, 0x3D, 0x44, 0x4A, 0x4F, 0x55, 0x5B, 0x61, 0x67, 0x6D, 0x72, 0x78, 0x7D, 0x83, 0x88, 0x8D, 0x92, 0x97, 0x9C, 0xA1, 0xA6, 0xAB, 0xAF, 0xB4, 0xB8, 0xBC, 0xC1, 0xC5, 0xC9, 0xCC, 0xD0, 0xD4, 0xD7, 0xDA, 0xDD, 0xE0, 0xE3, 0xE6, 0xE9, 0xEB, 0xED, 0xF0, 0xF2, 0xF4, 0xF5, 0xF7, 0xF8, 0xFA, 0xFB, 0xFC, 0xFD, 0xFD, 0xFE, 0xFE, 0xFE, 0xFF};
const uint8_t SqrtData[] = {0x3B, 0xE5, 0x3B, 0xFD, 0xE1, 0x3B, 0xF1, 0xB7, 0xED, 0x62, 0xEB, 0xED, 0x62, 0x01, 0x40, 0x0C, 0x00, 0x91, 0xED, 0x52, 0x30, 0x03, 0x81, 0xED, 0x5A, 0x3F, 0xCB, 0x13, 0xCB, 0x12, 0xFD, 0x29, 0x17, 0xED, 0x6A, 0xFD, 0x29, 0x17, 0xED, 0x6A, 0x10, 0xE7, 0xC9};
const uint8_t XorData[] = {0x01, 0xFF, 0xFF, 0xFF, 0x09, 0x9F, 0xEB, 0x09, 0x99, 0x8F, 0xED, 0x62, 0x23};
#endif

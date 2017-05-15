segment data

.assume adl = 1

.def _AndOrXorData
.def _RandRoutine
.def _KeypadRoutine

_AndOrXorData:
    ld      bc, -1           ; 0
    add     hl, bc           ; 4
    sbc     a, a             ; 5
    ex      de, hl           ; 6
    ld      d, a             ; 7
    add     hl, bc           ; 8
    sbc     a, a             ; 9
    nop                      ; 10
    sbc     hl, hl           ; 11
    and     a, 1             ; 13
    ld      l, a             ; 15
    
_RandRoutine:
    ld      hl, (ix+81)
    ld      de, (ix+81+3)
    ld      b, h
    ld      c, l
    add     hl, hl
    rl      e
    rl      d
    add     hl, hl
    rl      e
    rl      d
    inc     l
    add     hl, bc
    ld      (ix+81), hl
    adc     hl, de
    ld      (ix+81+3), hl
    ex      de, hl
    ld      hl, (ix+81+6)
    ld      bc, (ix+81+9)
    add     hl, hl
    rl      c
    rl      b
    ld      (ix+81+9), bc
    sbc     a, a
    and     a, 197
    xor     a, l
    ld      l, a
    ld      (ix+81+6), hl
    ex      de, hl
    add     hl, bc
    ret
    
_KeypadRoutine:
    di
    ld      hl, 0F50200h
    ld      (hl), h
    xor     a, a
CheckKeyPressLoop:
    cp      a, (hl)
    jr      nz, CheckKeyPressLoop
    ei
    ld      l, b
    ld      a, (hl)
    sbc     hl, hl
    and     a, c
    ret     z
    inc     l
    ret

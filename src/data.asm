segment data

.assume adl = 1

.def _AndOrXorData
.def _SqrtRoutine
.def _RandRoutine
.def _KeypadRoutine
.def _MeanRoutine

_AndOrXorData:
    ld      bc, -1
    add     hl, bc
    sbc     a, a
    ex      de, hl
    ld      d, a
    add     hl, bc
    sbc     a, a
    and     a, d            ; This byte can be modified: and/or/xor
    sbc     hl, hl
    and     a, 1
    ld      l, a
   
_SqrtRoutine:
    di
    dec     sp              ; (sp) = ?
    push    hl              ; (sp) = ?uhl
            dec     sp      ; (sp) = ?uhl?
            pop     iy      ; (sp) = ?u, uix = hl?
            dec  sp         ; (sp) = ?u?
    pop     af              ; af = u?
    or      a, a
    sbc     hl, hl
    ex      de, hl          ; de = 0
    sbc     hl, hl          ; hl = 0
    ld      bc, 0C40h       ; b = 12, c = 0x40
Sqrt24Loop:
    sub     a, c
    sbc     hl, de
    jr      nc, Sqrt24Skip
    add     a, c
    adc     hl, de
Sqrt24Skip:
    ccf
    rl      e
    rl      d
    add     iy, iy
    rla
    adc     hl, hl
    add     iy, iy
    rla
    adc     hl, hl
    djnz    Sqrt24Loop
    ret
    
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

_MeanRoutine:
    ld      ix, 0
    add     ix, sp
    add     hl, de
    push    hl
            rr      (ix-1)
    pop     hl
    rr      h
    rr      l
    ld      ix, 0D1383Fh
    ret
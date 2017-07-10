.assume adl = 1
segment data
.def _RandData

_RandData:
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

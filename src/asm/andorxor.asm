.assume adl = 1
segment data
.def _AndOrXorData

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

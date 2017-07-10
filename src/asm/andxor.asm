.assume adl = 1
segment data
.def _AndXorData

_AndXorData:
    ld      bc, -1
    add     hl, bc
    sbc     a, a
    ex      de, hl
    add     hl, bc
    sbc     hl, hl
    and     a, l            ; This byte can be modified: and/xor
    inc     hl
    and     a, 1
    ld      l, a

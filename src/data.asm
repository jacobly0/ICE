segment data

.assume adl = 1

.def _AndData
.def _XorData
.def _OrData

_AndData:
    ld      bc, -1
    add     hl, bc
    sbc     a, a
    ex      de, hl
    ld      d, a
    add     hl, bc
    sbc     a, a
    and     a, d
    sbc     hl, hl
    and     a, 1
    ld      l, a
    
_XorData:
    ld      bc, -1
    add     hl, bc
    sbc     a, a
    ex      de, hl
    ld      d, a
    add     hl, bc
    sbc     a, a
    xor     a, d
    sbc     hl, hl
    and     a, 1
    ld      l, a
    
_OrData:
    ld      bc, -1
    add     hl, bc
    sbc     a, a
    ex      de, hl
    ld      d, a
    add     hl, bc
    sbc     a, a
    or      a, d
    sbc     hl, hl
    and     a, 1
    ld      l, a
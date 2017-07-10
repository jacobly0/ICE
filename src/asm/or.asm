.assume adl = 1
segment data
.def _OrData

_OrData:
    ld      bc, -1
    add     hl, bc
    ex      de, hl
    adc     hl, bc
    ccf
    sbc     hl, hl
    inc     hl

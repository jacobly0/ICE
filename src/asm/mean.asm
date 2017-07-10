.assume adl = 1
segment data
.def _MeanData

_MeanData:
    ld      iy, 0
    add     iy, sp
    add     hl, de
    push    hl
            rr      (iy-1)
    pop     hl
    rr      h
    rr      l
    ret
.assume adl = 1
segment data
.def _StringConcatenateData

_StringConcatenateData:
    push    bc
            push    hl
                    call 00000D4h           ; __strlen
                    push    hl
                    pop     bc
            pop     hl
            ld      a, b
            or      a, c
            jr      z, $+4
            ldir
            call 00000D4h                   ; __strlen
            push    hl
            pop     bc
    pop     hl
    ld      a, b
    or      a, c
    ret     z
    ldir
    ret
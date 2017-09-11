.assume adl = 1
segment data
.def _StringStoData

_StringStoData:
    push    hl
    call    00000D4h
    inc     hl
    push    hl
    pop     bc
    pop     hl

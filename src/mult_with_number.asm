segment data

.assume adl = 1

.def _MultWithNumber

_MultWithNumber:
    scf
    sbc hl, hl
    ld (hl), 2
    ld      iy, 0
    add     iy, sp
    ld      hl, (iy+3)
    ld      de, (iy+3)
    ld      b, 0
    ld      a, 26
GetFirstBitSetLoop:
    dec     a
    add     hl, hl
    jr      nc, GetFirstBitSetLoop
GetAmountOfOpcodesNeededLoop:
    adc     a, b
    adc     hl, hl
    jr      nz, GetAmountOfOpcodesNeededLoop
    rl      b
    jr      nz, DunnoWhatThisDoes
    sub     a, 3
DunnoWhatThisDoes:
    cp      a, 10
    jr      c, DoInsertHLDE
    ld      hl, -256
    add     hl, de
    jr      c, MultiplyLarge
MultiplySmall:
    ld      hl, 0CD003Eh
    ld      h, e
    ld      de, 0000150h
    ex      de, hl
    jp      InsertDEHL
MultiplyLarge:
    ld      a, 001h
    ex      de, hl
    call    InsertAHL
    ld      hl, 0000154h
    jp      InsertCallHL
DoInsertHLDE:
    djnz    DontNeedPushHLPopDE
    ld      a, 0E5h
    call    InsertA
    ld      a, 0D1h
    call    InsertA
DontNeedPushHLPopDE:
    ex      de, hl
    scf
GetFirstSetBit2Loop:
    adc     hl, hl
    jr      nc, GetFirstSetBit2Loop
InsertAddHLDE:
    or      a, a
    adc     hl, hl
    ret     z
    ld      a, 029h
    call    InsertA
    ld      a, 019h
    call    c, InsertA
    jr      InsertAddHLDE
    
InsertA:
    push    hl
            ld      hl, (iy+6)
            push    hl
                    ld      bc, (hl)
                    ld      (bc), a
                    inc     bc
            pop     hl
            ld      (hl), bc
    pop     hl
    ret
InsertAHL:
    push    hl
            call    InsertA
    pop     hl
InsertHL:
    ex      de, hl
    ld      hl, (iy+6)
    push    hl
            ld      hl, (hl)
            ld      (hl), de
            inc     hl
            inc     hl
            inc     hl
            ex      de, hl
    pop     hl
    ld      (hl), de
    ret
InsertCallHL:
    ld      a, 0CDh
    jr      InsertAHL
InsertDEHL:
    push    hl
            ex      de, hl
            call    InsertHL
    pop     hl
    jr      InsertHL
segment data

.assume adl = 1

.def _AndData
.def _XorData
.def _OrData
.def _RandRoutine

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
    
; 54 bytes
_RandRoutine:
	ld	hl, (ix+81)
	ld	de, (ix+81+3)
	ld	b, h
	ld	c, l
	add	hl, hl
	rl	e
	rl	d
	add	hl, hl
	rl	e
	rl	d
	inc	l
	add	hl, bc
	ld	(ix+81), hl
	adc	hl, de
	ld	(ix+81+3), hl
	ex	de, hl
	ld	hl, (ix+81+6)
	ld	bc, (ix+81+9)
	add	hl, hl
	rl	c
	rl	b
	ld	(ix+81+9), bc
	sbc	a, a
	and	a, 197
	xor	a, l
	ld	l, a
	ld	(ix+81+6), hl
	ex	de, hl
	add	hl, bc
	ret
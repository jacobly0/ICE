.assume adl = 1
segment data
.def _InputData

_InputData:
	call	0020814h
	call	0020828h
	xor	a, a
	ld	(0D00879h), a
	ld	(0D00599h), a
	call	0021320h
	ld	hl, (0D0244Eh)
	call	0020AE8h
	call	002050Ch
	ld	a, (de)
	inc	de
	inc	de
	ld	b, a
	sbc	hl, hl
_loop:	push	bc
	add	hl, hl
	push	hl
	pop	bc
	add	hl, hl
	add	hl, hl
	add	hl, bc
	ld	a, (de)
	sub	a, 030h
	ld	bc, 0
	ld	c, a
	add	hl, bc
	inc	de
	pop	bc
	djnz	_loop
	ld	(ix+0), hl
	jp	0021578h

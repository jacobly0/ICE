.assume adl = 1
segment data
.def _InputData

_InputData:
	call	0020814h		; _ClrScrn
	call	0020828h		; _HomeUp
	xor	a, a
	ld	(0D00879h), a		; ioPrompt
	ld	(0D00599h), a		; curUnder
	ld	b, (iy+9)
	ld	c, (iy+28)
	res	6, (iy+28)
	set	7, (iy+9)
	push	bc
	call	0021320h		; _GetStringInput
	pop	bc
	res	4, b
	ld	(iy+9), b
	ld	(iy+28), c
	ld	hl, (0D0244Eh)		; editSym
	call	0020AE8h		; _VarNameToOP1HL
	call	002050Ch		; _ChkFindSym
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
	jp	0021578h		; _DeleteTempEditEqu

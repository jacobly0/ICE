.assume adl = 1
segment data
.def _SinCosData

_SinCosData:
_FP_Cos:
	ld	a, l
	add	a, 64
	ld	l, a
_FP_Sin:
	push	hl
		ld	a, l
		and	a, 011000000b
		jr	z, _FP_DoSinI
		cp	a, 001000000b
		jr	z, _FP_DoSinII
		cp	a, 010000000b
		jr	z, _FP_DoSinIII
_FP_DoSinIV:
		xor	a, a
		jr	_FP_Insert
_FP_DoSinIII:
		ld	a, l
		sub	a, 128
		ld	l, a
		jr	_FP_DoSinI
_FP_DoSinII:
		ld	a, 128
_FP_Insert:
		sub	a, l
		ld	l, a
_FP_DoSinI:
		ld	a, l
		cp	a, 64
		jr	nz, _FP_Do
		ld	hl, 00100h
		jr	_FP_Stop
_FP_Do:		add	a, a
		add	a, a
		ld	l, a
		ld	h, 0
		ld	d, 1				; pi/2 in fixed point format (1+146/256)
		ld	e, 146
		call	_FP_Mul
		push	hl
			sra	h
			rr	l
			ld	d, h
			ld	e, l
			call	_FP_Mul
			push	hl			; x^2/4
				sra	h
				rr	l
				ld	d, h
				ld	e, l
				call	_FP_Mul
				sra	h
				rr	l
				inc	h
				ex	(sp), hl	; x^4/128+1 is on stack, HL=x^2/4
				xor	a, a
				ld	d, a
				ld	b, h
				ld	c, l
				add.s	hl, hl
				rla
				add.s	hl, hl
				rla
				add.s	hl, bc
				adc a, d
				ld	b, h
				ld	c, l
				add.s	hl, hl
				rla
				add.s	hl, hl
				rla
				add.s	hl, hl
				rla
				add.s	hl, hl
				rla
				add.s	hl, bc
				adc a, d
				ld	e, l
				ld	l, h
				ld	h, a
				rl	e
				adc.s	hl, hl
				rl	e
				jr	nc, $+3
				inc	hl
			pop	de
			ex	de, hl
			or	a, a
			sbc	hl, de
		pop	de
		call	_FP_Mul
_FP_Stop:
	pop	de
	bit	7, e
	ret	z
	ex	de, hl
	sbc	hl, hl
	sbc	hl, de
	ret
_FP_Mul:
; Inputs: H.L * D.E
; Output: H.L
; Speed:
;   154 cycles
	ld	a, e
	push	hl
	ld	b, l
	ld	c, d
	ld	l, d
	ld	d, h
	mlt	hl
	mlt	de
	mlt	bc
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, de
	add	hl, bc
	pop	de
	ld	d, a
	mlt	de
	ld	e, d
	ld	d, 0
	add	hl, de
	ret

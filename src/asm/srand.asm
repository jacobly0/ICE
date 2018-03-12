.assume adl = 1
segment data
.def _SRandData

_SRandData:
	xor	a, a
	ld	(0), hl
	ld	hl, 0
	ld	(hl), a
	ld	b, 12
__setstateloop:
	inc	hl
	ld	(hl), b
	djnz	__setstateloop
	ret

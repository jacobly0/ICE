.assume adl = 1
segment data
.def _MeanData

_MeanData:
	add	hl, de
	ld	a, 1
_ShiftRight:
	ld	b, a
__ShiftRightLoop:
	ld	(ix+07Ch), hl
	rr	(ix+07Eh)
	ld	hl, (ix+07Ch)
	rr	h
	rr	l
	or	a, a
	djnz	__ShiftRightLoop
	ret

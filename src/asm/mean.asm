.assume adl = 1
segment data
.def _MeanData

_MeanData:
	add	hl, de
	push	hl
	ld	hl,2
	add	hl,sp
	rr	(hl)
	pop	hl
	rr	h
	rr	l
	ret

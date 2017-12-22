.assume adl = 1
segment data
.def _MeanData

_MeanData:
	add	hl, de
	rla
	push	hl
	ld	hl,2
	add	hl,sp
	rra
	rr	(hl)
	pop	hl
	rr	h
	rr	l
	ret

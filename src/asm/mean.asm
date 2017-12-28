.assume adl = 1
segment data
.def _MeanData

_MeanData:
	add	hl, de
	ld	(ix+07Ch), hl
	rr	(ix+07Eh)
	ld	hl, (ix+07Ch)
	rr	h
	rr	l
	ret

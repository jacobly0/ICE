.assume adl = 1
segment data
.def _XorData

_XorData:				; Credits to Runer112
	ld	bc, -1
	add	hl, bc
	sbc	a, a
	ex	de, hl
	add	hl, bc
	sbc	a, c
	adc	a, a
	sbc	hl, hl
	inc	hl
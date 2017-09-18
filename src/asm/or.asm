.assume adl = 1
segment data
.def _OrData

_OrData:				; Credits to Runer112
	or	a, a
	adc	hl, de
	ccf
	jr	z, Return
	or	a, a
Return:
	sbc	hl, hl
	inc	hl

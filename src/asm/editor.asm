.assume adl = 1
segment data
.def _GotoEditor

_GotoEditor:
	ld	a, tProgObj
	ld	(progToEdit), a
	pop	de
	pop	hl
	ld	hl, (hl)
	ld	de, progToEdit + 1
	ld	bc, 8
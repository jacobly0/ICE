functionInc:
	call _IncFetch
	ret c
	ld a, 0DDh
	call InsertA											; inc (ix+*) (1)
	ld a, 034h
	call InsertA											; inc (ix+*) (2)
	call _CurFetch
	sub t0
	call InsertA											; inc (ix+*) (3)
	jp _IncFetch
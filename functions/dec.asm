functionDec:
	call _IncFetch
	ret c
	ld a, 0DDh
	call InsertA											; dec (ix+*) (1)
	ld a, 035h
	call InsertA											; dec (ix+*) (2)
	call _CurFetch
	sub t0
	call InsertA											; dec (ix+*) (3)
	jp _IncFetch
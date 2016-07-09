functionRepeat:
	call _IncFetch
	ld hl, AmountOfEnds
	inc (hl)
	ld hl, (programPtr)
	push hl
		ld hl, (curPC)
		push hl
_:			call _IncFetch
			cp tEnter
			jr nz, -_
			call _IncFetch
			call ParseToEnd
		pop hl
		ld de, (curPC)
		push de
			ld (curPC), hl
			call ParseExpression
		pop de
		ld (curPC), de
		ld a, 0B7h
		call InsertA								; or a
		ld a, 0CAh
		call InsertA								; jp z, ******
	pop hl
	jp InsertHL										; jp z, XXXXXX
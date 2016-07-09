functionWhile:
	call _IncFetch
	ld hl, AmountOfEnds
	inc (hl)
	ld hl, (programPtr)
	push hl
		call ParseExpression
		ld a, 0B7h
		call InsertA											; or a
		ld a, 0CAh
		call InsertA											; jp z, ******
		ld hl, (programPtr)
		push hl
			call InsertHL										; jp z, RANDOM
			call ParseToEnd
		pop hl
		ld de, (programPtr)
		inc de													; jump over jump
		inc de
		inc de
		inc de
		ld (hl), de												; jp z, XXXXXX
		ld a, 0C3h
		call InsertA											; jp ******
	pop hl
	jp InsertHL													; jp XXXXXX
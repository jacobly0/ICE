	ld hl, (outputPtr)
	bit prev_is_number, (iy+myFlags)
	jr z, NewNumber
LastEntryWasAlreadyNumber:
	dec hl											; HL = pointer to previous value on output
	ld a, (hl)
	push hl
		ld h, a
		ld l, 10
		mlt hl
		push hl
			call _CurFetch
		pop hl
		sub t0
		add a, l
	pop hl
	ld (hl), a
	jr StopNumber
NewNumber:
	ld a, typeNumber
	add a, b
	ld (hl), a
	inc hl
	push hl
		call _CurFetch
	pop hl
	sub t0
	ld (hl), a
	ld hl, (outputPtr)
	inc hl
	inc hl
	ld (outputPtr), hl
StopNumber:
	set prev_is_number, (iy+myFlags)
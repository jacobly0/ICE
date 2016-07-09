functionInput:
	ld a, 03Eh
	call InsertA									; ld a, **
	call _IncFetch
	sub t0
	call InsertA									; ld a, XX
	call _IncFetch
	ld a, 032h
	call InsertA									; ld (******), a
	call InsertProgramPtrToDataOffset
	bit has_already_input, (iy+myFlags)
	jr nz, AddPointerToInput
	ld hl, (programDataDataPtr)
	ld bc, 52
	add hl, bc
	call InsertHL									; ld (XXXXXX), a
	ld a, 0CDh
	call InsertA									; call ******
	call InsertProgramPtrToDataOffset
	ld hl, (programDataDataPtr)
	ld (InputStartData), hl
	push hl
	pop de
	call InsertHL									; call XXXXXX
	ld hl, InputRoutine
	ld bc, InputRoutineEnd-InputRoutine
	ldir
	ld (programDataDataPtr), de
	set has_already_input, (iy+myFlags)
	ret
AddPointerToInput:
	ld hl, (InputStartData)
	push hl
		ld bc, 52
		add hl, bc
		call InsertHL									; ld (XXXXXX), a
		ld a, 0CDh
		call InsertA									; call ******
		call InsertProgramPtrToDataOffset
	pop hl
	jp InsertHL											; call XXXXXX
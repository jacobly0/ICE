functionDisp:
	call _IncFetch
	ret c
	cp tString
	jr z, DispString
DispInteger:
	call ParseExpression
	ld hl, 062EDB7h
	call InsertHL												; or a \ sbc hl, hl
	ld a, 06Fh
	call InsertA												; ld l, a
	ld a, 0CDh
	call InsertA												; call ******
	ld hl, _DispHL
	call InsertHL												; call _DispHL
	ld a, 0CDh
	call InsertA												; call ******
	ld hl, _NewLine
	jp InsertHL													; call _NewLine
DispString:
	ld hl, (programDataDataPtr)
	push hl
DispStringLoop:
		push hl
			call _IncFetch
		pop hl
		jr c, StopString
		cp tString
		jr z, StopString
		cp tEnter
		jr z, StopString
		ld (hl), a
		inc hl
		jr DispStringLoop
StopString:
		ld (hl), 0
		inc hl
		ld (programDataDataPtr), hl
		ld a, 021h
		call InsertA											; ld hl, ******
		call InsertProgramPtrToDataOffset
	pop hl
	call InsertHL												; ld hl, XXXXXX
	ld a, 0CDh
	call InsertA												; call ******
	ld hl, _PutS
	call InsertHL												; call _PutS
	ld a, 0CDh
	call InsertA												; call ******
	ld hl, _NewLine
	call InsertHL												; call _NewLine
	jp _IncFetch
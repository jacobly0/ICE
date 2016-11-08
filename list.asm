	ld hl, (outputPtr)
	ld (hl), typeList
	inc hl
	ld de, (tempListsPtr)
	ld (hl), de
	ld (ListLengthSMC), de
	inc hl
	inc hl
	inc hl
	ld (outputPtr), hl
	ex de, hl
	inc hl
	inc hl
	inc hl
	ld (tempListsPtr), hl
ListStart:
	or a
	sbc hl, hl
ListLoop:
	push hl
		call _IncFetch
	pop hl
	cp t0
	jr c, ListNotNumber
	cp t9+1
	jr nc, ListNotNumber
	sub t0
	add hl, hl
	push hl
	pop de
	add hl, hl
	add hl, hl
	add hl, de
	ld de, 0
	ld e, a
	add hl, de
	jr ListLoop
ListNotNumber:
	cp tComma
	jr nz, ListNotNumberOrComma
	ex de, hl
	ld hl, (tempListsPtr)
	ld (hl), de
	inc hl
	inc hl
	inc hl
	ld (tempListsPtr), hl
	jr ListStart
ListNotNumberOrComma:
	cp tRBrace
	jr z, ListStop
	cp tStore
	jr nz, ListError
ListStop:
	ex de, hl
	ld hl, (tempListsPtr)
	ld (hl), de
	inc hl
	inc hl
	inc hl
	ld (tempListsPtr),  hl
ListLengthSMC = $+1
	ld de, 0
	dec hl
	dec hl
	dec hl
	or a
	sbc hl, de
	ex de, hl
	ld (hl), de
	cp tStore
	call nz, _IncFetch
	jp MainLoop
ListError:
	ld hl, InvalidListArgumentMessage
	jp DispFinalString
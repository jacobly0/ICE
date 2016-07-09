functionGoto:
	ld a, 0C3h
	call InsertA									; jp ******
	ld hl, (gotoPtr)
	ld bc, (programPtr)
	ld (hl), bc
	inc hl
	inc hl
	inc hl
	ld de, (curPC)
	inc de
	ld (hl), de
	inc hl
	inc hl
	inc hl
	ld (gotoPtr), hl
	call InsertHL									; jp RANDOM
GotoNameLoop:
	call _IncFetch
	ret c
	cp tEnter
	ret z
	jr GotoNameLoop
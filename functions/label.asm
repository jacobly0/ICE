functionLabel:
	ld hl, (labelPtr)
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
	ld (labelPtr), hl
LabelNameLoop:
	call _IncFetch
	ret c
	cp tEnter
	ret z
	jr LabelNameLoop
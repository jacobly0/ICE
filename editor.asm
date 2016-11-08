OpenEditBuffer:
	push hl
		ld de, saveSScreen
		ld hl, EditBufferStart
		ld bc, EditBufferStop - EditBufferStart2
		ldir
		jp saveSScreen
EditBufferStart:
.org saveSScreen
EditBufferStart2:
		ld hl, OP1+1
		ld de, progToEdit
		call _Mov8b
		scf
		sbc hl, hl
		ld (hl), 2
		ld hl, UserMem
		ld de, EditBufferStart-start+EditBufferStop-EditBufferStart2
		call _DelMem
		ld a, cxPrgmEdit
		call _NewContext
	pop hl
EditBufferSMC = $+1
	ld de, 0
	or a
	sbc hl, de
	push hl
	pop bc
	ld de, (editTop)
	ld hl, (editTail)
	jr z, +_
	ldir
	ld (editCursor), de
	ld (editTail), hl
_:	push bc
	pop hl
	push hl
EditorLoop:
		ld de, (editTop)
		or a
		sbc hl, de
		jr z, EditorFound
		call _BufLeft
		ld hl, (editCursor)
		ld a, (hl)
		cp tEnter
		jr z, EditorFound
	pop hl
	inc hl
	push hl
		jr EditorLoop
EditorFound:
		call _BufRight
		call _DispEOW
	pop hl
	call _SetAToHLU
	or a, h
	or a, l
	jr z, EditorMon
EditorScroll:
	dec hl
	call _SetAToHLU
	or a, h
	or a, l
	jr z, EditorMon
	push hl
		call _CursorRight
	pop hl
	jr EditorScroll
EditorMon:
	set 2, (iy+1)
	res 2, (iy+50)
	res 7, (iy+20)
	res 3, (iy+5)
	jp _Mon
	;ret
EditBufferStop:
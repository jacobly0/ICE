FindNextGoodVar:
	ld de, (pTemp)
	call _CpHLDE
	jr nz, FindContinue
	inc a									; return nz
	ret
FindContinue:
	ld a, (hl)
	dec hl
	dec hl
	dec hl
	cp ProgObj
	jr z, FindGoodProgram
	cp ProtProgObj
	jr nz, FindWrongProgram
FindGoodProgram:
	ld e, (hl)
	dec hl
	ld d, (hl)
	dec hl
	ld a, (hl)
	call _SetDEUToA
	cp 0D0h
	jr nc, +_
	push hl
		ex de, hl
		ld de, 9
		add hl, de
		ld e, (hl)
		inc e
		add hl, de
		ex de, hl
	pop hl
_:	inc de
	inc de
	ld a, (de)
	cp tii
	jr nz, FindWrongProgram2
	dec hl
	push de
		push hl
			ld hl, (ProgramNamesPtr)
			ld de, 8
			add hl, de
			ld (ProgramNamesPtr), hl
			or a
			sbc hl, de
			ex de, hl
		pop hl
		ld b, (hl)
		ld c, b
		dec hl
FindCopyProgName:
		ld a, (hl)
		ld (de), a
		dec hl
		inc de
		djnz FindCopyProgName
		xor a
		ld (de), a
	pop de
	cp a
	ret
FindWrongProgram:
	dec hl
	dec hl
FindWrongProgram2:
	dec hl
	ld b, (hl)
	dec hl
FindSkipProgramName:
	dec hl
	djnz FindSkipProgramName
	jr FindNextGoodVar
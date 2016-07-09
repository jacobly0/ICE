functionDiv:
	ld a, b
	cp typeNumber
	jp z, DivNumberXXX
	cp typeVariable
	jp z, DivVariableXXX
	cp typeFunction
	jp z, DivFunctionXXX
DivChainXXX:
	ld a, c
	cp ChainFirst
	jr z, DivChainFirstXXX
DivChainPushXXX:
	ld a, d
	cp typeNumber
	jr z, DivChainPushNumber
	cp typeVariable
	jr z, DivChainPushVariable
	cp typeFunction
	jr z, DivChainPushFunction
DivChainPushChain:
	ld hl, 0012EE1h
	call InsertHL						; pop hl \ ld l, 1
	ld a, 0EDh
	call InsertA						; mlt hl (1)
	ld a, 06Ch
	call InsertA						; mlt hl (2)
	jp DivInsert
DivChainPushNumber:
	ld a, 0F1h
	call InsertA						; pop af
	jr DivChainFirstNumber
DivChainPushVariable:
	ld hl, 0012EE1h
	call InsertHL						; pop hl \ ld l, 1
	ld a, 0EDh
	call InsertA						; mlt hl (1)
	ld a, 06Ch
	call InsertA						; mlt hl (2)
	call InsertAIXE						; ld a, (ix+*)
	jp DivInsert
DivChainPushFunction:
	ld hl, 0012EE1h
	call InsertHL						; pop hl \ ld l, 1
	ld a, 0EDh
	call InsertA						; mlt hl (1)
	ld a, 06Ch
	call InsertA						; mlt hl (2)
	ld a, e
	call GetFunction
	jp DivInsert
DivChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, DivChainFirstNumber
	cp typeFunction
	jp z, DivChainFirstFunction
DivChainFirstVariable:
	ld hl, 062EDB7h
	call InsertA						; or a \ sbc hl, hl
	ld a, 06Fh
	call InsertA						; ld l, a
	call InsertAIXE						; ld a, (ix+*)
	jp DivInsert
DivChainFirstNumber:
	ld a, e
	or a
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	cp 21
	jr nc, +_
	cp 1
	ret z
	dec e
	dec e
	ld d, 14
	mlt de
	ld hl, div_table
	add hl, de
	ld bc, 0
	ld c, (hl)
	inc hl
	ld de, (programPtr)
	ldir
	ld (programPtr), de
	ret
_:	cp 128
	jr nc, +_
	ld hl, 0D6FF06h
	call InsertHL						; ld b, -1 \ sub a, *
	call InsertA						; sub a, XX
	ld hl, 0FC3004h
	call InsertHL						; inc b \ jr nc, $-2
	ld a, 081h
	call InsertA						; add a, c
	ld a, 078h
	jp InsertA							; ld a, b
_:	ld hl, 09F00D6h
	ld h, e
	call InsertHL						; sub a, XX \ sbc a, a
	ld a, 03Ch
	jp InsertA							; inc a
DivChainFirstFunction:
	ld hl, 062EDB7h
	call InsertA						; or a \ sbc hl, hl
	ld a, 06Fh
	call InsertA						; ld l, a
	ld a, e
	call GetFunction
	jp DivInsert
DivNumberXXX:
	ld a, d
	cp typeNumber
	jr z, DivNumberNumber
	cp typeVariable
	jr z, DivNumberVariable
	cp typeFunction
	jr z, DivNumberFunction
DivNumberChain:
	ld a, 021h
	call InsertA						; ld hl, ******
	or a
	sbc hl, hl
	ld l, c
	call InsertHL						; ld hl, XXXXXX
	jp DivInsert
DivNumberNumber:
	xor	a
	ld	b, 8
_:	sla	c
	rla
	cp	e
	jr	c, $+4
	sub	e
	inc	c
	djnz -_
	inc hl
	ld (hl), c
	res chain_operators, (iy+myFlags)
	ret
DivNumberVariable:
	ld a, c
	or a
	jr nz, +_
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	ld a, 021h
	call InsertA						; ld hl, ******
	or a
	sbc hl, hl
	ld l, c
	call InsertHL						; ld hl, XXXXXX	
	call InsertAIXE						; ld a, (ix+*)
	jr DivInsert
DivNumberFunction:
	ld a, 021h
	call InsertA						; ld hl, ******
	or a
	sbc hl, hl
	ld l, c
	call InsertHL						; ld hl, XXXXXX	
	ld a, e
	call GetFunction
	jr DivInsert
DivVariableXXX:
	ld a, d
	cp typeNumber
	jr z, DivVariableNumber
	cp typeVariable
	jr z, DivVariableVariable
	cp typeFunction
	jr z, DivVariableFunction
DivVariableChain:
	ld hl, 062EDB7h
	call InsertA						; or a \ sbc hl, hl
	call InsertLIXC						; ld l, (ix+*)
	jr DivInsert
DivVariableNumber:
	ld a, e
	or a
	jp z, DivChainFirstNumber
	call InsertAIXC						; ld a, (ix+*)
	jp DivChainFirstNumber
DivVariableVariable:
	ld a, c
	cp e
	jr nz, +_
	ld e, 1
	jp OrVariableNumber
_:	ld hl, 062EDB7h
	call InsertHL						; or a \ sbc hl, hl
	call InsertLIXC						; ld l, (ix+*)
	call InsertAIXE						; ld a, (ix+*)
DivInsert:
	ld a, 0CDh
	call InsertA						; call ******
	ld hl, _DivHLByA
	call InsertHL						; call _DivHLByA
	ld a, 07Dh
	jp InsertA							; ld a, l
DivVariableFunction:
	ld hl, 062EDB7h
	call InsertHL						; or a \ sbc hl, hl
	call InsertLIXC						; ld l, (ix+*)
	ld a, e
	call GetFunction
	jr DivInsert
DivFunctionXXX:
	ld a, d
	cp typeChain
	jr nz, +_
	ld a, 047h
	call InsertA						; ld b, a
	ld a, c
	call GetFunction
	ld a, 080h
	jp InsertA							; add a, b
_:	ld a, c
	call GetFunction
	ld a, d
	cp typeNumber
	jp z, DivChainFirstNumber
	cp typeVariable
	jp z, DivChainFirstVariable
	jp DivChainFirstFunction
functionAnd:
	ld a, b
	cp typeNumber
	jp z, AndNumberXXX
	cp typeVariable
	jp z, AndVariableXXX
	cp typeFunction
	jp z, AndFunctionXXX
AndChainXXX:
	ld a, c
	cp ChainFirst
	jr z, AndChainFirstXXX
AndChainPushXXX:
	ld a, d
	cp typeChain
	jr z, AndChainPushChain
	ld a, 0F1h
	call InsertA						; pop af
	ld a, d
	cp typeNumber
	jr z, AndChainFirstNumber
	cp typeVariable
	jr z, AndChainFirstVariable
	cp typeFunction
	jr z, AndChainFirstFunction
AndChainPushChain:
	ld hl, 028B7C1h
	call InsertHL						; pop bc \ or a \ jr z, *
	ld hl, 0B77806h
	call InsertHL						; jr z, 06 \ ld a, b \ or a
	ld hl, 03E0228h
	call InsertHL						; jr z, 02 \ ld a, *
	ld a, 1
	jp InsertA							; ld a, 1
AndChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, AndChainFirstNumber
	cp typeFunction
	jr z, AndChainFirstFunction
AndChainFirstVariable:
	ld hl, 00828B7h
	call InsertHL						; or a \ jr z, 08
	call InsertAIXE						; ld a, (ix+*)
	ld hl, 0022887h
	call InsertHL						; or a \ jr z, 02
	ld a, 03Eh
	call InsertA						; ld a, *
	ld a, 1
	jp InsertA							; ld a, 1
AndChainFirstNumber:
	ld c, e
	jr AndNumberChain
AndChainFirstFunction:
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	ld a, 080h
	jp InsertA							; add a, b
AndNumberXXX:
	ld a, d
	cp typeNumber
	jr z, AndNumberNumber
	cp typeVariable
	jr z, AndNumberVariable
	cp typeFunction
	jr z, AndNumberFunction
AndNumberChain:
	ld a, c
	or a
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	ld hl, 09F01D6h
	call InsertHL						; sub a, 1 \ sbc a, a
	ld a, 03Ch
	jp InsertA							; inc a
AndNumberNumber:
	ld a, c
	or a
	jr z, +_
	ld a, e
	or a
	jr z, +_
	inc a
_:	inc hl
	ld (hl), a
	res chain_operators, (iy+myFlags)
	ret
AndNumberVariable:
	ld a, c
	ld c, e
	ld e, a
	jr AndVariableNumber
AndNumberFunction:
	ld a, e
	call GetFunction
	ld e, c
	jr AndChainFirstNumber
AndVariableXXX:
	ld a, d
	cp typeNumber
	jr z, AndVariableNumber
	cp typeVariable
	jr z, AndVariableVariable
	cp typeFunction
	jr z, AndVariableFunction
AndVariableChain:
	ld hl, 00828B7h
	call InsertHL						; or a \ jr z, 08
	call InsertAIXC						; ld a, (ix+*)
	ld hl, 00228B7h
	call InsertHL						; or a \ jr z, 2
	ld a, 03Eh
	call InsertA						; ld a, **
	ld a, 1
	jp InsertA							; ld a, 1
AndVariableNumber:
	ld a, e
	or a
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	call InsertAIXC						; ld a, (ix+*)
	ld hl, 09F01D6h
	call InsertHL						; sub a, 1 \ sbc a, a
	ld a, 03Ch
	jp InsertA							; inc a
AndVariableVariable:
	ld a, c
	cp e
	jr nz, +_
	ld c, 1
	jr AndNumberVariable
_:	call InsertAIXE						; ld a, (ix+*)
	jr AndVariableChain
AndVariableFunction:
	ld a, e
	call GetFunction
	ld e, c
	jp AndChainFirstVariable
AndFunctionXXX:
	ld a, c
	ld c, e
	ld e, a
	ld a, d
	cp typeNumber
	jr z, AndNumberFunction
	cp typeVariable
	jr z, AndVariableFunction
	cp typeChain
	jp z, AndChainFirstFunction
AndFunctionFunction:
	ld a, c
	call GetFunction
	ld a, 0B7h
	call InsertA						; or a
	ld a, 028h
	call InsertA						; jr z, *
	push hl
		call InsertA					; jr z, RANDOM
		ld a, e
		call GetFunction
		ld hl, 0228B7h
		call InsertHL					; or a \ jr z, 02
	pop de
	or a
	sbc hl, de
	ld a, l
	inc a
	ld (de), a
	ld a, 03Eh
	call InsertA						; ld a, *
	ld a, 1
	jp InsertA							; ld a, 1
functionOr:
	ld a, b
	cp typeNumber
	jp z, OrNumberXXX
	cp typeVariable
	jp z, OrVariableXXX
	cp typeFunction
	jp z, OrFunctionXXX
OrChainXXX:
	ld a, c
	cp ChainFirst
	jr z, OrChainFirstXXX
OrChainPushXXX:
	ld a, d
	cp typeNumber
	jr z, OrChainPushNumber
	cp typeVariable
	jr z, OrChainPushVariable
	cp typeFunction
	jr z, OrChainPushFunction
OrChainPushChain:
	ld a, 0C1h
	call InsertA						; pop bc
	ld a, 0B0h
	call InsertA						; or a, b
	jp OrInsert
OrChainPushNumber:
	ld a, 0F1h
	call InsertA						; pop af
	jr OrChainFirstNumber
OrChainPushVariable:
	ld a, 0F1h
	call InsertA						; pop af
	jr OrChainFirstVariable
OrChainPushFunction:
	ld a, 0F1h
	call InsertA						; pop af
	jr OrChainFirstFunction
OrChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, OrChainFirstNumber
	cp typeFunction
	jr z, OrChainFirstFunction
OrChainFirstVariable:
	ld a, 0DDh
	call InsertA						; or a, (ix+*) (1)
	ld a, 0B6h
	call InsertA						; or a, (ix+*) (2)
	ld a, e
	call InsertA						; or a, (ix+*) (3)
	jp OrInsert
OrChainFirstNumber:
	ld a, e
	or a
	jp z, OrInsert
	ld (hl), typeNumber
	inc hl
	ld (hl), 1
	res chain_operators, (iy+myFlags)
	ret
OrChainFirstFunction:
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	ld a, 0B0h
	call InsertA						; or a, b
	jr OrInsert
OrNumberXXX:
	ld a, d
	cp typeNumber
	jr z, OrNumberNumber
	cp typeVariable
	jr z, OrNumberVariable
	cp typeFunction
	jr z, OrNumberFunction
OrNumberChain:
	ld e, c
	jr OrChainFirstNumber
OrNumberNumber:
	ld a, c
	or e
	ld a, 0
	jr z, +_
	inc a
_:	inc hl
	ld (hl), a
	res chain_operators, (iy+myFlags)
	ret
OrNumberVariable:
	ld a, e
	ld e, c
	ld c, a
	jr OrVariableNumber
OrNumberFunction:
	ld a, c
	or a
	jr z, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 1
	res chain_operators, (iy+myFlags)
	ret
_:	ld a, e
	call GetFunction
	ld e, c
	jr OrChainFirstNumber
OrVariableXXX:
	ld a, d
	cp typeNumber
	jr z, OrVariableNumber
	cp typeVariable
	jr z, OrVariableVariable
	cp typeFunction
	jr z, OrVariableFunction
OrVariableChain:
	ld e, c
	jp OrChainFirstVariable
OrVariableNumber:
	ld a, e
	or a
	jr z, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 1
	res chain_operators, (iy+myFlags)
	ret
_:	call InsertAIXC						; ld a, (ix+*)
	jr OrInsert
OrVariableVariable:
	ld a, c
	cp e
	jr nz, +_
	ld e, 0
	jr OrVariableNumber
_:	call InsertAIXC						; ld a, (ix+*)
	jp OrChainFirstVariable
OrInsert:
	ld hl, 9F01D6h
	call InsertHL						; sub a, 1 \ sbc a, a
	ld a, 3Ch
	jp InsertA							; inc a
OrVariableFunction:
	ld a, e
	call GetFunction
	ld e, c
	jp OrChainFirstVariable
OrFunctionXXX:
	ld a, c
	ld c, e
	ld e, a
	ld a, d
	cp typeNumber
	jr z, OrNumberFunction
	cp typeVariable
	jr z, OrVariableFunction
	cp typeChain
	jp z, OrChainFirstFunction
OrFunctionFunction:
	ld a, c
	call GetFunction
	ld a, 047h
	call InsertA						; ld a, b
	ld a, e
	call GetFunction
	ld a, 0B0h
	call InsertA						; or a, b
	jr OrInsert
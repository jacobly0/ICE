functionNE:
	ld a, b
	cp typeNumber
	jp z, NENumberXXX
	cp typeVariable
	jp z, NEVariableXXX
	cp typeFunction
	jp z, NEFunctionXXX
NEChainXXX:
	ld a, c
	cp ChainFirst
	jr z, NEChainFirstXXX
NEChainPushXXX:
	ld a, d
	cp typeNumber
	jr z, NEChainPushNumber
	cp typeVariable
	jr z, NEChainPushVariable
	cp typeFunction
	jr z, NEChainPushFunction
NEChainPushChain:
	ld a, 0C1h
	call InsertA						; pop bc
	ld a, 090h
	call InsertA						; sub a, b
	jp NEInsert
NEChainPushNumber:
	ld a, 0F1h
	call InsertA						; pop af
	jr NEChainFirstNumber
NEChainPushVariable:
	ld a, 0F1h
	call InsertA						; pop af
	jr NEChainFirstVariable
NEChainPushFunction:
	ld a, e
	call GetFunction
	ld a, 0C1h
	call InsertA						; pop bc
	ld a, 090h
	call InsertA						; sub a, b
	jp NEInsert
NEChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, NEChainFirstNumber
	cp typeFunction
	jr z, NEChainFirstFunction
NEChainFirstVariable:
	ld a, 0DDh
	call InsertA						; sub a, (ix+*) (1)
	ld a, 096h
	call InsertA						; sub a, (ix+*) (2)
	ld a, e
	call InsertA						; sub a, (ix+*) (3)
	jp NEInsert
NEChainFirstNumber:
	ld a, e
	cp 1
	jr nz, +_
	ld a, 03Dh
	call InsertA						; dec a
	xor a
_:	or a
	jr z, +_
	ld a, 0D6h
	call InsertA						; sub a, *
	ld a, e
	call InsertA						; sub a, XX
_:	cp 255
	jr nz, +_
	ld a, 03Ch
	call InsertA						; inc a
_:	jp NEInsert
NEChainFirstFunction:
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	ld a, 080h
	call InsertA						; sub a, b
	jr NEInsert
NENumberXXX:
	ld a, d
	cp typeNumber
	jr z, NENumberNumber
	cp typeVariable
	jr z, NENumberVariable
	cp typeFunction
	jr z, NENumberFunction
NENumberChain:
	ld e, c
	jr NEChainFirstNumber
NENumberNumber:
	ld a, c
	cp e
	ld a, 0
	jr nz, $+3
	inc a
	inc hl
	ld (hl), a
	res chain_operators, (iy+myFlags)
	ret
NENumberVariable:
	ld a, c
	ld c, e
	ld e, a
	jr NEVariableNumber
NENumberFunction:
	ld a, e
	call GetFunction
	ld a, 0D6h
	call InsertA						; sub a, *
	ld a, c
	call InsertA						; sub a, XX
	jr NEInsert
NEVariableXXX:
	ld a, d
	cp typeNumber
	jr z, NEVariableNumber
	cp typeVariable
	jr z, NEVariableVariable
	cp typeFunction
	jr z, NEVariableFunction
NEVariableChain:
	ld e, c
	jp NEChainFirstVariable
NEVariableNumber:
	call InsertAIXC						; ld a, (ix+*)
	jp NEChainFirstNumber
NEVariableVariable:
	ld a, c
	cp e
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	call InsertAIXC						; ld a, (ix+*)
	jp NEChainFirstVariable
NEVariableFunction:
	ld a, e
	call GetFunction
	ld a, 0DDh
	call InsertA						; sub a, (ix+*) (1)
	ld a, 096h
	call InsertA						; sub a, (ix+*) (2)
	ld a, c
	call InsertA						; sub a, (ix+*) (3)
NEInsert:
	ld a, 0D6h
	call InsertA						; add a, *
	ld hl, 03C9F01h
	jp InsertHL							; sub a, 1 \ sbc a, a \ inc a
NEFunctionXXX:
	ld a, c
	ld c, e
	ld e, a
	ld a, d
	cp typeNumber
	jr z, NENumberFunction
	cp typeVariable
	jr z, NEVariableFunction
	cp typeChain
	jp z, NEChainFirstFunction
NEFunctionFunction:
	ld a, c
	call GetFunction
	jp NEChainFirstFunction
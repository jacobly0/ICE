functionXor:
	ld a, b
	cp typeNumber
	jp z, XorNumberXXX
	cp typeVariable
	jp z, XorVariableXXX
	cp typeFunction
	jp z, XorFunctionXXX
XorChainXXX:
	ld a, c
	cp ChainFirst
	jr z, XorChainFirstXXX
XorChainPushXXX:
	ld a, d
	cp typeNumber
	jr z, XorChainPushNumber
	cp typeVariable
	jr z, XorChainPushVariable
	cp typeFunction
	jr z, XorChainPushFunction
XorChainPushChain:
	ld hl, 001C69Fh
	call InsertHL				; sub a, 1 \ sbc a, a
	ld a, 047h
	call InsertA				; ld b, a
	ld a, 0F1h
	call InsertA				; pop af
	jp XorInsert
XorChainPushNumber:
	ld a, 0F1h
	call InsertA				; pop af
	jr XorChainFirstNumber
XorChainPushVariable:
	call InsertAIXE				; ld a, (ix+*)
	jr XorChainPushChain
XorChainPushFunction:
	ld a, e
	call GetFunction
	jr XorChainPushChain
XorChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, XorChainFirstNumber
	cp typeFunction
	jr z, XorChainFirstFunction
XorChainFirstVariable:
	ld hl, 001C69Fh
	call InsertHL						; sub a, 1 \ sbc a, a
	ld a, 047h
	call InsertA						; ld b, a
	call InsertAIXE						; ld a, (ix+*)
	jp XorInsert
XorChainFirstNumber:
	ld a, e
	or a
	jp nz, XorInsert
	ld hl, 9F01D6h
	call InsertHL						; sub a, 1 \ sbc a, a
	ld a, 3Ch
	jp InsertA							; inc a
XorChainFirstFunction:
	ld hl, 001C69Fh
	call InsertHL						; sub a, 1 \ sbc a, a
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	jr XorInsert
XorNumberXXX:
	ld a, d
	cp typeNumber
	jr z, XorNumberNumber
	cp typeVariable
	jr z, XorNumberVariable
	cp typeFunction
	jr z, XorNumberFunction
XorNumberChain:
	ld a, c
	ld c, e
	ld e, a
	jr XorChainFirstNumber
XorNumberNumber:
	ld a, c
	sub a, 1
	sbc a, a
	ld b, a
	ld a, e
	add a, 255
	sbc a, a
	xor b
	inc a
	inc hl
	ld (hl), a
	res chain_operators, (iy+myFlags)
	ret
XorNumberVariable:
	ld a, c
	ld c, e
	ld e, a
	jr XorVariableNumber
XorNumberFunction:
	ld a, e
	call GetFunction
	ld e, c
	jr XorChainFirstNumber
XorVariableXXX:
	ld a, d
	cp typeNumber
	jr z, XorVariableNumber
	cp typeVariable
	jr z, XorVariableVariable
	cp typeFunction
	jr z, XorVariableFunction
XorVariableChain:
	ld e, c
	jr XorChainFirstVariable
XorVariableNumber:
	call InsertAIXC						; ld a, (ix+*)
	jr XorChainFirstNumber
XorVariableVariable:
	ld a, c
	cp e
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	call InsertAIXC						; ld a, (ix+*)
	ld hl, 09F01D6h
	call InsertHL						; sub a, 1 \ sbc a, a
	ld a, 047h
	call InsertA						; ld b, a
	call InsertAIXE						; ld a, (ix+*)
XorInsert:
	ld a, 0C6h
	call InsertA						; add a, **
	ld a, 0FFh
	call InsertA						; add a, 255
	ld hl, 03CA89Fh
	jp InsertHL							; sbc a, a \ xor b \ inc a
XorVariableFunction:
	ld a, e
	call GetFunction
	ld hl, 09F01D6h
	call InsertHL						; sub a, 1 \ sbc a, a
	ld a, 047h
	call InsertA						; ld b, a
	call InsertAIXE						; ld a, (ix+*)
	jr XorInsert
XorFunctionXXX:
	ld a, c
	ld c, e
	ld e, a
	ld a, d
	cp typeNumber
	jr z, XorNumberFunction
	cp typeVariable
	jr z, XorVariableFunction
	cp typeChain
	jp z, XorChainFirstFunction
XorFunctionFunction:
	ld a, c
	call GetFunction
	ld hl, 09F01D6h
	call InsertHL						; sub a, 1 \ sbc a, a
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	jr XorInsert
functionGE:
	ld a, b
	cp typeNumber
	jp z, GENumberXXX
	cp typeVariable
	jp z, GEVariableXXX
	cp typeFunction
	jp z, GEFunctionXXX
GEChainXXX:
	ld a, c
	cp ChainFirst
	jr z, GEChainFirstXXX
GEChainPushXXX:
	ld a, d
	cp typeNumber
	jr z, GEChainPushNumber
	cp typeVariable
	jr z, GEChainPushVariable
	cp typeFunction
	jr z, GEChainPushFunction
GEChainPushChain:
	ld a, 047h
	call InsertA						; ld b, a
	ld a, 0F1h
	call InsertA						; pop af
	xor a
	ld hl, 03C9F90h
	jp InsertHL							; sub a, b \ sbc a, a \ inc a
GEChainPushNumber:
	ld a, 0F1h
	call InsertA						; pop af
	jr GEChainFirstNumber
GEChainPushVariable:
	ld a, 0F1h
	call InsertA						; pop af
	jr GEChainFirstVariable
GEChainPushFunction:
	ld a, e
	call GetFunction
	ld a, 047h
	call InsertA						; ld a, b
	ld a, 0F1h
	call InsertA						; pop af
	ld hl, 03C9F90h
	jp InsertHL							; sub a, b \ sbc a, a \ inc a
GEChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, GEChainFirstNumber
	cp typeFunction
	jr z, GECHainFirstFunction
GEChainFirstVariable:
	ld a, 0DDh
	call InsertA						; sub a, (ix+*) (1)
	ld a, 096h
	call InsertA						; sub a, (ix+*) (2)
	ld hl, 03C9F00h
	ld l, e
	jp InsertHL							; sub a, (ix+*) (3) \ sbc a, a \ inc a
GEChainFirstNumber:
	ld hl, 09F00D6h
	ld h, e
	call InsertHL						; sub a, XX \ sbc a, a
	ld a, 03Ch
	jp InsertA							; inc a
GECHainFirstFunction:
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	ld hl, 03F9837h
	call InsertHL						; scf \ sbc a, b \ ccf
	ld a, 09Fh
	call InsertA						; sbc a, a
	ld a, 03Ch
	jp InsertA							; inc a
GENumberXXX:
	ld a, d
	cp typeNumber
	jr z, GENumberNumber
	cp typeVariable
	jr z, GENumberVariable
	cp typeFunction
	jr z, GENumberFunction
GENumberChain:
	ld e, c
	jp LEChainFirstNumber
GENumberNumber:
	ld a, c
	sub a, e
	sbc a, a
	inc a
	inc hl
	ld (hl), a
	res chain_operators, (iy+myFlags)
	ret
GENumberVariable:
	ld a, c
	ld c, e
	ld e, a
	jp LEVariableNumber
GENumberFunction:
	ld a, c
	ld c, e
	ld e, a
	jp LEFunctionNumber
GEVariableXXX:
	ld a, d
	cp typeNumber
	jr z, GEVariableNumber
	cp typeVariable
	jr z, GEVariableVariable
	cp typeFunction
	jr z, GEVariableFunction
GEVariableChain:
	ld e, c
	jp LEChainFirstVariable
GEVariableNumber:
	ld a, e
	or a
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 1
	res chain_operators, (iy+myFlags)
	ret
_:	call InsertAIXC						; ld a, (ix+*)
	jr GEChainFirstNumber
GEVariableVariable:
	ld a, c
	cp e
	jr nz, +_
	ld e, 1
	jp OrVariableNumber
_:	call InsertAIXC						; ld a, (ix+*)
	jp GEChainFirstVariable
GEVariableFunction:
	ld a, e
	call GetFunction
	ld a, 047h
	call InsertA						; ld b, a
	call InsertAIXC						; ld a, (ix+*)
	ld hl, 03C9F90h
	jp InsertHL							; sub a, b \ sbc a, a \ inc a
GEFunctionXXX:
	ld a, d
	cp typeNumber
	jr z, GEFunctionNumber
	cp typeVariable
	jr z, GEFunctionVariable
	cp typeFunction
	jr z, GEFunctionFunction
GEFunctionChain:
	ld e, c
	jp LEChainFirstFunction
GEFunctionNumber:
	ld a, e
	or a
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 1
	res chain_operators, (iy+myFlags)
	ret
_:	ld a, c
	call GetFunction
	jp GEChainFirstNumber
GEFunctionVariable:
	ld a, c
	call GetFunction
	jp GEChainFirstVariable
GEFunctionFunction:
	ld a, e
	call GetFunction
	ld a, 047h
	call InsertA						; ld b, a
	ld a, c
	call GetFunction
	ld hl, 03C9F90h
	jp InsertHL							; sub a, b \ sbc a, a \ inc a
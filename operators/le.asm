functionLE:
	ld a, b
	cp typeNumber
	jp z, LENumberXXX
	cp typeVariable
	jp z, LEVariableXXX
	cp typeFunction
	jp z, LEFunctionXXX
LEChainXXX:
	ld a, c
	cp ChainFirst
	jr z, LEChainFirstXXX
LEChainPushXXX:
	ld a, d
	cp typeNumber
	jr z, LEChainPushNumber
	cp typeVariable
	jr z, LEChainPushVariable
	cp typeFunction
	jr z, LEChainPushFunction
LEChainPushChain:
	ld a, 0C1h
	call InsertA						; pop bc
	ld hl, 03C9F90h
	jp InsertHL							; sub a, b \ sbc a, a \ inc a
LEChainPushNumber:
	ld a, 0F1h
	call InsertA						; pop af
	jr LEChainFirstNumber
LEChainPushVariable:
	ld a, 0F1h
	call InsertA						; pop af
	jr LEChainFirstVariable
LEChainPushFunction:
	ld a, 0C1h
	call InsertA						; pop bc
	ld a, e
	call GetFunction
	ld hl, 03C9F90h
	jp InsertHL							; sub a, b \ sbc a, a \ inc a
LEChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, LEChainFirstNumber
	cp typeFunction
	jr z, LEChainFirstFunction
LEChainFirstVariable:
	ld a, 0DDh
	call InsertA						; sub a, (ix+*) (1)
	ld hl, 09F0096h
	ld h, e
	call InsertHL						; sub a, (ix+*) \ sbc a, a
	ld a, 0E6h
	call InsertA						; and a, *
	ld a, 1
	jp InsertA							; and a, 1
LEChainFirstNumber:
	ld a, 047h
	call InsertA						; ld b, a
	ld hl, 090003Eh
	ld h, e
	call InsertHL						; ld a, XX \ sub a, b
	ld a, 09Fh
	call InsertA						; sbc a, a
	ld a, 03Ch
	jp InsertA							; inc a
LEChainFirstFunction:
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	ld hl, 03C9F90h
	jp InsertHL							; sub a, b \ sbc a, a \ inc a
LENumberXXX:
	ld a, d
	cp typeNumber
	jr z, LENumberNumber
	cp typeVariable
	jr z, LENumberVariable
	cp typeFunction
	jr z, LENumberFunction
LENumberChain:
	ld e, c
	jp GEChainFirstNumber
LENumberNumber:
	ld a, c
	cp e
	ld a, 0
	jr nc, +_
	inc a
_:	inc hl
	ld (hl), a
	res chain_operators, (iy+myFlags)
	ret
LENumberVariable:
	ld a, e
	ld e, c
	ld c, a
	jp GEVariableNumber
LENumberFunction:
	ld a, c
	ld c, e
	ld e, a
	jp GEFunctionNumber
LEVariableXXX:
	ld a, d
	cp typeNumber
	jr z, LEVariableNumber
	cp typeVariable
	jr z, LEVariableVariable
	cp typeFunction
	jr z, LEVariableFunction
LEVariableChain:
	ld e, c
	jp GEChainFirstVariable
LEVariableNumber:
	ld a, e
	cp 255
	jr nz, +_
	ld a, 03Eh
	call InsertA						; ld a, **
	ld a, 1
	jp InsertA							; ld a, 1
_:	ld hl, 0DD003Eh
	ld h, e
	call InsertHL						; ld a, XX \ sub a, (ix+*) (1)
	ld hl, 09F0096h
	ld h, c
	call InsertHL						; sub a, (ix+*) \ sbc a, a
	ld a, 03Ch
	jp InsertA							; inc a
LEVariableVariable:
	ld a, c
	cp e
	jr nz, +_
	ld e, 1
	jp OrVariableNumber
_:	ld a, c
	ld c, e
	ld e, a
	jp GEVariableVariable
LEVariableFunction:
	ld a, c
	ld c, e
	ld e, a
	jp GEFunctionVariable
LEFunctionXXX:
	ld a, d
	cp typeNumber
	jr z, LEFunctionNumber
	cp typeVariable
	jr z, LEFunctionVariable
	cp typeFunction
	jr z, LEFunctionFunction
LEFunctionChain:
	ld e, c
	jp LEChainFirstFunction
LEFunctionNumber:
	ld a, e
	cp 255
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 1
	res chain_operators, (iy+myFlags)
	ret
_:	ld a, c
	call GetFunction
	jp LEChainFirstNumber
LEFunctionVariable:
	ld a, c
	call GetFunction
	jp LEChainFirstVariable
LEFunctionFunction:
	ld a, c
	call GetFunction
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	ld hl, 03C9F90h
	jp InsertHL							; sub a, b \ sbc a, a \ inc a
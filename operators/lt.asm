functionLT:
	ld a, b
	cp typeNumber
	jp z, LTNumberXXX
	cp typeVariable
	jp z, LTVariableXXX
	cp typeFunction
	jp z, LTFunctionXXX
LTChainXXX:
	ld a, c
	cp ChainFirst
	jr z, LTChainFirstXXX
LTChainPushXXX:
	ld a, d
	cp typeNumber
	jr z, LTChainPushNumber
	cp typeVariable
	jr z, LTChainPushVariable
	cp typeFunction
	jr z, LTCHainPushFunction
LTChainPushChain:
	ld a, 0C1h
	call InsertA						; pop bc
	ld a, 037h
	call InsertA						; scf
	ld hl, 03C9F98h
	jp InsertHL							; sbc a, b \ sbc a, a \ inc a
LTChainPushNumber:
	ld a, 0F1h
	call InsertA						; pop af
	jr LTChainFirstNumber
LTChainPushVariable:
	ld a, 0F1h
	call InsertA						; pop af
	jr LTChainFirstVariable
LTCHainPushFunction:
	ld a, 0C1h
	call InsertA						; pop bc
	ld a, e
	call GetFunction
	ld a, 037h
	call InsertA						; scf
	ld hl, 03C9F98h
	jp InsertHL							; sbc a, b \ sbc a, a \ inc a
LTChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, LTChainFirstNumber
	cp typeFunction
	jr z, LTChainFirstFunction
LTChainFirstVariable:
	ld a, 0DDh
	call InsertA						; sub a, (ix+*) (1)
	ld a, 096h
	call InsertA						; sub a, (ix+*) (2)
	ld a, e
	call InsertA						; sub a, (ix+*) (3)
	ld hl, 01E69Fh
	jp InsertHL							; sbc a, a \ and a, 1
LTChainFirstNumber:
	ld a, e
	or a
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	dec a								; 256-a
	cpl
	ld hl, 09F00C6h
	ld h, e
	call InsertHL						; add a, XX \ sbc a, a
	ld a, 03Ch
	jp InsertA							; inc a
LTChainFirstFunction:
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	ld hl, 03C9837h
	jp InsertHL							; scf \ sbc a, b \ inc a
LTNumberXXX:
	ld a, d
	cp typeNumber
	jr z, LTNumberNumber
	cp typeVariable
	jr z, LTNumberVariable
	cp typeFunction
	jr z, LTNumberFunction
LTNumberChain:
	ld e, c
	jp GTChainFirstNumber
LTNumberNumber:
	ld a, c
	cp e
	ld a, 0
	jr nc, +_
	inc a
_:	inc hl
	ld (hl), a
	res chain_operators, (iy+myFlags)
	ret
LTNumberVariable:
	ld a, c
	ld c, e
	ld e, a
	jp GTVariableNumber
LTNumberFunction:
	ld a, e
	call GetFunction
	ld e, c
	jp GTChainFirstNumber
LTVariableXXX:
	ld a, d
	cp typeNumber
	jr z, LTVariableNumber
	cp typeVariable
	jr z, LTVariableVariable
	cp typeFunction
	jr z, LTVariableFunction
LTVariableChain:
	ld e, c
	jp GTChainFirstVariable
LTVariableNumber:
	ld a, e
	or a
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	call InsertAIXC						; ld a, (ix+*)
	jp LTChainFirstNumber
LTVariableVariable:
	ld a, c
	cp e
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	call InsertAIXC						; ld a, (ix+*)
	jp LTChainFirstVariable
LTVariableFunction:
	ld a, e
	call GetFunction
	ld e, c
	jp GTChainFirstVariable
LTFunctionXXX:
	ld a, c
	cp typeNumber
	jr z, LTFunctionNumber
	cp typeVariable
	jr z, LTFunctionVariable
	cp typeChain
	jr z, LTFunctionChain
LTFunctionFunction:
	ld a, c
	call GetFunction
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	jr LTInsert
LTFunctionNumber:
	ld a, c
	call GetFunction
	jp LTChainFirstNumber
LTFunctionVariable:
	ld a, c
	call GetFunction
	jp LTChainFirstVariable
LTFunctionChain:
	ld a, 047h
	call InsertA						; ld b, a
	ld a, c
LTInsert:
	call GetFunction
	ld a, 037h
	call InsertA						; scf
	ld hl, 03C9F98h
	jp InsertHL							; sbc a, b \ sbc a, a \ inc a
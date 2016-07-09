functionMul:
	ld a, b
	cp typeNumber
	jp z, MulNumberXXX
	cp typeVariable
	jp z, MulVariableXXX
	cp typeFunction
	jp z, MulFunctionXXX
MulChainXXX:
	ld a, c
	cp ChainFirst
	jr z, MulChainFirstXXX
MulChainPushXXX:
	ld a, d
	cp typeNumber
	jr z, MulChainPushNumber
	cp typeVariable
	jr z, MulChainPushVariable
	cp typeFunction
	jr z, MulChainPushFunction
MulChainPushChain:
	ld a, 0E1h
	call InsertA						; pop hl
	ld a, 06Fh
	call InsertA						; ld l, a
	jp MulInsert
MulChainPushNumber:
	ld a, 0F1h
	call InsertA						; pop af
	ld c, e
	jr MulNumberChain
MulChainPushVariable:
	ld a, 0E1h
	call InsertA						; pop hl
	call InsertLIXE						; ld l, (ix+*)
	jp MulInsert
MulChainPushFunction:
	ld a, 0E1h
	call InsertA						; pop hl
	ld a, e
	call GetFunction
	ld a, 06Fh
	call InsertA						; ld l, a
	jp MulInsert
MulChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, MulChainFirstNumber
	cp typeFunction
	jr z, MulChainFirstFunction
MulChainFirstVariable:
	ld c, e
	ld e, 0
	jp MulVariableChain
MulChainFirstNumber:
	ld c, e
	jr MulNumberChain
MulChainFirstFunction:
	ld a, 067h
	call InsertA						; ld h, a
	ld a, e
	call GetFunction
	ld a, 06Fh
	call InsertA						; ld l, a
	jp MulInsert
MulNumberXXX:
	ld a, d
	cp typeNumber
	jr z, MulNumberNumber
	cp typeVariable
	jr z, MulNumberVariable
	cp typeFunction
	jr z, MulNumberFunction
MulNumberChain:
	ld a, c
	or a
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	cp 2
	ret c
	cp 25
	jr nc, +_
	ld b, 7
	dec c
	dec c
	mlt bc
	ld hl, mul_table
	add hl, bc
	ex de, hl
MulLoop:
	ld a, (de)
	inc de
	or a
	ret z
	call InsertA
	jr MulLoop
_:	cp 32
	jr nz, +_
	ld hl, 0878787h
	call InsertHL						; add a, a \ add a, a \ add a, a
	ld a, 87h
	call InsertA						; add a, a
	jp InsertA							; add a, a
_:	cp 64
	jr nz, +_
	ld hl, 0E60F0Fh
	call InsertHL						; rrca \ rrca \ and a, **
	ld a, 00111111b
	jp InsertA							; and a, %00111111
_:	ld hl, 6F0026h
	ld h, a
	call InsertHL						; ld h, XX \ ld l, a
	jr MulInsert
MulNumberNumber:
	ld b, e
	mlt bc
	inc hl
	ld (hl), c
	res chain_operators, (iy+myFlags)
	ret
MulNumberVariable:
	ld a, c
	ld c, e
	ld e, a
	jr MulVariableNumber
MulNumberFunction:
	ld a, e
	call GetFunction
	ld a, 06Fh
	call InsertA						; ld l, a
	ld a, 026h
	call InsertA						; ld h, **
	ld a, c
	call InsertA						; ld h, XX
	jr MulInsert
MulVariableXXX:
	ld a, d
	cp typeNumber
	jr z, MulVariableNumber
	cp typeVariable
	jr z, MulVariableVariable
	cp typeFunction
	jr z, MulVariableFunction
MulVariableChain:
	ld a, 067h
	call InsertA						; ld h, a
	ld a, c
	cp e
	jr z, +_
	call InsertLIXC						; ld l, (ix+*)
	jr MulInsert
_:	ld a, 06Fh
	call InsertA						; ld l, a
MulInsert:
	ld hl, 07D6CEDh
	jp InsertHL							; mlt hl \ ld a, l
MulVariableNumber:
	ld a, e
	or a
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	call InsertAIXC						; ld a, (ix+*)
	ld c, e
	jp MulNumberChain
MulVariableVariable:
	call InsertLIXC						; ld l, (ix+*)
	ld a, 0DDh
	call InsertA						; ld h, (ix+*) (1)
	ld a, 066h
	call InsertA						; ld h, (ix+*) (2)
	ld a, e
	sub 011h
	call InsertA						; ld h, (ix+*) (3)
	jr MulInsert
MulVariableFunction:
	ld a, e
	call GetFunction
	ld a, 067h
	call InsertA						; ld h, a
	call InsertLIXC						; ld l, (ix+*)
	jr MulInsert
MulFunctionXXX:
	ld a, c
	ld c, e
	ld e, a
	ld a, c
	cp typeNumber
	jp z, MulNumberFunction
	cp typeVariable
	jr z, MulVariableFunction
	cp typeChain
	jp z, MulChainFirstFunction
MulFunctionFunction:
	ld a, c
	call GetFunction
	ld a, 06Fh
	call InsertA						; ld l, a
	ld a, e
	call GetFunction
	ld a, 067h
	call InsertA						; ld h, a
	jr MulInsert
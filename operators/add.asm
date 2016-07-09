functionAdd:
	ld a, b
	cp typeNumber
	jp z, AddNumberXXX
	cp typeVariable
	jp z, AddVariableXXX
	cp typeFunction
	jp z, AddFunctionXXX
AddChainXXX:
	ld a, c
	cp ChainFirst
	jr z, AddChainFirstXXX
AddChainPushXXX:
	ld a, d
	cp typeNumber
	jr z, AddChainPushNumber
	cp typeVariable
	jr z, AddChainPushVariable
	cp typeFunction
	jr z, AddChainPushFunction
AddChainPushChain:
	ld a, 0C1h
	call InsertA						; pop bc
	ld a, 080h
	jp InsertA							; add a, b
AddChainPushNumber:
	ld a, 0F1h
	call InsertA						; pop af
	jr AddChainFirstNumber
AddChainPushVariable:
	ld a, 0F1h
	call InsertA						; pop af
	jr AddChainFirstVariable
AddChainPushFunction:
	ld a, 0C1h
	call InsertA						; pop bc
	ld a, e
	call GetFunction
	ld a, 080h
	jp InsertA							; add a, b
AddChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, AddChainFirstNumber
	cp typeFunction
	jr z, AddChainFirstFunction
AddChainFirstVariable:
	ld a, 0DDh
	call InsertA						; add a, (ix+*) (1)
	ld a, 086h
	call InsertA						; add a, (ix+*) (2)
	ld a, e
	jp InsertA							; add a, (ix+*) (3)
AddChainFirstNumber:
	ld a, e
	or a
	ret z
	cp 1
	jr nz, +_
	ld a, 03Ch
	jp InsertA							; inc a
_:	cp 255
	jr nz, +_
	ld a, 03Dh
	jp InsertA							; dec a
_:	ld a, 0C6h
	call InsertA						; add a, **
	ld a, e
	jp InsertA							; add a, XX
AddChainFirstFunction:
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	ld a, 080h
	jp InsertA							; add a, b
AddNumberXXX:
	ld a, d
	cp typeNumber
	jr z, AddNumberNumber
	cp typeVariable
	jr z, AddNumberVariable
	cp typeFunction
	jr z, AddNumberFunction
AddNumberChain:
	ld e, c
	jr AddChainFirstNumber
AddNumberNumber:
	ld a, c
	add a, e
	inc hl
	ld (hl), a
	res chain_operators, (iy+myFlags)
	ret
AddNumberVariable:
	call InsertAIXE						; ld a, (ix+*)
	ld e, c
	jr AddChainFirstNumber
AddNumberFunction:
	ld a, e
	call GetFunction
	ld e, c
	jr AddChainFirstNumber
AddVariableXXX:
	ld a, d
	cp typeNumber
	jr z, AddVariableNumber
	cp typeVariable
	jr z, AddVariableVariable
	cp typeFunction
	jr z, AddVariableFunction
AddVariableChain:
	ld e, c
	jp AddChainFirstVariable
AddVariableNumber:
	ld a, c
	ld c, e
	ld e, a
	jr AddNumberVariable
AddVariableVariable:
	call InsertAIXC						; ld a, (ix+*)
	ld a, c
	cp e
	jp nz, AddChainFirstVariable
	ld a, 087h
	jp InsertA							; add a, a
AddVariableFunction:
	ld a, e
	call GetFunction
	ld e, c
	jp AddChainFirstVariable
AddFunctionXXX:
	ld a, c
	ld c, e
	ld e, a
	ld a, d
	cp typeNumber
	jr z, AddNumberFunction
	cp typeVariable
	jr z, AddVariableFunction
	cp typeChain
	jp z, AddChainFirstFunction
AddFunctionFunction:
	ld a, c
	call GetFunction
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	ld a, 080h
	jp InsertA							; add a, b
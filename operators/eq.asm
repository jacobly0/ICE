functionEq:
	ld a, b
	cp typeNumber
	jp z, EqNumberXXX
	cp typeVariable
	jp z, EqVariableXXX
	cp typeFunction
	jp z, EqFunctionXXX
EqChainXXX:
	ld a, c
	cp ChainFirst
	jr z, EqChainFirstXXX
EqChainPushXXX:
	ld a, d
	cp typeNumber
	jr z, EqChainPushNumber
	cp typeVariable
	jr z, EqChainPushVariable
	cp typeFunction
	jr z, EqChainPushFunction
EqChainPushChain:
	ld a, 0C1h
	call InsertA						; pop bc
	ld a, 090h
	call InsertA						; sub a, b
	jp EqInsert
EqChainPushNumber:
	ld a, 0F1h
	call InsertA						; pop af
	jr EqChainFirstNumber
EqChainPushVariable:
	ld a, 0F1h
	call InsertA						; pop af
	jr EqChainFirstVariable
EqChainPushFunction:
	ld a, e
	call GetFunction
	ld a, 0C1h
	call InsertA						; pop bc
	ld a, 090h
	call InsertA						; sub a, b
	jp EqInsert
EqChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, EqChainFirstNumber
	cp typeFunction
	jr z, EqChainFirstFunction
EqChainFirstVariable:
	ld a, 0DDh
	call InsertA						; sub a, (ix+*) (1)
	ld a, 096h
	call InsertA						; sub a, (ix+*) (2)
	ld a, e
	call InsertA						; sub a, (ix+*) (3)
	jp EqInsert
EqChainFirstNumber:
	ld a, e
	or a
	jr z, EqInsert2
	cp 1
	jr nz, +_
	ld a, 03Dh
	call InsertA						; dec a
	jr EqInsert2
_:	cp 255
	jr nz, +_
	ld a, 03Ch
	call InsertA						; inc a
	jr EqInsert2
_:	ld a, 0D6h
	call InsertA						; sub a, *
	ld a, e
	call InsertA						; sub a, XX
EqInsert2:
	jp EqInsert
EqChainFirstFunction:
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	ld a, 080h
	call InsertA						; sub a, b
	jr EqInsert
EqNumberXXX:
	ld a, d
	cp typeNumber
	jr z, EqNumberNumber
	cp typeVariable
	jr z, EqNumberVariable
	cp typeFunction
	jr z, EqNumberFunction
EqNumberChain:
	ld e, c
	jr EqChainFirstNumber
EqNumberNumber:
	ld a, c
	cp e
	ld a, 0
	jr nz, $+3
	inc a
	inc hl
	ld (hl), a
	res chain_operators, (iy+myFlags)
	ret
EqNumberVariable:
	ld a, c
	ld c, e
	ld e, a
	jr EqVariableNumber
EqNumberFunction:
	ld a, e
	call GetFunction
	ld a, 0D6h
	call InsertA						; sub a, *
	ld a, c
	call InsertA						; sub a, XX
	jr EqInsert
EqVariableXXX:
	ld a, d
	cp typeNumber
	jr z, EqVariableNumber
	cp typeVariable
	jr z, EqVariableVariable
	cp typeFunction
	jr z, EqVariableFunction
EqVariableChain:
	ld e, c
	jp EqChainFirstVariable
EqVariableNumber:
	call InsertAIXC						; ld a, (ix+*)
	jp EqChainFirstNumber
EqVariableVariable:
	ld a, c
	cp e
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 1
	res chain_operators, (iy+myFlags)
	ret
_:	call InsertAIXC						; ld a, (ix+*)
	jp EqChainFirstVariable
EqVariableFunction:
	ld a, e
	call GetFunction
	ld a, 0DDh
	call InsertA						; sub a, (ix+*) (1)
	ld a, 096h
	call InsertA						; sub a, (ix+*) (2)
	ld a, c
	call InsertA						; sub a, (ix+*) (3)
EqInsert:
	ld a, 0C6h
	call InsertA						; add a, *
	ld hl, 03C9FFFh
	jp InsertHL							; add a, 255 \ sbc a, a \ inc a
EqFunctionXXX:
	ld a, c
	ld c, e
	ld e, a
	ld a, d
	cp typeNumber
	jr z, EqNumberFunction
	cp typeVariable
	jr z, EqVariableFunction
	cp typeChain
	jp z, EqChainFirstFunction
EqFunctionFunction:
	ld a, c
	call GetFunction
	jp EqChainFirstFunction
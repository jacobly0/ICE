functionGT:
	ld a, b
	cp typeNumber
	jp z, GTNumberXXX
	cp typeVariable
	jp z, GTVariableXXX
	cp typeFunction
	jp z, GTFunctionXXX
GTChainXXX:
	ld a, c
	cp ChainFirst
	jr z, GTChainFirstXXX
GTChainPushXXX:
	ld a, d
	cp typeNumber
	jr z, GTChainPushNumber
	cp typeVariable
	jr z, GTChainPushVariable
	cp typeFunction
	jr z, GTChainPushFunction
GTChainPushChain:
	ld hl, 037F147h
	call InsertHL						; ld b, a \ pop af \ scf
	ld hl, 03C9F98h
	jp InsertHL							; sbc a, b \ sbc a, a \ inc a
GTChainPushNumber:
	ld a, 0F1h
	call InsertA						; pop af
	jr GTChainFirstNumber
GTChainPushVariable:
	ld a, 0F1h
	call InsertA						; pop af
	jr GTChainFirstVariable
GTChainPushFunction:
	ld a, e
	call GetFunction
	jr GTChainPushChain
GTChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, GTChainFirstNumber
	cp typeFunction
	jr z, GTChainFirstFunction
GTChainFirstVariable:
	ld hl, 09EDD37h
	call InsertHL						; scf \ sbc a, (ix+*) (1)
	ld a, e
	call InsertA						; sbc a, (ix+*)
	ld a, 09Fh
	call InsertA						; sbc a, a
	ld a, 03Ch
	jp InsertA							; inc a
GTChainFirstNumber:
	ld a, e
	cp 255
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	ld a, 0D6h
	call InsertA						; sub a, *
	ld hl, 03C9F00h
	ld l, e
	inc l
	jp InsertHL							; sub a, X+1 \ sbc a, a \ inc a
GTChainFirstFunction:
	ld a, 04Fh
	call InsertA						; ld c, a
	ld a, e
	call GetFunction
	ld a, 047h
	call InsertA						; ld b, a
	ld a, 079h
	call InsertA						; ld a, c
	ld hl, 03C9F98h
	jp InsertHL							; sbc a, b \ sbc a, a \ inc a
GTNumberXXX:
	ld a, d
	cp typeNumber
	jr z, GTNumberNumber
	cp typeVariable
	jr z, GTNumberVariable
	cp typeFunction
	jr z, GTNumberFunction
GTNumberChain:
	ld e, c
	jp LTChainFirstNumber
GTNumberNumber:
	ld a, c
	cp e
	ld a, 0
	jr z, +_
	jr c, +_
	inc a
_:	inc hl
	ld (hl), a
	res chain_operators, (iy+myFlags)
	ret
GTNumberVariable:
	ld a, c
	ld c, e
	ld e, a
	jp LTVariableNumber
GTNumberFunction:
	ld a, c
	ld c, e
	ld e, a
	jp LTFunctionNumber
GTVariableXXX:
	ld a, d
	cp typeNumber
	jr z, GTVariableNumber
	cp typeVariable
	jr z, GTVariableVariable
	cp typeFunction
	jr z, GTVariableFunction
GTVariableChain:
	ld e, c
	jp LTChainFirstVariable
GTVariableNumber:
	call InsertAIXC						; ld a, (ix+*)
	jp GTChainFirstNumber
GTVariableVariable:
	ld a, c
	cp e
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	call InsertAIXC						; ld a, (ix+*)
	jp GTChainFirstVariable
GTVariableFunction:
	call InsertAIXC						; ld a, (ix+*)
	jp GTChainFirstFunction
GTFunctionXXX:
	ld a, c
	ld c, e
	ld e, a
	ld a, d
	cp typeNumber
	jp z, LTNumberFunction
	cp typeVariable
	jp z, LTVariableFunction
	cp typeChain
	jp z, LTChainFirstFunction
GTFunctionFunction:
	ld a, c
	call GetFunction
	ld a, 047h
	call InsertA						; ld b, a
	ld a, e
	call GetFunction
	ld a, 037h
	call InsertA						; scf
	ld hl, 03C9F98h
	jp InsertHL							; sbc a, b \ sbc a, a \ inc a
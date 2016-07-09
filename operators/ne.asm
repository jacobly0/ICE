functionNE:
	ld a, b
	cp typeNumber
	jr z, NENumberXXX
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
NEChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, NEChainFirstNumber
NEChainFirstVariable:
	ld a, 021h
	call InsertA						; ld hl, ******
	call InsertVariableE				; ld hl, XXXXXX
	ld a, 096h
	call InsertA						; sub a, (hl)
	jr NEInsert
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
_:	jr NEInsert
NENumberXXX:
	ld a, d
	cp typeNumber
	jr z, NENumberNumber
	cp typeVariable
	jr z, NENumberVariable
NENumberChain:
	ld e, c
	jr NEChainFirstNumber
NENumberNumber:
	ld a, c
	cp e
	ld a, 0
	jr z, $+3
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
NEVariableXXX:
	ld a, d
	cp typeNumber
	jr z, NEVariableNumber
	cp typeVariable
	jr z, NEVariableVariable
NEVariableChain:
	ld e, c
	jr NEChainFirstVariable
NEVariableNumber:
	ld a, 03Ah
	call InsertA						; ld a, (******)
	call InsertVariableC				; ld a, (XXXXXX)
	jr NEChainFirstNumber
NEVariableVariable:
	ld a, c
	cp e
	ld a, 0AFh
	jp z, InsertA						; xor a
	ld a, 03Ah
	call InsertA						; ld a, (******)
	call InsertVariableC				; ld a, (XXXXXX)
	jp NEChainFirstVariable
NEInsert:
	ld a, 0D6h
	call InsertA						; sub a, *
	ld hl, 03C9F01h
	jp InsertHL							; sub a, 1 \ sbc a, a \ inc a
NEFunctionXXX:
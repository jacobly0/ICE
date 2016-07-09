functionSto:
	ld a, d
	cp typeVariable
	jr nz, StoError
	ld a, b
	cp typeNumber
	jr z, StoNumberVariable
	cp typeVariable
	jr z, StoVariableVariable
	cp typeFunction
	jr z, StoFunctionVariable
StoChainXXX:
	ld a, c
	cp ChainFirst
	jr z, StoChainFirstVariable
StoChainPushVariable:
	ld a, 0F1h
	call InsertA						; pop af
	jr StoChainFirstVariable
StoFunctionVariable:
	ld a, c
	call GetFunction
	jr StoChainFirstVariable
StoVariableVariable:
	call InsertAIXC						; ld a, (ix+*)
StoChainFirstVariable:
	ld a, 0DDh
	call InsertA						; ld (ix+*), a (1)
	ld a, 077h
	call InsertA						; ld (ix+*), a (2)
	ld a, e
	jp InsertA							; ld (ix+*), a (3)
StoNumberVariable:
	ld a, c
	or a
	jr nz, +_
	ld a, 0AFh
	call InsertA
	jr StoChainFirstVariable
_:	ld a, 03Eh
	call InsertA
	ld a, c
	call InsertA
	jr StoChainFirstVariable
StoError:
			pop hl
		pop hl
	pop hl
	ld hl, StoErrorMessage
	jp DisplayErrorMessage
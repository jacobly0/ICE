ParseExpression:
	res prev_is_number, (iy+myFlags)						; previous token was not a number
	ld hl, output
	ld (hl), 0
	push hl
	pop de
	inc de
	ld bc, 1700
	ldir
	ld hl, stack
	ld (stackPtr), hl
	ld hl, output
	ld (outputPtr), hl
	call _CurFetch
MainLoop2:
	jp c, StopReading
	cp t0
	jr c, NotNumber
	cp t9+1
	jr nc, NotNumber
	ld b, 0
	jr NumberOrVariable
NotNumber:
	cp tA
	jr c, NotVariable
	cp tTheta+1
	jr nc, NotVariable
	ld b, typeVariable-typeNumber
NumberOrVariable:
	#include "number.asm"
	call _IncFetch
	jp MainLoop2
NotVariable:
	res prev_is_number, (iy+myFlags)
	ld hl, operators_booleans
	ld bc, 14
	cpir
	jr nz, NotBoolean
	#include "operator.asm"
	call _IncFetch
	jp MainLoop2
NotBoolean:
	cp tComma
	jr z, CommaOrRParen
	cp tRParen
	jr nz, NotCommaOrRParen
CommaOrRParen:
	#include "closing.asm"
	call _IncFetch
	jp MainLoop2
NotCommaOrRParen:
	cp tEnter
	jr z, StopReading
	ld hl, (outputPtr)										; we didn't reached a new line
	ld (hl), typeFunction									; push token on stack
	inc hl
	ld (hl), a
	inc hl
	ld (outputPtr), hl
	call _IncFetch
	jp MainLoop2
; Move stack to output
StopReading:
	ld hl, (stackPtr)
	ld bc, stack
	or a
	sbc hl, bc
	push hl
		ld hl, (outputPtr)
		ld bc, output
		sbc hl, bc
	pop bc
	add hl, bc
	push hl
		ld hl, (outputPtr)
		ex de, hl
		ld hl, (stackPtr)
MoveStackEntryToOutput:
		dec hl
		dec hl
		ld a, b
		or c
		jr z, StopMoving
		ldi
		ldi
		dec hl
		dec hl
		jr MoveStackEntryToOutput
StopMoving:
		ld hl, output+4
	pop bc
loop:
	set chain_operators, (iy+myFlags)
	ld a, b
	or c
	cp 2
	jp z, CheckIfSingleOperand
	ld a, (hl)					; a = element type
	or a
	ret z
	cp typeOperator
	jp nz, notOperator
	inc hl
	ld a, (hl)
	dec hl
	push hl
		push bc
			push hl
				ld hl, operators_booleans
				ld bc, 14
				cpir
				ld b, 3
				mlt bc
				ld hl, operator_start
				add hl, bc
				ld hl, (hl)
				ld (SMC_1), hl
			pop hl
			dec hl
			ld e, (hl)
			dec hl
			ld d, (hl)
			dec hl
			ld c, (hl)
			dec hl
			ld b, (hl)
SMC_1 = $+1
			call 0000000h
		pop bc
	pop hl
	push hl
	pop de
	; hl = type current entry
	inc hl
	inc hl
	dec de
	dec de
	; hl = type next entry
	; de = type previous entry
	; remove current entry
	push de
		push bc
			ldir
		pop bc
		dec bc
		dec bc
		dec bc
		dec bc
	pop hl
	bit chain_operators, (iy+myFlags)
	jp z, loop
	ld a, b
	or c
	cp 2
	ret z
	ld a, (hl)
	or a
	ret z
	cp typeOperator
	jr nz, +_
	dec hl
	ld (hl), ChainLast
	dec hl
	ld (hl), typeChain
	jr stopChain
_:	inc hl
	inc hl
	ld a, (hl)
	dec hl
	dec hl
	cp typeOperator
	jr nz, +_
	dec hl
	ld (hl), ChainFirst
	dec hl
	ld (hl), typeChain
	jr stopChain
_:	dec hl
	ld (hl), ChainPush
	dec hl
	ld (hl), typeChain
	ld a, 0F5h					; push af
	push hl
		call InsertA
	pop hl
stopChain:
notOperator:
	inc hl
	inc hl
	jp loop
CheckIfSingleOperand:
	dec hl
	dec hl
	dec hl
	dec hl
	res fixed_output, (iy+myFlags)
	ld a, (hl)
	cp typeChain
	ret z
	cp typeVariable
	inc hl
	ld e, (hl)
	jp z, InsertAIXE			; ld a, (ix+*)
	cp typeFunction
	jr nz, +_
	ld a, e
	jp GetFunction
_:	set fixed_output, (iy+myFlags)
	ld a, 03Eh
	call InsertA				; ld a, *
	ld a, e
	jp InsertA					; ld a, XX
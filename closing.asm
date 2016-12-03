	ld hl, openedParensE
	ld b, a
	ld a, (hl)
	or a
	jr z, MismatchedParens
	ld a, b
	cp tComma
	jr z, +_
	dec (hl)
_:	ld hl, (stackPtr)
	ld bc, stack
	or a
	sbc hl, bc
	jr z, StopMovingForArgument2
	add hl, bc
	dec hl
	dec hl
	dec hl
	dec hl
	ld a, (hl)
	cp typeFunction
	jr z, StopMovingForArgument
	cp typeOperator
	jr nz, +_
	inc hl
	ld a, (hl)
	dec hl
	or a
	jr z, StopMovingForArgument
_:	ld (stackPtr), hl
	ld de, (outputPtr)
	ldi
	ldi
	ldi
	ldi
	ld (outputPtr), de
	jr --_
_:	add hl, bc
StopMovingForArgument:
	ld a, (tempToken)
	cp tComma
	jr z, StopMovingForArgument2
	ld (stackPtr), hl
	inc hl
	ld a, (hl)
	cp tLParen
	jr z, StopMovingForArgument2
	dec hl
	ld de, (outputPtr)
	ldi
	ldi
	ldi
	ldi
	ld (outputPtr), de
StopMovingForArgument2:
	call _IncFetch
	jp MainLoop
MismatchedParens:
	ld a, (openedParensF)
	or a
	jp z, MismatchError
	ld a, (tempToken)
	cp tComma
	jr nz, +_
	set triggered_a_comma, (iy+fExpression2)
_:	jp StopParsing
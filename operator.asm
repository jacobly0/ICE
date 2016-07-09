	ld hl, precedence
	add hl, bc
	ld e, (hl)										; e = precedence of current token
NextOperator:
	ld hl, (stackPtr)
	dec hl
	dec hl
	ld a, (hl)
	cp typeOperator
	jr nz, NotAnOperator
	inc hl
	ld a, (hl)
	ld hl, operators_booleans
	ld bc, 14
	cpir
	ld hl, precedence2
	add hl, bc
	ld a, (hl)										; a = precedence of last token on stack
	cp e
	jr c, HigherPrecedence
	push de
		ld hl, (stackPtr)
		dec hl
		dec hl
		ld (stackPtr), hl
		ld de, (outputPtr)
		ldi
		ldi
		ld (outputPtr), de
	pop de
	jr NextOperator
HigherPrecedence:
NotAnOperator:
	ld hl, (stackPtr)
	ld (hl), typeOperator
	inc hl
	push hl
		call _CurFetch
	pop hl
	ld (hl), a
	ld hl, (stackPtr)
	inc hl
	inc hl
	ld (stackPtr), hl
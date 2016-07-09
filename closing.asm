OperatorsLoop
	ld hl, (stackPtr)
	dec hl
	push hl
		call _Get_Tok_Strng
		ld hl, OP3-1
		add hl, bc
		ld a, (hl)
	pop hl
	cp tEnter
	jr z, StopOperators
	cp '('
	jr z, StopOperators
	dec hl
	ld de, (outputPtr)
	ldi
	ldi
	ld (outputPtr), de
	dec hl
	dec hl
	ld (stackPtr), hl
	jr OperatorsLoop
StopOperators:
	cp '('
	ret nz
	ld hl, (stackPtr)
	dec hl
	dec hl
	ld (stackPtr), hl
CFunction0Args:
	bit triggered_a_comma, (iy+fExpression3)
	jp nz, ErrorSyntax
	ld b, 0
CInsertCallPops:
	ld hl, usedCroutines
CFunctionArgsSMC = $+1
	ld de, 0
	ld a, e
	or a
	jr nz, +_
	ld hl, 0E5272Eh
	call InsertHL															; ld l, lcdBpp8 \ push hl
	inc b
_:	add hl, de
	ld e, (hl)
	ld d, 4
	mlt de
	ld hl, CData2-CData+UserMem-4
	add hl, de
	call InsertCallHL														; call *
	ld a, b
	or a
	ret z
	ld a, 0E1h
_:	call InsertA
	djnz -_
	ret																		; pop hl
	
CFunction1Arg:
	bit triggered_a_comma, (iy+fExpression3)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	bit arg1_is_small, (iy+fFunction1)
	call CGetArgumentLast
	ld b, 1
	jr CInsertCallPops
		
CFunction2Args:
	bit triggered_a_comma, (iy+fExpression3)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	ld (CFunction2ArgsSMC2), hl
	ld hl, tempArg1
	bit arg1_is_small, (iy+fFunction1)
	call CGetArgument
	push hl
CFunction2ArgsSMC2 = $+1
		ld hl, 0
		bit arg2_is_small, (iy+fFunction1)
		call CGetArgumentLast
		ld de, (programPtr)
	pop hl
	ld bc, tempArg1
	call CAddArgument
	ld (programPtr), de
	ld b, 2
	jp CInsertCallPops
	
CFunction3Args:
	bit triggered_a_comma, (iy+fExpression3)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	ld (CFunction3ArgsSMC2), hl
	ld hl, tempArg1
	bit arg1_is_small, (iy+fFunction1)
	call CGetArgument
	bit output_is_number, (iy+fExpression1)
	push af
		bit output_is_string, (iy+fExpression1)
		push af
			push hl
				ld hl, tempArg2
				bit arg2_is_small, (iy+fFunction1)
				call CGetArgument
				push hl
CFunction3ArgsSMC2 = $+1
					ld hl, 0
					bit arg3_is_small, (iy+fFunction1)
					call CGetArgumentLast
					ld de, (programPtr)
				pop hl
				ld bc, tempArg2
				call CAddArgument
			pop hl
			ld bc, tempArg1
			or a
			sbc hl, bc
			push hl
			pop bc
			ld hl, tempArg1
			ldi
		pop af
		jr z, +_
		push hl
			ld hl, (programDataOffsetPtr)
			dec hl
			dec hl
			dec hl
			ld (hl), de
		pop hl
_:		ldir
		ld (programPtr), de
		ld b, 3
		call CInsertCallPops
	pop af
	ret
	
CFunction4Args:
	bit triggered_a_comma, (iy+fExpression3)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	ld (CFunction4ArgsSMC2), hl
	ld hl, tempArg1
	bit arg1_is_small, (iy+fFunction1)
	call CGetArgument
	push hl
		ld hl, tempArg2
		bit arg2_is_small, (iy+fFunction1)
		call CGetArgument
		push hl
			ld hl, tempArg3
			bit arg3_is_small, (iy+fFunction1)
			call CGetArgument
			push hl
CFunction4ArgsSMC2 = $+1
				ld hl, 0
				bit arg4_is_small, (iy+fFunction1)
				call CGetArgumentLast
				ld de, (programPtr)
			pop hl
			ld bc, tempArg3
			call CAddArgument
		pop hl
		ld bc, tempArg2
		call CAddArgument
	pop hl
	ld bc, tempArg1
	call CAddArgument
	ld (programPtr), de
	ld b, 4
	jp CInsertCallPops
	
CFunction5Args:
	bit triggered_a_comma, (iy+fExpression3)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	ld (CFunction5ArgsSMC2), hl
	ld hl, tempArg1
	bit arg1_is_small, (iy+fFunction1)
	call CGetArgument
	bit output_is_number, (iy+fExpression1)
	push af
		push hl
			ld hl, tempArg2
			bit arg2_is_small, (iy+fFunction1)
			call CGetArgument
			push hl
				ld hl, tempArg3
				bit arg3_is_small, (iy+fFunction1)
				call CGetArgument
				push hl
					ld hl, tempArg4
					bit arg4_is_small, (iy+fFunction1)
					call CGetArgument
					push hl
CFunction5ArgsSMC2 = $+1
						ld hl, 0
						bit arg5_is_small, (iy+fFunction1)
						call CGetArgumentLast
						ld de, (programPtr)
					pop hl
					ld bc, tempArg4
					call CAddArgument
				pop hl
				ld bc, tempArg3
				call CAddArgument
			pop hl
			ld bc, tempArg2
			call CAddArgument
		pop hl
		ld bc, tempArg1
		call CAddArgument
		ld (programPtr), de
		ld b, 5
		call CInsertCallPops
	pop af
	ret
	
CFunction6Args:
	bit triggered_a_comma, (iy+fExpression3)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	ld (CFunction6ArgsSMC2), hl
	ld hl, tempArg1
	bit arg1_is_small, (iy+fFunction1)
	call CGetArgument
	push hl
		ld hl, tempArg2
		bit arg2_is_small, (iy+fFunction1)
		call CGetArgument
		push hl
			ld hl, tempArg3
			bit arg3_is_small, (iy+fFunction1)
			call CGetArgument
			push hl
				ld hl, tempArg4
				bit arg4_is_small, (iy+fFunction1)
				call CGetArgument
				push hl
					ld hl, tempArg5
					bit arg5_is_small, (iy+fFunction1)
					call CGetArgument
					push hl
CFunction6ArgsSMC2 = $+1
						ld hl, 0
						cp a												; reset zero flag
						call CGetArgumentLast
						ld de, (programPtr)
					pop hl
					ld bc, tempArg5
					call CAddArgument
				pop hl
				ld bc, tempArg4
				call CAddArgument
			pop hl
			ld bc, tempArg3
			call CAddArgument
		pop hl
		ld bc, tempArg2
		call CAddArgument
	pop hl
	ld bc, tempArg1
	call CAddArgument
	ld (programPtr), de
	ld b, 6
	jp CInsertCallPops
	
CTransparentSpriteNoClip:
	ld a, 60
	jr +_	
CSpriteNoClip:
	ld a, 59
_:	ld (iy+fFunction1), %00000100
	ld (CFunctionArgsSMC), a
	call CFunction3Args
	jr z, +_
	ld hl, (programPtr)
	ld de, -11
	add hl, de
	push hl
		ld hl, (hl)
		push hl
		pop de
		add hl, hl
		add hl, de
		ld de, (PrevProgramPtr)
		ld bc, UserMem - program
		add hl, bc
		add hl, de
		ex de, hl
	pop hl
	ld (hl), de
	dec hl
	ld (hl), 02Ah															; ld hl, (XXXXXX)
	ret
_:	ld hl, (programPtr)
	ld de, -8
	add hl, de
	ld (programPtr), hl
	inc hl
	inc hl
	ld hl, (hl)
	push hl
		ld a, 0E5h
		call InsertA														; push hl
		ld a, 0D1h
		ld hl, 0111929h
		call InsertAHL														; pop de \ add hl, hl \ add hl, de \ ld de, ******
		ld hl, (PrevProgramPtr)
		ld de, UserMem - program
		add hl, de
		call InsertHL														; ld de, XXXXXX
		ld a, 019h
		ld hl, 0E527EDh
		call InsertAHL														; add hl, de \ ld hl, (hl) \ push hl
	pop hl
	call InsertCallHL														; call ******
	ld hl, 0E1E1E1h
	jp InsertHL																; pop hl \ pop hl \ pop hl
	
CTransparentScaledSpriteNoClip:
	ld a, 63
	jr +_
	
CScaledSpriteNoClip:
	ld a, 62
_:	ld (CFunctionArgsSMC), a
	ld (iy+fFunction1), %00000111
	call CFunction5Args
	jr z, +_
	ld hl, (programPtr)
	ld de, -13
	add hl, de
	push hl
		ld hl, (hl)
		push hl
		pop de
		add hl, hl
		add hl, de
		ld de, (PrevProgramPtr)
		ld bc, UserMem - program
		add hl, bc
		add hl, de
		ex de, hl
	pop hl
	ld (hl), de
	dec hl
	ld (hl), 02Ah															; ld hl, (XXXXXX)
	ret
_:	ld hl, (programPtr)
	ld de, -10
	add hl, de
	ld (programPtr), hl
	inc hl
	inc hl
	ld hl, (hl)
	push hl
		ld a, 0E5h
		call InsertA														; push hl
		ld a, 0D1h
		ld hl, 0111929h
		call InsertAHL														; pop de \ add hl, hl \ add hl, de \ ld de, ******
		ld hl, (PrevProgramPtr)
		ld de, UserMem - program
		add hl, de
		call InsertHL														; ld de, XXXXXX
		ld a, 019h
		ld hl, 0E527EDh
		call InsertAHL														; add hl, de \ ld hl, (hl) \ push hl
	pop hl
	call InsertCallHL														; call ******
	ld a, 0E1h
	call InsertA															; pop hl
	ld hl, 0E1E1E1h
	jp InsertAHL															; pop hl \ pop hl \ pop hl \ pop hl
CFunction0Args:
	bit triggered_a_comma, (iy+fExpression2)
	jp nz, ErrorSyntax
	ld hl, usedCroutines
CFunction0ArgsSMC = $+1
	ld de, 0
	add hl, de
	ld c, (hl)
	ld b, 4
	mlt bc
	ld hl, CData2-CData+UserMem-4
	add hl, bc
	jp InsertCallHL															; call *
	
CFunction1Arg:
	bit triggered_a_comma, (iy+fExpression2)
	jp z, ErrorSyntax
	call _IncFetch
	call ParseExpression
	bit arg1_is_small, (iy+fFunction1)
	jr z, +_
	bit output_is_number, (iy+fExpression1)
	jr z, +_
	ld hl, (programPtr)
	dec hl
	dec hl
	ld (programPtr), hl
	dec hl
	dec hl
	ld (hl), 02Eh															; ld l, *
_:	call InsertPushHLDE
	ld hl, usedCroutines
CFunction1ArgSMC = $+1
	ld de, 0
	add hl, de
	ld c, (hl)
	ld b, 4
	mlt bc
	ld hl, CData2-CData+UserMem-4
	add hl, bc
	call InsertCallHL														; call *
	ld a, 0E1h
	jp InsertA																; pop hl
		
CFunction2Args:
	bit triggered_a_comma, (iy+fExpression2)
	jp z, ErrorSyntax
	call _IncFetch
	ld hl, (programPtr)
	push hl
		ld hl, tempArg1
		ld (programPtr), hl
		call ParseExpression
		bit triggered_a_comma, (iy+fExpression2)
		jp z, ErrorSyntax
		bit arg1_is_small, (iy+fFunction1)
		jr z, +_
		bit output_is_number, (iy+fExpression1)
		jr z, +_
		ld hl, (programPtr)
		dec hl
		dec hl
		ld (programPtr), hl
		dec hl
		dec hl
		ld (hl), 02Eh														; ld l, *
		inc hl
		ld de, (hl)
		ld (hl), e
_:		call InsertPushHLDE
	pop hl
	ld de, (programPtr)
	ld (programPtr), hl
	push de
		call _IncFetch
		call ParseExpression
		bit triggered_a_comma, (iy+fExpression2)
		jp nz, ErrorSyntax
		bit arg2_is_small, (iy+fFunction1)
		jr z, +_
		bit output_is_number, (iy+fExpression1)
		jr z, +_
		ld hl, (programPtr)
		dec hl
		dec hl
		ld (programPtr), hl
		dec hl
		dec hl
		ld (hl), 02Eh														; ld l, *
		inc hl
		ld de, (hl)
		ld (hl), e
_:		call InsertPushHLDE
	pop hl
	ld de, tempArg1
	or a
	sbc hl, de
	push hl
	pop bc
	ld hl, (programPtr)
	ex de, hl
	ldir
	ld hl, usedCroutines
CFunction2ArgsSMC = $+1
	ld de, 0
	add hl, de
	ld c, (hl)
	ld b, 4
	mlt bc
	ld hl, CData2-CData+UserMem-4
	add hl, bc
	call InsertCallHL														; call *
	ld a, 0E1h
	call InsertA															; pop hl
	jp InsertA																; pop hl
	
CFunction3Args:
	bit triggered_a_comma, (iy+fExpression2)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	ld (CFunction3ArgsSMC2), hl
	ld hl, tempArg1
	ld (programPtr), hl
	call _IncFetch
	call ParseExpression
	bit triggered_a_comma, (iy+fExpression2)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	bit arg1_is_small, (iy+fFunction1)
	jr z, +_
	bit output_is_number, (iy+fExpression1)
	jr z, +_
	dec hl
	dec hl
	ld (programPtr), hl
	dec hl
	dec hl
	ld (hl), 02Eh															; ld l, *
	inc hl
	ld de, (hl)
	ld (hl), e
	inc hl
_:	call InsertPushHLDE
	bit output_is_number, (iy+fExpression1)
	push af
		bit output_is_string, (iy+fExpression1)
		push af
			push hl
				ld hl, tempArg2
				ld (programPtr), hl
				call _IncFetch
				call ParseExpression
				bit triggered_a_comma, (iy+fExpression2)
				jp z, ErrorSyntax
				ld hl, (programPtr)
				bit arg2_is_small, (iy+fFunction1)
				jr z, +_
				bit output_is_number, (iy+fExpression1)
				jr z, +_
				dec hl
				dec hl
				ld (programPtr), hl
				dec hl
				dec hl
				ld (hl), 02Eh												; ld l, *
				inc hl
				ld de, (hl)
				ld (hl), e
				inc hl
_:				call InsertPushHLDE
				push hl
CFunction3ArgsSMC2 = $+1
					ld hl, 0
					ld (programPtr), hl
					call _IncFetch
					call ParseExpression
					bit triggered_a_comma, (iy+fExpression2)
					jp nz, ErrorSyntax
					bit arg3_is_small, (iy+fFunction1)
					jr z, +_
					bit output_is_number, (iy+fExpression1)
					jr z, +_
					ld hl, (programPtr)
					dec hl
					dec hl
					ld (programPtr), hl
					dec hl
					dec hl
					ld (hl), 02Eh											; ld l, *
					inc hl
					ld de, (hl)
					ld (hl), e
_:					call InsertPushHLDE
					ld de, (programPtr)
				pop hl
				ld bc, tempArg2
				or a
				sbc hl, bc
				push hl
				pop bc
				ld hl, tempArg2
				ldir
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
		ld hl, usedCroutines
CFunction3ArgsSMC = $+1
		ld de, 0
		add hl, de
		ld c, (hl)
		ld b, 4
		mlt bc
		ld hl, CData2-CData+UserMem-4
		add hl, bc
		call InsertCallHL													; call *
		ld hl, 0E1E1E1h
	pop af
	jp InsertHL																; pop hl \ pop hl \ pop hl
	
CFunction4Args:
	bit triggered_a_comma, (iy+fExpression2)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	ld (CFunction4ArgsSMC2), hl
	ld hl, tempArg1
	ld (programPtr), hl
	call _IncFetch
	call ParseExpression
	bit triggered_a_comma, (iy+fExpression2)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	bit arg1_is_small, (iy+fFunction1)
	jr z, +_
	bit output_is_number, (iy+fExpression1)
	jr z, +_
	dec hl
	dec hl
	ld (programPtr), hl
	dec hl
	dec hl
	ld (hl), 02Eh															; ld l, *
	inc hl
	ld de, (hl)
	ld (hl), e
	inc hl
_:	call InsertPushHLDE
	push hl
		ld hl, tempArg2
		ld (programPtr), hl
		call _IncFetch
		call ParseExpression
		bit triggered_a_comma, (iy+fExpression2)
		jp z, ErrorSyntax
		ld hl, (programPtr)
		bit arg2_is_small, (iy+fFunction1)
		jr z, +_
		bit output_is_number, (iy+fExpression1)
		jr z, +_
		dec hl
		dec hl
		ld (programPtr), hl
		dec hl
		dec hl
		ld (hl), 02Eh														; ld l, *
		inc hl
		ld de, (hl)
		ld (hl), e
		inc hl
_:		call InsertPushHLDE
		push hl
			ld hl, tempArg3
			ld (programPtr), hl
			call _IncFetch
			call ParseExpression
			bit triggered_a_comma, (iy+fExpression2)
			jp z, ErrorSyntax
			ld hl, (programPtr)
			bit arg3_is_small, (iy+fFunction1)
			jr z, +_
			bit output_is_number, (iy+fExpression1)
			jr z, +_
			dec hl
			dec hl
			ld (programPtr), hl
			dec hl
			dec hl
			ld (hl), 02Eh													; ld l, *
			inc hl
			ld de, (hl)
			ld (hl), e
			inc hl
_:			call InsertPushHLDE
			push hl
CFunction4ArgsSMC2 = $+1
				ld hl, 0
				ld (programPtr), hl
				call _IncFetch
				call ParseExpression
				bit triggered_a_comma, (iy+fExpression2)
				jp nz, ErrorSyntax
				bit arg4_is_small, (iy+fFunction1)
				jr z, +_
				bit output_is_number, (iy+fExpression1)
				jr z, +_
				ld hl, (programPtr)
				dec hl
				dec hl
				ld (programPtr), hl
				dec hl
				dec hl
				ld (hl), 02Eh												; ld l, *
				inc hl
				ld de, (hl)
				ld (hl), e
_:				call InsertPushHLDE
				ld de, (programPtr)
			pop hl
			ld bc, tempArg3
			or a
			sbc hl, bc
			push hl
			pop bc
			ld hl, tempArg3
			ldir
		pop hl
		ld bc, tempArg2
		or a
		sbc hl, bc
		push hl
		pop bc
		ld hl, tempArg2
		ldir
	pop hl
	ld bc, tempArg1
	or a
	sbc hl, bc
	push hl
	pop bc
	ld hl, tempArg1
	ldir
	ld (programPtr), de
	ld hl, usedCroutines
CFunction4ArgsSMC = $+1
	ld de, 0
	add hl, de
	ld c, (hl)
	ld b, 4
	mlt bc
	ld hl, CData2-CData+UserMem-4
	add hl, bc
	call InsertCallHL														; call *
	ld a, 0E1h
	ld hl, 0E1E1E1h
	jp InsertAHL															; pop hl \ pop hl \ pop hl \ pop hl
	
CFunction5Args:
	bit triggered_a_comma, (iy+fExpression2)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	ld (CFunction5ArgsSMC2), hl
	ld hl, tempArg1
	ld (programPtr), hl
	call _IncFetch
	call ParseExpression
	bit triggered_a_comma, (iy+fExpression2)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	bit arg1_is_small, (iy+fFunction1)
	jr z, +_
	bit output_is_number, (iy+fExpression1)
	jr z, +_
	dec hl
	dec hl
	ld (programPtr), hl
	dec hl
	dec hl
	ld (hl), 02Eh															; ld l, *
	inc hl
	ld de, (hl)
	ld (hl), e
	inc hl
_:	call InsertPushHLDE
	bit output_is_number, (iy+fExpression1)
	push af
		push hl
			ld hl, tempArg2
			ld (programPtr), hl
			call _IncFetch
			call ParseExpression
			bit triggered_a_comma, (iy+fExpression2)
			jp z, ErrorSyntax
			ld hl, (programPtr)
			bit arg2_is_small, (iy+fFunction1)
			jr z, +_
			bit output_is_number, (iy+fExpression1)
			jr z, +_
			dec hl
			dec hl
			ld (programPtr), hl
			dec hl
			dec hl
			ld (hl), 02Eh													; ld l, *
			inc hl
			ld de, (hl)
			ld (hl), e
			inc hl
_:			call InsertPushHLDE
			push hl
				ld hl, tempArg3
				ld (programPtr), hl
				call _IncFetch
				call ParseExpression
				bit triggered_a_comma, (iy+fExpression2)
				jp z, ErrorSyntax
				ld hl, (programPtr)
				bit arg3_is_small, (iy+fFunction1)
				jr z, +_
				bit output_is_number, (iy+fExpression1)
				jr z, +_
				dec hl
				dec hl
				ld (programPtr), hl
				dec hl
				dec hl
				ld (hl), 02Eh												; ld l, *
				inc hl
				ld de, (hl)
				ld (hl), e
				inc hl
_:				call InsertPushHLDE
				push hl
					ld hl, tempArg4
					ld (programPtr), hl
					call _IncFetch
					call ParseExpression
					bit triggered_a_comma, (iy+fExpression2)
					jp z, ErrorSyntax
					ld hl, (programPtr)
					bit arg4_is_small, (iy+fFunction1)
					jr z, +_
					bit output_is_number, (iy+fExpression1)
					jr z, +_
					dec hl
					dec hl
					ld (programPtr), hl
					dec hl
					dec hl
					ld (hl), 02Eh											; ld l, *
					inc hl
					ld de, (hl)
					ld (hl), e
					inc hl
_:					call InsertPushHLDE
					push hl
CFunction5ArgsSMC2 = $+1
						ld hl, 0
						ld (programPtr), hl
						call _IncFetch
						call ParseExpression
						bit triggered_a_comma, (iy+fExpression2)
						jp nz, ErrorSyntax
						bit arg5_is_small, (iy+fFunction1)
						jr z, +_
						bit output_is_number, (iy+fExpression1)
						jr z, +_
						ld hl, (programPtr)
						dec hl
						dec hl
						ld (programPtr), hl
						dec hl
						dec hl
						ld (hl), 02Eh										; ld l, *
						inc hl
						ld de, (hl)
						ld (hl), e
_:						call InsertPushHLDE
						ld de, (programPtr)
					pop hl
					ld bc, tempArg4
					or a
					sbc hl, bc
					push hl
					pop bc
					ld hl, tempArg4
					ldir
				pop hl
				ld bc, tempArg3
				or a
				sbc hl, bc
				push hl
				pop bc
				ld hl, tempArg3
				ldir
			pop hl
			ld bc, tempArg2
			or a
			sbc hl, bc
			push hl
			pop bc
			ld hl, tempArg2
			ldir
		pop hl
		ld bc, tempArg1
		or a
		sbc hl, bc
		push hl
		pop bc
		ld hl, tempArg1
		ldir
		ld (programPtr), de
		ld hl, usedCroutines
CFunction5ArgsSMC = $+1
		ld de, 0
		add hl, de
		ld c, (hl)
		ld b, 4
		mlt bc
		ld hl, CData2-CData+UserMem-4
		add hl, bc
		call InsertCallHL													; call *
		ld a, 0E1h
		call InsertA														; pop hl
		ld hl, 0E1E1E1h
	pop af
	ld a, 0E1h
	jp InsertAHL															; pop hl \ pop hl \ pop hl \ pop hl
	
CFunction6Args:
	bit triggered_a_comma, (iy+fExpression2)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	ld (CFunction6ArgsSMC2), hl
	ld hl, tempArg1
	ld (programPtr), hl
	call _IncFetch
	call ParseExpression
	bit triggered_a_comma, (iy+fExpression2)
	jp z, ErrorSyntax
	ld hl, (programPtr)
	bit arg1_is_small, (iy+fFunction1)
	jr z, +_
	bit output_is_number, (iy+fExpression1)
	jr z, +_
	dec hl
	dec hl
	ld (programPtr), hl
	dec hl
	dec hl
	ld (hl), 02Eh															; ld l, *
	inc hl
	ld de, (hl)
	ld (hl), e
	inc hl
_:	call InsertPushHLDE
	push hl
		ld hl, tempArg2
		ld (programPtr), hl
		call _IncFetch
		call ParseExpression
		bit triggered_a_comma, (iy+fExpression2)
		jp z, ErrorSyntax
		ld hl, (programPtr)
		bit arg2_is_small, (iy+fFunction1)
		jr z, +_
		bit output_is_number, (iy+fExpression1)
		jr z, +_
		dec hl
		dec hl
		ld (programPtr), hl
		dec hl
		dec hl
		ld (hl), 02Eh														; ld l, *
		inc hl
		ld de, (hl)
		ld (hl), e
		inc hl
_:		call InsertPushHLDE
		push hl
			ld hl, tempArg3
			ld (programPtr), hl
			call _IncFetch
			call ParseExpression
			bit triggered_a_comma, (iy+fExpression2)
			jp z, ErrorSyntax
			ld hl, (programPtr)
			bit arg3_is_small, (iy+fFunction1)
			jr z, +_
			bit output_is_number, (iy+fExpression1)
			jr z, +_
			dec hl
			dec hl
			ld (programPtr), hl
			dec hl
			dec hl
			ld (hl), 02Eh													; ld l, *
			inc hl
			ld de, (hl)
			ld (hl), e
			inc hl
_:			call InsertPushHLDE
			push hl
				ld hl, tempArg4
				ld (programPtr), hl
				call _IncFetch
				call ParseExpression
				bit triggered_a_comma, (iy+fExpression2)
				jp z, ErrorSyntax
				ld hl, (programPtr)
				bit arg4_is_small, (iy+fFunction1)
				jr z, +_
				bit output_is_number, (iy+fExpression1)
				jr z, +_
				dec hl
				dec hl
				ld (programPtr), hl
				dec hl
				dec hl
				ld (hl), 02Eh												; ld l, *
				inc hl
				ld de, (hl)
				ld (hl), e
				inc hl
_:				call InsertPushHLDE
				push hl
					ld hl, tempArg5
					ld (programPtr), hl
					call _IncFetch
					call ParseExpression
					bit triggered_a_comma, (iy+fExpression2)
					jp z, ErrorSyntax
					ld hl, (programPtr)
					bit arg5_is_small, (iy+fFunction1)
					jr z, +_
					bit output_is_number, (iy+fExpression1)
					jr z, +_
					dec hl
					dec hl
					ld (programPtr), hl
					dec hl
					dec hl
					ld (hl), 02Eh											; ld l, *
					inc hl
					ld de, (hl)
					ld (hl), e
					inc hl
_:					call InsertPushHLDE
					push hl
CFunction6ArgsSMC2 = $+1
						ld hl, 0
						ld (programPtr), hl
						call _IncFetch
						call ParseExpression
						bit triggered_a_comma, (iy+fExpression2)
						jp nz, ErrorSyntax
						call InsertPushHLDE
						ld de, (programPtr)
					pop hl
					ld bc, tempArg5
					or a
					sbc hl, bc
					push hl
					pop bc
					ld hl, tempArg5
					ldir
				pop hl
				ld bc, tempArg4
				or a
				sbc hl, bc
				push hl
				pop bc
				ld hl, tempArg4
				ldir
			pop hl
			ld bc, tempArg3
			or a
			sbc hl, bc
			push hl
			pop bc
			ld hl, tempArg3
			ldir
		pop hl
		ld bc, tempArg2
		or a
		sbc hl, bc
		push hl
		pop bc
		ld hl, tempArg2
		ldir
	pop hl
	ld bc, tempArg1
	or a
	sbc hl, bc
	push hl
	pop bc
	ld hl, tempArg1
	ldir
	ld (programPtr), de
	ld hl, usedCroutines
CFunction6ArgsSMC = $+1
	ld de, 0
	add hl, de
	ld c, (hl)
	ld b, 4
	mlt bc
	ld hl, CData2-CData+UserMem-4
	add hl, bc
	call InsertCallHL														; call *
	ld hl, 0E1E1E1h
	push hl
	pop de
	jp InsertDEHL															; pop hl (x6)
	
CTransparentSpriteNoClip:
	ld a, 60
	jr +_	
CSpriteNoClip:
	ld a, 59
_:	ld (iy+fFunction1), %00000100
	scf
	sbc hl, hl
	ld (hl), 2
	ld (CFunction3ArgsSMC), a
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
		ld de, spriteStack
		add hl, de
		ld bc, (hl)
	pop de
	ld hl, (programDataOffsetPtr)
	ld (hl), de
	inc hl
	inc hl
	inc hl
	ld (programDataOffsetPtr), hl
	ex de, hl
	ld (hl), bc
	ret
_:	ld hl, (programPtr)
	ld de, -8
	add hl, de
	ld (programPtr), hl
	inc hl
	inc hl
	ld hl, (hl)
	push hl
		ld a, 011h
		call InsertA															; ld de, *
		call InsertProgramPtrToDataOffset
		ld hl, programDataData
		call InsertHL															; ld de, RANDOM
		ld hl, 027ED19h
		call InsertHL															; add hl, de \ ld hl, (hl)
		ld a, 0E5h
		call InsertA															; push hl
	pop hl
	call InsertCallHL															; call ******
	ld hl, 0E1E1E1h
	jp InsertHL																	; pop hl \ pop hl \ pop hl
	
CScaledSpriteNoClip:
	ld a, 62
_:	ld (CFunction5ArgsSMC), a
	ld (iy+fFunction1), %00000111
	call CFunction5Args
	jp z, ErrorSyntax
	ld hl, (programPtr)
	ld de, -13
	add hl, de
	push hl
		ld hl, (hl)
		push hl
		pop de
		add hl, hl
		add hl, de
		ld de, spriteStack
		add hl, de
		ld bc, (hl)
	pop de
	ld hl, (programDataOffsetPtr)
	ld (hl), de
	inc hl
	inc hl
	inc hl
	ld (programDataOffsetPtr), hl
	ex de, hl
	ld (hl), bc
	ret

CTransparentScaledSpriteNoClip:
	ld a, 63
	jr -_
	
CBegin:
	bit triggered_a_comma, (iy+fExpression2)
	jp nz, ErrorSyntax
	ld hl, 0E5272Eh
	call InsertHL															; ld l, lcdBpp8 \ push hl
	ld hl, usedCroutines
	ld c, (hl)
	ld b, 4
	mlt bc
	ld hl, CData2-CData+UserMem-4
	add hl, bc
	call InsertCallHL														; call *
	ld a, 0E1h
	jp InsertA																; pop hl
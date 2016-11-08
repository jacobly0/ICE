functionFor:
	ld hl, amountOfEnds
	inc (hl)
	call _IncFetch
	sub tA
	jp c, ErrorSyntax
	cp ttheta+1-tA
	jp nc, ErrorSyntax
	; loop variable
	ld b, a
	add a, a
	add a, b
	ld (ForVariable1), a
	ld (ForVariable2), a
	ld l, a
	push hl
		call _IncFetch
_:		jp c, ErrorSyntax
		cp tComma
		jp nz, ErrorSyntax
		call _IncFetch
		; loop start point
		set multiple_arguments, (iy+myFlags)
		set for_step_is_number, (iy+myFlags2)
		res end_point_is_number, (iy+myFlags2)
		call ParseExpression
		bit triggered_a_comma, (iy+myFlags)
		res triggered_a_comma, (iy+myFlags)
		jp z, ErrorSyntax
ForVariable1 = $+3
		ld hl, 0002FDDh
		call InsertHL											; ld (ix+*), hl
		call _IncFetch
		jr c, -_
		; loop end point
		call ParseExpression
		bit output_is_number, (iy+myFlags)
		jr nz, ForEndPointIsNumber
ForEndPointIsExpression:
		ld a, 022h
		call InsertA											; ld (******), hl
		ld hl, (programPtr)
		ld (ForEndPointExpression), hl
		call InsertHL											; ld (RANDOM), hl
		jr ForGetStep
ForEndPointIsNumber:
		set end_point_is_number, (iy+myFlags2)
		ld hl, (programPtr)
		dec hl
		dec hl
		dec hl
		ld de, (hl)
		dec hl
		ld (programPtr), hl
		ld (ForFixedEndPoint), de
ForGetStep:
		bit triggered_a_comma, (iy+myFlags)
		res triggered_a_comma, (iy+myFlags)
		jr nz, +_
		; loop step
		ld hl, 1
		push hl
			jr ForStart
_:		call _IncFetch
		jr c, -_
		cp tChs
		jr nz, +_
		set negative_for_step, (iy+myFlags2)
		call _IncFetch
		jp c, ErrorSyntax
_:		cp tA
		jr c, ForGetStepNumber
		cp ttheta+1
		jp nc, ForGetStepNumber
ForGetStepVariable:
		res for_step_is_number, (iy+myFlags2)
		scf
		sbc hl, hl
		ld (hl), 2
		call _CurFetch
		sub tA
		ld c, a
		call InsertHIXC												; ld hl, (ix+*)
		call _NxtFetch
		jp c, ErrorSyntax
		cp tEnter
		jp nz, ErrorSyntax
		ld a, 022h
		call InsertA												; ld (*), hl
		ld hl, (programPtr)
		push hl
			call InsertHL											; ld (RANDOM), hl
			jr ForStart
ForGetStepNumber:
		call ParseExpression
		bit triggered_a_comma, (iy+myFlags)
		jr nz, -_
		bit output_is_number, (iy+myFlags)
		jp z, ErrorSyntax
		ld hl, (programPtr)
		dec hl
		dec hl
		dec hl
		dec hl
		ld (programPtr), hl
		inc hl
		ld hl, (hl)
		push hl
ForStart:
			ld hl, (programPtr)
			push hl
ForVariable2 = $+3
				ld hl, 00027DDh
				call InsertHL										; ld hl, (ix+*)
				bit end_point_is_number, (iy+myFlags2)
				jr nz, ForInsertEndPointNumber
ForInsertEndPointExpression:
				ld a, 0B7h
				bit negative_for_step, (iy+myFlags2)
				jr nz, +_
				bit end_point_is_number, (iy+myFlags2)
				jr nz, +_
				ld a, 037h
_:				ld (ForSetCarryFlag), a
				ld a, 011h
				call InsertA										; ld de, *
				ld hl, (programPtr)
				ld de, UserMem-program
				add hl, de
				ex de, hl
ForEndPointExpression = $+1
				ld hl, 0
				ld (hl), de
				call InsertHL										; ld de, RANDOM
				jr ForSkip
ForInsertEndPointNumber:
				ld a, 011h
ForFixedEndPoint = $+1
				ld hl, 0
				bit negative_for_step, (iy+myFlags2)
				jr nz, +_
				inc hl
_:				call InsertAHL										; ld de, *
ForSkip:
ForSetCarryFlag = $+1
				ld hl, 052EDB7h
				call InsertHL										; or a \ sbc hl, de
				ld a, 0D2h
				bit negative_for_step, (iy+myFlags2)
				jr z, +_
				add a, 8
_:				call InsertA										; jp [n]c, *
				ld hl, (programPtr)
				push hl
					call InsertHL									; jp [n]c, RANDOM
					ld a, (iy+myFlags2)
					push af
						call ParseProgramUntilEnd
						ld b, a
					pop af
					ld (iy+myFlags2), a
					ld a, b
					cp tElse
					jp z, ErrorSyntax
					ld ix, 0
					add ix, sp
					ld a, (ix+9)
					ld hl, 00027DDh
					call _SetHLUToA
					call InsertHL									; ld hl, (ix+*)
				pop bc
			pop de
		pop hl
		push de
			push bc
				bit for_step_is_number, (iy+myFlags2)
				jr z, InsertVariableChange
InsertNumberChange:
				ex de, hl
				bit negative_for_step, (iy+myFlags2)
				jr z, $+8
				call SubChainAnsNumber
				jr $+6
				call AddChainAnsNumber
				jr InsertStop
InsertVariableChange:
				ex de, hl
				ld a, 011h
				call InsertA										; ld de, *
				ld hl, (programPtr)
				ld bc, UserMem-program
				add hl, bc
				ex de, hl
				ld (hl), de
				call InsertHL										; ld de, RANDOM
				ld a, 019h
				ld hl, 052EDB7h
				bit negative_for_step, (iy+myFlags2)
				jr z, $+8
				call InsertHL										; or a \ sbc hl, de
				jr $+6
				call InsertA										; add hl, de
InsertStop:
			pop bc
		pop de
	pop hl
	ld a, l
	ld hl, 0002FDDh
	call _SetHLUToA
	call InsertHL													; ld (ix+*), hl
	ld a, 0C3h
	bit negative_for_step, (iy+myFlags2)
	jr z, $+4
	add a, 15
	ld hl, UserMem-program
	add hl, de
	call InsertAHL													; jp [nc] *
	ld de, UserMem-program
	add hl, de
	ex de, hl
	push bc
	pop hl
	ld (hl), de
	ret
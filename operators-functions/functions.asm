ExecuteFunction:
	push hl
		ld hl, FunctionsWithReturnValueArguments
		ld bc, FunctionsWithReturnValueEnd - FunctionsWithReturnValueArguments
		cpir
		ld b, 3
		mlt bc
		ld hl, FunctionsWithReturnValueStart
		add hl, bc
		ld hl, (hl)
		ld (JumpFunction), hl
	pop hl
JumpFunction = $+1
	jp 0
	
functionSkipLine:
	ld hl, (endPC)
	ld de, (curPC)
	or a
	sbc hl, de
	inc hl
	push hl
	pop bc
	ex de, hl
	ld a, tEnter
	cpir
	dec hl
	ld (curPC), hl
	ret
	
functionLbl:
	ld ix, (labelPtr)
	ld hl, (programPtr)
	ld (ix), hl
	ld hl, (curPC)
	inc hl
	ld (ix+3), hl
	lea hl, ix+6
	ld (labelPtr), hl
_:	call _IncFetch
	ret c
	cp tEnter
	ret z
	jr -_
	
functionGoto:
	ld a, 0C3h
functionCall2
	call InsertA															; jp ******
	ld ix, (gotoPtr)
	ld hl, (programPtr)
	ld (ix), hl
	ld hl, (curPC)
	inc hl
	ld (ix+3), hl
	lea hl, ix+6
	ld (gotoPtr), hl
	call InsertHL															; jp RANDOM
	jr -_
	
functionCall:
	ld a, 0CDh
	jr functionCall2
	
functionReturn:
	call _NxtFetch
	jr nc, +_
	set last_token_is_ret, (iy+myFlags2)
_:	ld a, 0C9h
	jp InsertA																; ret
	
functionDisp:
	call _IncFetch
	set multiple_arguments, (iy+myFlags)
	call ParseExpression
	ld de, (programPtr)
	ld bc, 14
	bit output_is_string, (iy+myFlags2)
	jr nz, DispString
	ld hl, DispNumberRoutine
	jr +_
DispString:
	ld hl, DispStringRoutine
	dec bc
_:	ldir
	ld (programPtr), de
	bit triggered_a_comma, (iy+myFlags)
	ret z
	jr functionDisp
		
functionRepeat:
	ld hl, amountOfEnds
	inc (hl)
	ld hl, (programPtr)
	ld de, UserMem - program
	add hl, de
	push hl
		ld hl, (curPC)
		inc hl
		push hl
_:			call _IncFetch
			jp c, ErrorSyntax
			cp tEnter
			jr nz, -_
			call ParseProgramUntilEnd
			cp tElse
			jp z, ErrorSyntax
			ld de, (curPC)
		pop hl
		ld (curPC), hl
		push de
			call ParseExpression
		pop de
		ld (curPC), de
		bit ans_set_z_flag, (iy+myFlags)
		jr z, +_
		ld hl, (programPtr)
		dec hl
		dec hl
		dec hl
		ld a, (hl)
		ld ixh, a
		dec hl
		dec hl
		dec hl
		dec hl
		ld (programPtr), hl
		jr ++_
_:		ld a, 019h
		ld hl, 052EDB7h
		call InsertAHL														; add hl, de \ or a \ sbc hl, de
_:		ld bc, UserMem - program
		add hl, bc
	pop de
	or a
	sbc hl, de
	ld a, h
	or a
	jr nz, functionRepeatLarge
	ld a, l
	cpl
	dec a
	cp %10000000
	jr c, functionRepeatLarge
functionRepeatSmall:
	ld b, a
	ld a, 028h
	bit ans_set_z_flag, (iy+myFlags)
	jr z, +_
	ld a, ixh
_:	call InsertA															; jr [n]z, *
	ld a, b
	jp InsertA																; jr [n]z, *
functionRepeatLarge:
	ex de, hl
	bit ans_set_z_flag, (iy+myFlags)
	jr z, +_
	ld a, ixh
	add a, 0A2h
	jr ++_
_:	ld a, 0CAh
_:	jp InsertAHL															; jp [n]z, XXXXXX

functionIf:
	ld hl, amountOfEnds
	inc (hl)
	call _IncFetch
	call ParseExpression
	bit ans_set_z_flag, (iy+myFlags)
	jr z, +_
	ld hl, (programPtr)
	dec hl
	dec hl
	dec hl
	ld a, (hl)
	add a, 0A2h
	dec hl
	dec hl
	dec hl
	dec hl
	ld (programPtr), hl
	jr ++_
_:	ld a, 019h
	ld hl, 052EDB7h
	call InsertAHL															; add hl, de \ or a \ sbc hl, de
	ld a, 0CAh
_:	call InsertA															; jp z, ******
	ld hl, (programPtr)
	push hl
		call InsertHL														; jp z, XXXXXX
		call ParseProgramUntilEnd
		cp tElse
		jr nz, +_
		ld hl, amountOfEnds
		inc (hl)
		ld a, 0C3h
		call InsertA														; jp ******
		ld de, (programPtr)
		call InsertHL														; jp XXXXXX
		ld bc, UserMem - program
		add hl, bc
		push hl
		pop bc
	pop hl
	ld (hl), bc
	push de
		call ParseProgramUntilEnd
		cp tElse
		jp z, ErrorSyntax
_:		ld hl, (programPtr)
		ld de, UserMem - program
		add hl, de
		ex de, hl
	pop hl
	ld (hl), de
	ret
	
functionWhile:
	ld hl, amountOfEnds
	inc (hl)
	call _IncFetch
	jp c, ErrorSyntax
	cp tEnter
	jp z, ErrorSyntax
	ld hl, (programPtr)
	push hl
		call ParseExpression
		bit ans_set_z_flag, (iy+myFlags)
		jr z, +_
		ld hl, (programPtr)
		dec hl
		dec hl
		dec hl
		ld a, (hl)
		add a, 0A2h
		dec hl
		dec hl
		dec hl
		dec hl
		ld (programPtr), hl
		jr ++_
_:		ld a, 019h
		ld hl, 052EDB7h
		call InsertAHL														; add hl, de \ or a \ sbc hl, de
		ld a, 0CAh
_:		call InsertA														; jp z, ******
		ld hl, (programPtr)
		push hl
			call InsertHL													; jp z, ******
			call ParseProgramUntilEnd
			cp tElse
			jp z, ErrorSyntax
			ld a, 0C3h
		pop de
	pop hl
	ld bc, UserMem - program
	add hl, bc
	call InsertAHL															; jp XXXXXX
	add hl, bc
	ex de, hl
	ld (hl), de
	ret
	ex de, hl
	jp InsertHL																; jp z, XXXXXX
	
functionClrHome:
	call _NxtFetch
	jr c, +_
	cp tEnter
	jp nz, ErrorSyntax
_:	ld a, 0CDh
	ld hl, _HomeUp
	call InsertAHL															; call _HomeUp
	ld hl, _ClrLCDFull
	jp InsertAHL															; call _ClrLCDFull
	
functionPause:
	call _IncFetch
	jr c, +_
	cp tEnter
	jr z, +_
	call ParseExpression
	ld a, 0CDh
	call InsertA															; call *
	call InsertProgramPtrToDataOffset
	ld hl, (programDataDataPtr)
	push hl
	pop de
	call InsertHL															; call *
	ld hl, PauseRoutine
	ld bc, PauseRoutineEnd - PauseRoutine
	ldir
	ld (programDataDataPtr), de
	ret
_:	ld a, 0CDh
	ld hl, _GetCSC
	call InsertAHL															; call _GetCSC
	ld a, 0FEh
	ld hl, 0F82009h
	jp InsertAHL															; cp tEnter \ jr nz, $-9
	
functionInput:
	call _IncFetch
	cp tA
	jp c, ErrorSyntax
	cp ttheta+1
	jp nc, ErrorSyntax
	call _NxtFetch
	jr c, +_
	cp tEnter
	jp nz, ErrorSyntax
_:	ld a, 03Eh
	call InsertA															; ld a, **
	call _CurFetch
	sub tA
	ld b, a
	add a, a
	add a, b
	call InsertA															; ld a, XX
	ld a, 032h
	call InsertA															; ld (******), a
	call InsertProgramPtrToDataOffset
	bit has_already_input, (iy+myFlags2)
	jr nz, AddPointerToInput
	ld hl, (programDataDataPtr)
	ld (InputStartData), hl
	ld bc, InputOffset-InputRoutine
	add hl, bc
	call InsertHL															; ld (XXXXXX), a
	ld a, 0CDh
	call InsertA															; call ******
	call InsertProgramPtrToDataOffset
	ld hl, (programDataDataPtr)
	push hl
	pop de
	call InsertHL															; call XXXXXX
	ld hl, InputRoutine
	ld bc, InputRoutineEnd-InputRoutine
	ldir
	ld (programDataDataPtr), de
	set has_already_input, (iy+myFlags2)
	ret
AddPointerToInput:
	ld hl, (InputStartData)
	push hl
		ld bc, InputOffset-InputRoutine
		add hl, bc
		call InsertHL														; ld (XXXXXX), a
		ld a, 0CDh
		call InsertA														; call ******
		call InsertProgramPtrToDataOffset
	pop hl
	jp InsertHL																; call XXXXXX
	
functionNot:
	ld a, 1
	ld (amountOfArguments), a
	push hl
	pop ix
	ld a, (ix-4)
	or a
	jr z, NotNumber
	dec a
	jr z, NotVariable
	dec a
	jr z, NotChainPush
	dec a
	jr z, NotChainAns
	dec a
	jr z, NotFunction
	jp ErrorSyntax
NotNumber:
	set output_is_number, (iy+myFlags)
	ld hl, (ix-3)
	add hl, de
	or a
	sbc hl, de
	ld hl, 0
	jr nz, +_
	inc hl
_:	ld (ix-3), hl
	ret
NotVariable:
	ld c, (ix-3)
	ld b, 3
	mlt bc
	ld a, c
	ld hl, 00027DDh
	call _SetHLUToA
	call InsertHL															; ld hl, (ix+*)
	jr NotChainAns
NotChainPush:
	jp UnknownError
NotChainAns:
	ld a, 011h
	call InsertA															; ld de, *
	ld a, 0FFh
	ld de, 019FFFFh
	ld hl, 02362EDh
	jp InsertADEHL															; ld de, -1 \ add hl, de \ sbc hl, hl \ inc hl
NotFunction:
	ld a, (ix-3)
	ld b, OutputInHL
	call GetFunction
	jr NotChainAns
	
functionCE:
	inc hl
	inc hl
	ld a, (hl)
	cp tRemainder
	jp nz, ErrorSyntax
	ld de, -10
	add hl, de
		
functionRemainder:
	ld a, 2
	ld (amountOfArguments), a
	ld a, (hl)
	ld ixh, a
	inc hl
	push hl
		ld bc, (hl)
		inc hl
		inc hl
		inc hl
		ld a, (hl)
		ld ixl, a
		inc hl
		ld de, (hl)
	pop hl
	ld a, ixh
	or a
	jr z, RemainderNumberXXX
	dec a
	jp z, RemainderVariableXXX
	dec a
	jp z, RemainderChainPushXXX
	dec a
	jp z, RemainderChainAnsXXX
	dec a
	jp z, RemainderFunctionXXX
	jp ErrorSyntax
RemainderNumberXXX:
	ld a, ixl
	or a
	jr z, RemainderNumberNumber
	dec a
	jr z, RemainderNumberVariable
	dec a
	jr z, RemainderNumberChainPush
	dec a
	jr z, RemainderNumberChainAns
	dec a
	jr z, RemainderNumberFunction
	jp ErrorSyntax
RemainderNumberNumber:
	set output_is_number, (iy+myFlags)
	push hl
		push bc
		pop hl
		push de
		pop bc
		call __idvrmu
		ex de, hl
	pop hl
	ld (hl), de
	ret
RemainderNumberVariable:
	ld a, 021h
	push bc
	pop hl
	call InsertAHL															; ld hl, *
	jp RemainderChainAnsVariable
RemainderNumberChainPush:
	jp UnknownError
RemainderNumberChainAns:
	ld de, 021C1E5h
	push bc
	pop hl
	call InsertDEHL															; push hl \ pop bc \ ld hl, *
	ld a, 0CDh
	ld hl, __idvrmu
	jp InsertAHL															; call __idvrmu
RemainderNumberFunction:
	ld a, e
	push bc
		ld b, OutputInBC
		call GetFunction
	pop hl
	ld a, 021h
	call InsertAHL															; ld hl, *
	ld a, 0CDh
	ld hl, __idvrmu
	jp InsertAHL															; call __idvrmu
RemainderVariableXXX:
	ld a, ixl
	or a
	jr z, RemainderVariableNumber
	dec a
	jr z, RemainderVariableVariable
	dec a
	jr z, RemainderVariableChainPush
	dec a
	jr z, RemainderVariableChainAns
	dec a
	jr z, RemainderVariableFunction
	jp ErrorSyntax
RemainderVariableNumber:
	call InsertHIXC															; ld hl, (ix+*)
	jr RemainderChainAnsNumber
RemainderVariableVariable
	call InsertHIXC															; ld hl, (ix+*)
	jr RemainderChainAnsVariable
RemainderVariableChainPush:
	jp UnknownError
RemainderVariableChainAns:
	ld a, 0E5h
	call InsertA															; push hl
	ld a, 0C1h
	call InsertA															; pop bc
_:	call InsertHIXC															; ld hl, (ix+*)
	ld a, 0CDh
	ld hl, __idvrmu
	jp InsertAHL															; call __idvrmu
RemainderVariableFunction:
	ld a, e
	ld b, OutputInBC
	call GetFunction
	jr -_
RemainderChainPushXXX:
	ld a, ixl
	cp typeChainAns
	jp nz, UnknownError
	ld hl, 0E1C1E5h
	call InsertAHL															; push hl \ pop bc \ pop hl
	jr +_
RemainderChainAnsXXX:
	ld a, ixl
	or a
	jr z, RemainderChainAnsNumber
	dec a
	jr z, RemainderChainAnsVariable
	dec a
	jr z, RemainderChainAnsChainPush
	dec a
	jr z, RemainderChainAnsChainAns
	dec a
	jr z, RemainderChainAnsFunction
	jp ErrorSyntax
RemainderChainAnsNumber:
	ex de, hl
	ld a, 001h
	call InsertAHL															; ld bc, *
_:	ld a, 0CDh
	ld hl, __idvrmu
	jp InsertAHL															; call __idvrmu
RemainderChainAnsVariable:
	call InsertIXC															; ld bc, (ix+*)
	jr -_
RemainderChainAnsChainPush:
	jp UnknownError
RemainderChainAnsChainAns:
	jp UnknownError
RemainderChainAnsFunction:
	ld a, e
	ld b, OutputInBC
	set need_push, (iy+myFlags)
	call GetFunction
	jr -_
RemainderFunctionXXX:
	ld a, ixl
	or a
	jr z, RemainderFunctionNumber
	dec a
	jr z, RemainderFunctionVariable
	dec a
	jr z, RemainderFunctionChainPush
	dec a
	jr z, RemainderFunctionChainAns
	dec a
	jr z, RemainderFunctionFunction
	jp ErrorSyntax
RemainderFunctionNumber:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jr RemainderChainAnsNumber
RemainderFunctionVariable:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jr RemainderChainAnsVariable
RemainderFunctionChainPush:
	jp UnknownError
RemainderFunctionChainAns:
	ld a, 0E5h
	call InsertA														; push hl
	ld a, 0C1h
	call InsertA														; pop bc
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jr -_
RemainderFunctionFunction:
	ld a, e
	ld b, OutputInBC
	call GetFunction
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jr -_
	
functionMax:
	ld a, 030h
	jr +_
functionMin:
	ld a, 038h
_:	ld (MaxMinInsertSMC), a
	ld a, 2
	ld (amountOfArguments), a
	dec hl
	dec hl
	dec hl
	ld de, (hl)
	dec hl
	ld a, (hl)
	ld ixl, a
	dec hl
	dec hl
	dec hl
	ld bc, (hl)
	dec hl
	ld a, (hl)
	inc hl
	or a
	jr z, MaxMinNumberXXX
	dec a
	jp z, MaxMinVariableXXX
	dec a
	jp z, MaxMinChainPushXXX
	dec a
	jp z, MaxMinChainAnsXXX
	dec a
	jp z, MaxMinFunctionXXX
	jp ErrorSyntax
MaxMinNumberXXX:
	ld a, ixl
	or a
	jr z, MaxMinNumberNumber
	dec a
	jr z, MaxMinNumberVariable
	dec a
	jr z, MaxMinNumberChainPush
	dec a
	jr z, MaxMinNumberChainAns
	dec a
	jr z, MaxMinNumberFunction
	jp ErrorSyntax
MaxMinNumberNumber:
	set output_is_number, (iy+myFlags)
	push hl
		push bc
		pop hl
		or a
		sbc hl, de
		jr c, +_
		add hl, de
		ex de, hl
_:	pop hl
	ld (hl), de
	ret
MaxMinNumberVariable:
	push bc
	pop hl
	ex de, hl
	push hl
	pop bc
	jr MaxMinVariableNumber
MaxMinNumberChainPush:
	jp UnknownError
MaxMinNumberChainAns:
	push bc
	pop de
	jr MaxMinChainAnsNumber
MaxMinNumberFunction:
	ld a, e
	push bc
		ld b, OutputInHL
		call GetFunction
	pop de
	jr MaxMinChainAnsNumber
MaxMinVariableXXX:
	ld a, ixl
	or a
	jr z, MaxMinVariableNumber
	dec a
	jr z, MaxMinVariableVariable
	dec a
	jr z, MaxMinVariableChainPush
	dec a
	jr z, MaxMinVariableChainAns
	dec a
	jr z, MaxMinVariableFunction
	jp ErrorSyntax
MaxMinVariableNumber:
	call InsertHIXC															; ld hl, (ix+*)
	jr MaxMinChainAnsNumber
MaxMinVariableVariable
	call InsertHIXC															; ld hl, (ix+*)
	jr MaxMinChainAnsVariable
MaxMinVariableChainPush:
	jp UnknownError
MaxMinVariableChainAns:
_:	ld e, c
	call InsertIXE															; ld de, (ix+*)
	jr MaxMinInsert
MaxMinVariableFunction:
	ld a, e
	ld b, OutputInHL
	call GetFunction
	jr -_
MaxMinChainPushXXX:
	ld a, ixl
	cp typeChainAns
	jp nz, UnknownError
	ld a, 0EDh
	call InsertA															; ex de, hl
	ld a, 0E1h
	call InsertA															; pop hl
	jr MaxMinInsert
MaxMinChainAnsXXX:
	ld a, ixl
	or a
	jr z, MaxMinChainAnsNumber
	dec a
	jr z, MaxMinChainAnsVariable
	dec a
	jr z, MaxMinChainAnsChainPush
	dec a
	jr z, MaxMinChainAnsChainAns
	dec a
	jr z, MaxMinChainAnsFunction
	jp ErrorSyntax
MaxMinChainAnsNumber:
	ex de, hl
	ld a, 011h
	call InsertAHL															; ld de, *
MaxMinInsert:
	ld a, 0B7h
	ld de, 01952EDh
MaxMinInsertSMC = $+1
	ld hl, 0EB0130h
	jp InsertADEHL															; or a \ sbc hl, de \ add hl, de \ jr [n]c, $+3 \ add hl, de
MaxMinChainAnsVariable:
	call InsertIXE															; ld de, (ix+*)
	jr MaxMinInsert
MaxMinChainAnsChainPush:
	jp UnknownError
MaxMinChainAnsChainAns:
	jp UnknownError
MaxMinChainAnsFunction:
	ld a, e
	ld b, OutputInDE
	set need_push, (iy+myFlags)
	call GetFunction
	jr MaxMinInsert
MaxMinFunctionXXX:
	ld a, ixl
	or a
	jr z, MaxMinFunctionNumber
	dec a
	jr z, MaxMinFunctionVariable
	dec a
	jr z, MaxMinFunctionChainPush
	dec a
	jr z, MaxMinFunctionChainAns
	dec a
	jr z, MaxMinFunctionFunction
	jp ErrorSyntax
MaxMinFunctionNumber:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jr MaxMinChainAnsNumber
MaxMinFunctionVariable:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jr MaxMinChainAnsVariable
MaxMinFunctionChainPush:
	jp UnknownError
MaxMinFunctionChainAns:
	ld a, 0EBh
	call InsertA														; ex de, hl
	ld a, c
	ld b, OutputInHL
	set need_push, (iy+myFlags)
	call GetFunction
	jr MaxMinInsert
MaxMinFunctionFunction:
	ld a, e
	ld b, OutputInDE
	call GetFunction
	ld a, c
	ld b, OutputInHL
	set need_push, (iy+myFlags)
	call GetFunction
	jr MaxMinInsert
	
functionC:
	ld hl, openedParens
	inc (hl)
	call _IncFetch
	set multiple_arguments, (iy+myFlags)
	call ParseExpression
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
	ld de, 75
	or a
	sbc hl, de
	jp nc, ErrorSyntax
	add hl, de
	ld a, l
	or a
	sbc hl, hl
	ld l, a
	ld de, CFunctionsArguments
	add hl, de
	ld b, a
	ld a, (hl)
	cp %11100000
	jp z, ImplementError
	ld (iy+myFlags4), a
	rlca
	rlca
	rlca
	and 000000111b
	ld d, a
	ld a, b
	ld hl, CSpecialFunctions
	ld bc, CSpecialFunctionsEnd - CSpecialFunctions
	cpir
	jr nz, +_
	ld b, 3
	mlt bc
	ld hl, CSpecialFunctionsStart
	add hl, bc
	ld hl, (hl)
	jp (hl)
_:	ld c, d
	ld b, 3
	mlt bc
	ld ix, CArguments
	add ix, bc
	ld hl, (ix)
	ld c, 7*3
	add ix, bc
	ld de, (ix)
	ld (de), a
	jp (hl)

functionCustom:
	call _IncFetch
	sub 10
	jp c, ErrorSyntax
	cp AMOUNT_OF_CUSTOM_TOKENS + 1
	jp nc, ErrorSyntax
	ld c, a
	ld b, 3
	mlt bc
	ld hl, functionCustomStart
	add hl, bc
	ld hl, (hl)
	jp (hl)
	
functionExecHex:
	call _IncFetch
	ret c
	cp tEnter
	ret z
	cp tString
	ret z
	ld hl, hexadecimals
	ld bc, 16
	cpir
_:	jp nz, ErrorSyntax
	ld d, c
	call _IncFetch
	jp c, ErrorSyntax
	ld hl, hexadecimals
	ld bc, 16
	cpir
	jr nz, -_
	ld a, d
	add a, a
	add a, a
	add a, a
	add a, a
	add a, c
	call InsertA
	jr functionExecHex
	
functionDefineSprite:
	call _IncFetch
	set multiple_arguments, (iy+myFlags)
	call ParseExpression
	bit output_is_number, (iy+myFlags)
	jp z, ErrorSyntax
	bit triggered_a_comma, (iy+myFlags)
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
	pop bc
	add hl, hl
	add hl, bc
	ld de, (programDataDataPtr)
	ld bc, spriteStack
	add hl, bc
	ld (hl), de
	ld b, 2
_:	push de
		push bc
			call _IncFetch
			call ParseExpression
			bit output_is_number, (iy+myFlags)
			jp z, ErrorSyntax
			bit triggered_a_comma, (iy+myFlags)
			jp z, ErrorSyntax
			ld hl, (programPtr)
			dec hl
			dec hl
			dec hl
			dec hl
			ld (programPtr), hl
			inc hl
			ld a, (hl)
		pop bc
	pop de
	ld (de), a
	inc de
	djnz -_
	ld hl, (programPtr)
	push hl
		ld (programPtr), de
		call _IncFetch
		cp tString
		jp nz, ErrorSyntax
		call _NxtFetch
		jp c, ErrorSyntax
		cp tEnter
		jp z, ErrorSyntax
		call functionExecHex
		ld hl, (programPtr)
		ld (programDataDataPtr), hl
	pop hl
	ld (programPtr), hl
	ret
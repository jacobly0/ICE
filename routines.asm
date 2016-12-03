InsertBCDEAHL:
	push hl
		push bc
		pop hl
		call InsertHL
		ex de, hl
		call InsertHL
	pop hl
InsertAHL:
	call InsertA
InsertHL:
	push de
		ex de, hl
		ld hl, (programPtr)
		ld (hl), de
		inc hl
		inc hl
		inc hl
		ld (programPtr), hl
	pop de
	ret
	
InsertCallHL:
	ld a, 0CDh
	jr InsertAHL
	
InsertHIXC:
	ld h, 027h
	ld b, 3
	mlt bc
	ld a, c
	jr ++_
InsertIXE:
	ld h, 017h
_:	ld d, 3
	mlt de
	ld a, e
_:	ld l, 0DDh
	call _SetHLUToA
	jr InsertHL																; ld hl/de/bc, (ix+*)
InsertIXC:
	ld h, 007h
	jr --_

InsertBCDEHL:
	push hl
		push bc
		pop hl
		call InsertHL
	pop hl
	jr InsertDEHL
InsertADEHL:
	call InsertA
InsertDEHL:
	ex de, hl
	call InsertHL
	ex de, hl
	jr InsertHL

InsertE:
	ld a, e
InsertA:
	push hl
		ld hl, (programPtr)
		ld (hl), a
		inc hl
		ld (programPtr), hl
	pop hl
	ret
	
MaybeChangeHLToDE:
	ld a, (ExprOutput)
	or a
	ret z
_:	ld a, 0EBh
	jp InsertA																; ex de, hl
	
MaybeChangeDEToHL:
	ld a, (ExprOutput)
	or a
	ret nz
	jr -_
	
InsertProgramPtrToDataOffset:
	push de
		ld hl, (programDataOffsetPtr)
		ld de, (programPtr)
		ld (hl), de
		inc hl
		inc hl
		inc hl
		ld (programDataOffsetPtr), hl
	pop de
	ret
	
GetFunction:
	push bc
		push de
			call GetRightFunction
		pop de
	pop bc
	ret
	
GetRightFunction:
	dec b
	jp z, GetFunctionDE
	dec b
	jr z, GetFunctionHL
	jp GetFunctionBC
GetFunctionHL:
	cp tGetKey
	jr nz, +_
	ld hl, _GetCSC
	call InsertCallHL														; call _GetCSC
	ld a, 0B7h
	ld hl, 06F62EDh
	jp InsertAHL															; or a \ sbc hl, hl \ ld l, a
_:	cp trand
	jr nz, +_
	ld a, 0D5h
	bit need_push, (iy+fExpression1)
	call nz, InsertA														; push de
	call InsertRandRoutine
	bit need_push, (iy+fExpression1)
	ret z
	ld a, 0D1h
	jp InsertA																; pop de
_:	call InsertKeypadRoutine1
	jp InsertKeypadRoutine2
GetFunctionDE:
	cp tGetKey
	jr nz, +_
	ld a, 0E5h
	bit need_push, (iy+fExpression1)
	call nz, InsertA														; push hl
		ld hl, _GetCSC
	call InsertCallHL														; call _GetCSC
	bit need_push, (iy+fExpression1)
	jr z, $+8
	ld a, 0E1h
	call InsertA															; pop hl
	ld a, 011h
	call InsertA															; ld de, *
	xor a
	ld hl, 05F0000h
	jp InsertAHL															; ld de, 0 \ ld e, a
_:	cp trand
	jr nz, ++_
	ld a, 0E5h
	bit need_push, (iy+fExpression1)
	call nz, InsertA														; push hl
	call InsertRandRoutine
_:	ld a, 0EBh
	call InsertA															; ex de, hl
	bit need_push, (iy+fExpression1)
	ret z
	ld a, 0E1h
	jp InsertA																; pop hl
_:	call InsertKeypadRoutine1
	ld a, 0E5h
	bit need_push, (iy+fExpression1)
	call nz, InsertA														; push hl
	call InsertKeypadRoutine2
	jr --_
GetFunctionBC:
	cp tGetKey
	jr nz, +_
	ld hl, _GetCSC
	call InsertCallHL														; call _GetCSC
	ld a, 001h
	call InsertA															; ld bc, *
	xor a
	ld hl, 05F0000h
	jp InsertAHL															; ld bc, 0 \ ld e, a
_:	cp trand
	jr nz, ++_
	call InsertRandRoutine
_:	ld a, 0E5h
	call InsertA															; push hl
	ld a, 0C1h
	jp InsertA																; pop bc
_:	call InsertKeypadRoutine1
	call InsertKeypadRoutine2
	jr --_
InsertKeypadRoutine1:
	ld b, a
	ld a, 006h
	call InsertA															; ld b, *
	ld a, b
	dec a
	rrca
	rrca
	and %0001110
	ld c, a
	ld a, 01Eh
	sub c
	call InsertA															; ld b, X
	ld a, 00Eh
	call InsertA															; ld c, *
	ld a, b
	and %00000111
	ld b, a
	xor a
	scf
_:	rla
	djnz -_
	jp InsertA																; ld c, X
	
InsertKeypadRoutine2:
	ld a, 0CDh
	call InsertA															; call *
	call InsertProgramPtrToDataOffset
	ld hl, (KeypadStartData)
	bit has_already_keypad, (iy+fProgram1)
	jp nz, InsertHL															; call XXXXXX
	ld hl, (programDataDataPtr)
	ld (KeypadStartData), hl
	push hl
	pop de
	bit output_is_number, (iy+fExpression1)
	call InsertHL															; call *
	ld hl, KeypadRoutine
	ld bc, KeypadRoutineEnd - KeypadRoutine
	ldir
	ld (programDataDataPtr), de
	set has_already_keypad, (iy+fProgram1)
	ret
	
InsertRandRoutine:
	ld a, 0CDh
	call InsertA															; call *
	call InsertProgramPtrToDataOffset
	ld hl, (RandStartData)
	bit has_already_rand, (iy+fProgram1)
	jp nz, InsertHL															; call XXXXXX
	ld hl, (programDataDataPtr)
	ld (RandStartData), hl
	push hl
	pop de
	call InsertHL															; call XXXXXX
	ld hl, RandRoutine
	ld bc, RandRoutineEnd - RandRoutine
	ldir
	ld (programDataDataPtr), de
	set has_already_rand, (iy+fProgram1)
	ret
	
CompareStrings:
	ld a, (de)
	cp a, (hl)
	inc hl
	inc de
	ret nz
	cp tEnter
	jr nz, CompareStrings
	ret
	
SubError:
	ld a, '-'
	jr DispOperatorErrorString
AddError:
	ld a, '+'
	jr DispOperatorErrorString
DivError:
	ld a, '/'
	jr DispOperatorErrorString
MulError:
	ld a, '*'
	jr DispOperatorErrorString
	
GLETError:
	ld a, (tempToken2)
	ld c, a
	cp tGE
	ld a, 019h
	jr z, DispOperatorErrorString
	ld a, c
	cp tLE
	ld a, 017h
	jr z, DispOperatorErrorString
	ld a, c
	cp tGT
	ld a, '>'
	jr z, DispOperatorErrorString
	ld a, '<'
	jr DispOperatorErrorString

NEQError:
	ld a, (tempToken2)
	cp tEq
	ld a, '='
	jr z, DispOperatorErrorString
	ld a, 018h
	jr DispOperatorErrorString
XORANDError:
	ld a, (tempToken2)
	ld c, a
	cp tOr
	ld a, '|'
	jr z, DispOperatorErrorString
	ld a, c
	cp tXor
	ld a, '^'
	jr z, DispOperatorErrorString
	ld a, '&'
	jr DispOperatorErrorString
StoError:
	ld a, 01Ch
	jr DispOperatorErrorString
	
DispOperatorErrorString:
	push af
		call ClearScreen
		ld hl, ErrorMessageStandard
		call PrintString
	pop af
	call _PrintChar_ASM
	ld a, '\''
	call _PrintChar_ASM
	jr ReturnToOS
	
ErrorSyntax:
	ld hl, SyntaxErrorMessage
	jr DispFinalString
MismatchError:
	ld hl, MismatchErrorMessage
	jr DispFinalString
NoProgramsError:
	ld hl, NoProgramsMessage
	set good_compilation, (iy+fProgram1)
	jr DispFinalString
InvalidTokenError:
	ld hl, InvalidTokenMessage
	jr DispFinalString
InvalidNameLength:
	ld hl, InvalidNameLengthMessage
	jr DispFinalString
ErrorNotFound:
	ld hl, NotFoundMessage
	jr DispFinalString
SameNameError:
	ld hl, SameNameMessage
	jr DispFinalString
FunctionError:
	ld hl, FunctionFunctionMessage
	jr DispFinalString
UnknownError:
	ld hl, UnknownMessage
	jr DispFinalString
WrongSpriteDataError:
	ld hl, WrongSpriteDataMessgae
	jr DispFinalString
ImplementError:
	ld hl, ImplementMessage
	jr DispFinalString
EndError:
	ld hl, EndErrorMessage
	jr DispFinalString
ErrorTooLargeLoop:
	ld hl, TooLargeLoopMessage
	jr DispFinalString
LabelError:
	ld hl, LabelErrorMessage
	
DispFinalString:
	push hl
		call ClearScreen
	pop hl
	call PrintString
ReturnToOS:
	bit good_compilation, (iy+fProgram1)
	jr nz, SkipDisplayLineNumber
	ld a, 21
	ld (TextYPos_ASM), a
	ld hl, 1
	ld (TextXPos_ASM), hl
	ld hl, LineNumber
	call PrintString
	ld hl, (curPC)
	ld de, (begPC)
	or a
	sbc hl, de
	push hl
	pop bc
	sbc hl, hl
	inc hl
	ex de, hl
GetAmountOfLines:
	ld a, b
	or c
	jr z, +_
	ld a, tEnter
	cpir
	jr nz, +_
	inc de
	jr GetAmountOfLines
_:	ex de, hl
	xor a
	ld de, OP3+10
	ld (de), a
	dec de
	ld b, 8
	ld a, 10
_:	call _DivHLByA
	add a, t0
	ld (de), a
	dec de
	add hl, de
	or a
	sbc hl, de
	jr z, +_
	djnz -_
_:	ex de, hl
	inc hl
	call PrintString
SkipDisplayLineNumber:
	ld a, 230
	ld (TextYPos_ASM), a
	ld hl, 85
	ld (TextXPos_ASM), hl
	ld hl, PressKey
	call PrintString
_:	call _GetCSC
	or a
	jr z, -_
StopProgram:
backupSP = $+1
	ld sp, 0
backupBegPC = $+1
	ld hl, 0
	ld (begPC), hl
backupCurPC = $+1
	ld hl, 0
	ld (curPC), hl
backupEndPC = $+1
	ld hl, 0
	ld (endPC), hl
	ld a, lcdBpp16
	ld (mpLcdCtrl), a
	call _ClrLCDFull
	call _HomeUp
	jp _DrawStatusBar
	
PrintCompilingProgram:
	call ClearScreen
	ld hl, StartParseMessage
	call PrintString
	ld hl, OP1+1
	jp PrintString
	
ClearScreen:
	ld hl, vRAM+(320*12)
	ld (hl), 255
	push hl
	pop de
	inc de
	ld bc, 320*228-1
	ldir
	ld hl, 1
	ld (TextXPos_ASM), hl
	ld a, 12
	ld (TextYPos_ASM), a
	xor a
	ld (color), a
	ret
	
PrintString:
	ld a, (hl)
	inc hl
	or a
	call nz, _PrintChar_ASM
	jr nz, PrintString
	ret
	
PreScanPrograms:
	ld hl, (endPC)
	ld de, (begPC)
	or a
	sbc hl, de
	inc hl
	push hl
	pop bc
	ld hl, (curPC)
ScanLoop:
	ld a, tEnter
	cpir
	ret nz
	ld a, (hl)
	cp tDet
	jr z, ScanFoundDet
	cp tPause
	jr z, ScanFoundPause
	cp tInput
	jr z, ScanFoundInput
	jr ScanLoop
ScanFoundInput:
	ld a, (amountOfInput)
	inc a
	ld (amountOfInput), a
	jr ScanLoop
ScanFoundPause:
	ld a, (amountOfPause)
	inc a
	ld (amountOfPause), a
	jr ScanLoop
ScanFoundDet:
	inc hl
	ld a, (hl)
	sub t0
	jr c, ScanLoop
	cp t9-t0+1
	jr nc, ScanLoop
	ld de, 0
	ld e, a
	inc hl
	dec bc
	ld a, b
	or c
	jr z, FoundRightCFunction
	ld a, (hl)
	cp tComma
	jr z, FoundRightCFunction
	cp tEnter
	jr z, FoundRightCFunction
	sub t0
	jr c, FoundRightCFunction
	cp t9-t0+1
	jr nc, FoundRightCFunction
	push hl
		ex de, hl
		add hl, hl
		push hl
		pop de
		add hl, hl
		add hl, hl
		add hl, de
		ld de, 0
		ld e, a
		add hl, de
		ex de, hl
	pop hl
FoundRightCFunction:
	push hl
		ex de, hl
		ld de, AMOUNT_OF_C_FUNCTIONS
		or a
		sbc hl, de
		jr nc,  WrongCFunction
		add hl, de
		ex de, hl
		ld hl, usedCroutines
		add hl, de
		ld a, (hl)
		or a
		jr nz, FunctionAlreadyInProgram
AddCFunctionToProgram:
		ld a, (amountOfCRoutines)
		inc a
		ld (hl), a
		ld (amountOfCRoutines), a
		ld a, 0C3h
		push de
		pop hl
		add hl, hl
		add hl, de
		call InsertAHL																; jp *
FunctionAlreadyInProgram:
WrongCFunction:
	pop hl
	jp ScanLoop

GetSpriteData:
	or a
	sbc hl, de
	ret z
	add hl, de
	ld a, (de)
	sub t0
	jp c, WrongSpriteDataError
	cp tA-t0
	jr c, +_
	sub tA-t0
	cp tG-tA
	jp nc, WrongSpriteDataError
	add a, 10
_:	add a, a
	add a, a
	add a, a
	add a, a
	ld ixh, a
	inc de
	or a
	sbc hl, de
	jp z, ErrorSyntax
	add hl, de
	ld a, (de)
	sub t0
	jp c, WrongSpriteDataError
	cp tA-t0
	jr c, +_
	sub tA-t0
	cp tG-tA
	jp nc, WrongSpriteDataError
	add a, 10
_:	or a, ixh
	ld (bc), a
	inc bc
	inc de
	jr GetSpriteData
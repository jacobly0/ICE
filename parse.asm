ParseExpression:
	ld hl, stack
	ld (stackPtr), hl
	ld hl, output
	ld (outputPtr), hl
	ld hl, openedParens
	ld (hl), 0
	res output_is_string, (iy+myFlags2)
	res triggered_a_comma, (iy+myFlags)
	call _CurFetch
	ld hl, FunctionsSingle
	ld bc, FunctionsSingleEnd - FunctionsSingle
	cpir
	jr nz, MainLoop
	ld b, 3
	mlt bc
	ld hl, FunctionsSingleStart
	add hl, bc
	ld hl, (hl)
	jp (hl)
MainLoop:
	ld (tempToken), a
	jp c, StopParsing
	cp t0
	jr c, NotANumber
	cp t9+1
	jr nc, NotANumber
ANumber:
#include "number.asm"

NotANumber:
	res prev_is_number, (iy+myFlags)
	cp tA
	jr c, NotAVariable
	cp ttheta+1
	jr nc, NotAVariable
AVariable:
	ld hl, (outputPtr)
	ld (hl), typeVariable
	inc hl
	sub a, tA
	jr InsertAndUpdatePointer
NotAVariable:
	ld hl, operators_booleans
	ld bc, 15
	cpir
	jr nz, NotABoolean
ABoolean:
#include "operator.asm"

ReturnToLoop:
	call _IncFetch
	jp MainLoop
NotABoolean:
	cp tComma
	jr z, CloseArgument
	cp tRParen
	jp nz, NotACommaOrRParen
CloseArgument:
#include "closing.asm"

NotACommaOrRParen:
	cp tLBrace
	jp nz, NotAList
AList:
#include "list.asm"

NotAList:
	cp tVarLst
	jr nz, NotAnOSList
AnOSList:
	call _IncFetch
	cp 6
	jp nc, InvalidTokenError
	ld c, a
	ld b, 3
	mlt bc
	ld hl, lists
	add hl, bc
	ld hl, (hl)
	ex de, hl
	ld hl, (outputPtr)
	ld (hl), typeOSList
	inc hl
	ld (hl), de
	inc hl
	inc hl
	inc hl
	ld (outputPtr), hl
	call _IncFetch
	cp tLParen
	jr nz, +_
	ld hl, openedParens
	inc (hl)
	ld hl, (stackPtr)
	ld (hl), typeOperator
	inc hl
	ld (hl), 0
	inc hl
	inc hl
	inc hl
	ld (stackPtr), hl
	call _IncFetch
_:	jp MainLoop
NotAnOSList:
	cp tString
	jr nz, NotAString
AString:
	ld hl, (outputPtr)
	ld (hl), typeString
	inc hl
	ld de, (tempStringsPtr)
	ld (hl), de
	inc hl
	inc hl
	inc hl
	ld (outputPtr), hl
StringLoop:
	call _IncFetch
	jr c, StringStop2
	cp tEnter
	jr z, StringStop2
	cp tString
	jr z, StringStop
	cp tStore
	jr z, StringStop
	call _IsA2ByteTok
	jr nz, +_
	inc hl
	ld (curPC), hl
	dec hl
_:	push de
		call _Get_Tok_Strng
	pop de
	ld hl, OP3
	ldir
	jr StringLoop
StringStop:
	cp tEnter
	call nz, _IncFetch
StringStop2:
	ex de, hl
	ld (hl), 0
	inc hl
	ld (tempStringsPtr), hl
	jp MainLoop
	
NotAString:
	cp tEnter
	jr z, StopParsing
	ld hl, FunctionsWithReturnValue
	ld bc, FunctionsWithReturnValueEnd - FunctionsWithReturnValue
	cpir
	jp nz, InvalidTokenError
	cp tGetKey
	jr z, AddFunctionToOutput
	cp trand
	jr nz, AddFunctionToStack
AddFunctionToOutput:
	call _IsA2ByteTok
	call z, _IncFetch
	ld b, a
	ld hl, (outputPtr)
	ld a, (tempToken)
	ld (hl), typeReturnValue
	inc hl
	ld (hl), a
	inc hl
	inc hl
	inc hl
	ld (outputPtr), hl
	jp ReturnToLoop
AddFunctionToStack:
	ld hl, openedParens
	inc (hl)
	call _IsA2ByteTok
	call z, _IncFetch
	ld b, a
	ld hl, (stackPtr)
	ld a, (tempToken)
	ld (hl), typeFunction
	inc hl
	ld (hl), a
	inc hl
	ld (hl), b
	inc hl
	inc hl
	ld (stackPtr), hl
	jp ReturnToLoop
StopParsing:																; move stack to output
	call MoveStackEntryToOutput
	res output_is_number, (iy+myFlags)
	ld hl, (outputPtr)
	ld de, output
	or a
	sbc hl, de
	push hl
	pop bc																	; BC / 4 is amount of elements in the stack
	push de
	pop hl
	ld a, b
	or a, c
	cp 4
	ret c
	jp z, ParseSingleArgument
Loop:
	res output_is_number, (iy+myFlags)
	res ans_set_z_flag, (iy+myFlags)
	or a
	sbc hl, bc
	ld de, output
	sbc hl, de
	jp z, ErrorSyntax
	add hl, de
	add hl, bc
	ld a, b
	or a, c
	cp 4
	ret z
	ld a, (hl)
	cp typeOperator
	jr z, ExpressOperator
	cp typeFunction
	jr z, ExpressFunction
	inc hl
	inc hl
	inc hl
	inc hl
	jr Loop
ExpressFunction:
	inc hl																	; function = a
	ld a, (hl)
	dec hl
	push bc
		push hl
			call ExecuteFunction
		pop de
		push de
		pop ix
		lea hl, ix+4
		ld a, (amountOfArguments)
		dec a
		jr z, ++_
		ld b, a
_:		dec de
		dec de
		dec de
		dec de
		djnz -_
_:	pop bc
	push de
		push bc
			push hl
				ldir
			pop hl
		pop bc
	pop de
	ex de, hl
	add hl, bc
	or a
	sbc hl, de
	push hl
	pop bc																	; BC = BC+DE-HL
	ld a, b
	or c
	cp 4
	ret z
	ex de, hl
	ld a, (amountOfArguments)
	ld b, a
_:	dec hl
	dec hl
	dec hl
	dec hl
	djnz -_
	jr AddChain
ExpressOperator:
	inc hl
	ld a, (hl)
	dec hl
	push bc
		push hl
		pop ix
		ld de, (ix-3)
		ld bc, (ix-7)
		call ExecuteOperator
		lea de, ix-4
	pop bc
	ld hl, 8
	add hl, de
	push de
		push bc
			ldir
		pop bc
		ld hl, -12
		add hl, bc
		add hl, de
		or a
		sbc hl, de
		push hl
		pop bc
	pop hl
	jr nz, +_
	bit output_is_number, (iy+myFlags)
	ret z
	dec hl
	dec hl
	dec hl
	jr ParseSingleArgument2
_:	inc bc
	inc bc
	inc bc
	inc bc
	bit output_is_number, (iy+myFlags)
	jp nz, Loop
AddChain:
	ld e, typeChainAns
	ld a, (hl)
	inc hl
	inc hl
	inc hl
	inc hl
	or a, (hl)
	dec hl
	dec hl
	dec hl
	dec hl
	cp typeOperator
	jr z, ChainAns2
ChainPush2:
	ld a, 0E5h
	call InsertA															; push hl
	ld e, typeChainPush
ChainAns2:
	push hl
		dec hl
		dec hl
		dec hl
		dec hl
		ld (hl), e
	pop hl
	jp Loop
	
ParseSingleArgument:
	ld a, (hl)
	or a
	jr nz, +_
	set output_is_number, (iy+myFlags)
	inc hl
ParseSingleArgument2:
	ld hl, (hl)
	ld a, 021h
	jp InsertAHL														; ld hl, *
_:	dec a
	jr nz, +_
	inc hl
	ld c, (hl)
	jp InsertHIXC
_:	dec a
	dec a
	jr nz, +_
	inc hl
	ld a, (hl)
	ld b, OutputInHL
	res need_push, (iy+myFlags)
	jp GetFunction
_:	sub a, 4
	jp nz, ErrorSyntax
	set output_is_string, (iy+myFlags2)
	push hl
		ld a, 021h
		call InsertA														; ld hl, *
		call InsertProgramPtrToDataOffset
		ld hl, (programDataDataPtr)
		call InsertHL														; ld hl, XXXXXXX
	pop hl
	inc hl
	ld de, (hl)																; hl points to string in string stack
	ld hl, (hl)
	ld bc, -1
	xor a
	cpir
	sbc hl, de
	push hl
	pop bc																	; bc = length of string
	ex de, hl
	ld de, (programDataDataPtr)
	push de
		ldir
		ld (programDataDataPtr), de
	pop hl
	ret
	
MoveStackEntryToOutput:
	ld hl, (stackPtr)
	ld de, stack
	or a
	sbc hl, de
	ret z
	add hl, de
	dec hl
	dec hl
	dec hl
	dec hl
	ld (stackPtr), hl
	ld de, (outputPtr)
	ld a, (hl)
	cp typeFunction
	jr nz, +_
	inc hl
	ld a, (hl)
	dec hl
	cp tLParen
	jr z, MoveStackEntryToOutput
_:	ldi
	ldi
	ldi
	ldi
	ld (outputPtr), de
	jr MoveStackEntryToOutput
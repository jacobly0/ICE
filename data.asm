operators_booleans:
	.db tStore, tAnd, tXor, tOr, tEQ, tLT, tGT, tLE, tGE, tNE, tMul, tDiv, tAdd, tSub, 0			; 0 = temp +
	;		7	6		6	6		4	5	5	5		5	4	3		2	1	0
operators_special:
	.db 0, 1, 2, 3, 4, 5, 5, 5, 5, 4, 6, 6, 6, 7
	
FunctionsWithReturnValue:
	.db tGetKey, trand, tLParen
FunctionsWithReturnValueArguments:
	.db tMean, tMin, tMax, tNot, tExtTok
FunctionsWithReturnValueEnd:

FunctionsWithReturnValueStart:
	.dl functionCE, functionNot, functionMax, functionMin, functionMean

FunctionsSingle:
	.db tInput, tClLCD, tPause, tIf, tWhile, tRepeat, tDisp, tFor, tReturn, tVarOut, tLbl, tGoto, tii, tDet
FunctionsSingleEnd:

FunctionsSingleStart:
	.dl functionC, functionSkipLine, functionGoto, functionLbl, functionCustom, functionReturn, functionFor, functionDisp
	.dl functionRepeat, functionWhile, functionIf, functionPause, functionClrHome, functionInput
	
operator_start:
	.dl SubNumberXXX, SubVariableXXX, SubChainPushXXX, SubChainAnsXXX, SubFunctionXXX, SubError
	.dl AddNumberXXX, AddVariableXXX, AddChainPushXXX, AddChainAnsXXX, AddFunctionXXX, AddError
	.dl DivNumberXXX, DivVariableXXX, DivChainPushXXX, DivChainAnsXXX, DivFunctionXXX, DivError
	.dl MulNumberXXX, MulVariableXXX, MulChainPushXXX, MulChainAnsXXX, MulFunctionXXX, MulError
	.dl NEQNumberXXX, NEQVariableXXX, NEQChainPushXXX, NEQChainAnsXXX, NEQFunctionXXX, NEQError
	.dl GLETNumberXXX, GLETVariableXXX, GLETChainPushXXX, GLETChainAnsXXX, GLETFunctionXXX, GLETError
	.dl XORANDNumberXXX, XORANDVariableXXX, XORANDChainPushXXX, XORANDChainAnsXXX, XORANDFunctionXXX, XORANDError
	.dl StoNumberXXX, StoVariableXXX, StoChainPushXXX, StoChainAnsXXX, StoFunctionXXX, StoListXXX
	
CSpecialFunctions:
	.db 63, 62, 60, 59, 0
CSpecialFunctionsEnd:

CSpecialFunctionsStart:
	.dl CBegin, CSpriteNoClip, CTransparentSpriteNoClip, CScaledSpriteNoClip, CTransparentScaledSpriteNoClip
	
CArguments:
	.dl CFunction0Args, CFunction1Arg, CFunction2Args, CFunction3Args, CFunction4Args, CFunction5Args, CFunction6Args
	.dl CFunction0ArgsSMC, CFunction1ArgSMC, CFunction2ArgsSMC, CFunction3ArgsSMC, CFunction4ArgsSMC, CFunction5ArgsSMC, CFunction5ArgsSMC
	
functionCustomStart:
	.dl functionExecHex, functionDefineSprite, functionCall, functionCompilePrgm
	
precedence:	 .db 7, 4,4,5,5,3,3,3,3,3,3,2, 2,  1,  0
	         ;   t+ - + / * ≠ ≥ ≤ > < = or xor and →
precedence2: .db 0, 4,4,5,5,3,3,3,3,3,3,2, 2,  1,  6
	
offsets:
	.dl stack, output, program, programNamesStack, labelStack, gotoStack, programDataOffsetStack, tempStringsStack, tempListsStack, programDataData
	.fill 8, 0
offset_end:

lists:
	.dl L1, L2, L3, L4, L5, L6
	
hexadecimals:
	.db tF, tE, tD, tC, tB, tA, t9, t8, t7, t6, t5, t4, t3, t2, t1, t0
	
stackPtr:				.dl stack
outputPtr:				.dl output
programPtr:				.dl program
programNamesPtr:		.dl programNamesStack
labelPtr:				.dl labelStack
gotoPtr:				.dl gotoStack
programDataOffsetPtr:	.dl programDataOffsetStack
tempStringsPtr:			.dl tempStringsStack
tempListsPtr:			.dl tempListsStack
programDataDataPtr:		.dl programDataData
amountOfPrograms		.db 0
openedParensE			.db 0
openedParensF			.db 0
amountOfArguments		.db 0
amountOfCRoutines		.db 0
amountOfEnds			.db 0
amountOfInput			.db 0
amountOfPause			.db 0
ExprOutput				.db 0
ExprOutput2				.db 0
AmountOfSubPrograms		.db 0

varname:
	.db ProtProgObj, 0,0,0,0,0,0,0,0
	
usedCroutines:
.fill AMOUNT_OF_C_FUNCTIONS, 0

ICEAppvar:
	.db AppVarObj, "ICEAPPV", 0
ICEProgram:
	.db ProtProgObj, "ICE", 0
	
ErrorMessageStandard:
	.db "Invalid arguments for '", 0
EndErrorMessage:
	.db "Too many Ends!", 0
GoodCompileMessage:
	.db "Succesfully compiled!", 0
NoProgramsMessage:
	.db "No programs found!", 0
InvalidTokenMessage:
	.db "Unsupported token!", 0
InvalidListArgumentMessage:
	.db "Only integers in lists supported!", 0
InvalidNameLengthMessage:
	.db "Program name too long!", 0
SameNameMessage:
	.db "Output name is the same as input name!", 0
WrongSpriteDataMessgae:
	.db "Invalid sprite data!", 0
FunctionFunctionMessage:
	.db "You can't use a function in a function!", 0
NotFoundMessage:
	.db "Program not found!", 0
ImplementMessage:
	.db "This function has not been implemented yet!", 0
SyntaxErrorMessage:
	.db "Invalid arguments entered!", 0
TooLargeLoopMessage:
	.db "Too large anonymous loop!", 0
LineNumber:
	.db "Error on line ", 0
MismatchErrorMessage:
	.db "Mismatched parenthesis!", 0
UnknownMessage:
	.db "Something went wrong! Please report it!", 0
NotEnoughMem:
	.db "Not enough free RAM!", 0
LabelErrorMessage:
	.db "Can't find specific label!", 0
StartParseMessage:
	.db "Compiling program ", 0
ICEName:
	.db "ICE Compiler v1.2 - By Peter \"PT_\" Tillema", 0
PressKey:
	.db "[Press any key to exit]", 0
	
MulTable:
	.db 1 \ add hl, hl \ .db 0,0,0,0,0,0,0,0
	.db 4 \ push hl \ pop de \ add hl, hl \ add hl, de \ .db 0,0,0,0,0
	.db 2 \ add hl, hl \ add hl, hl \ .db 0,0,0,0,0,0,0
	.db 5 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, de \ .db 0,0,0,0
	.db 5 \ add hl, hl \ push hl \ pop de \ add hl, hl \ add hl, de \ .db 0,0,0,0
	.db 6 \ push hl \ pop de \ add hl, hl \ add hl, de \ add hl, hl \ add hl, de \ .db 0,0,0
	.db 3 \ add hl, hl \ add hl, hl \ add hl, hl \ .db 0,0,0,0,0,0
	.db 6 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, hl \ add hl, de \ .db 0,0,0
	.db 6 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, de \ add hl, hl \ .db 0,0,0
	.db 7 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, de \ add hl, hl \ add hl, de \ .db 0,0
	.db 6 \ add hl, hl \ add hl, hl \ push hl \ pop de \ add hl, hl \ add hl, de \ .db 0,0,0
	.db 9 \ push hl \ pop bc \ add hl, hl \ add hl, hl \ push hl \ pop de \ add hl, hl \ add hl, de \ add hl, bc
	.db 7 \ push hl \ pop de \ add hl, hl \ add hl, de \ add hl, hl \ add hl, de \ add hl, hl \ .db 0,0
	.db 9 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, hl \ add hl, hl \ or a \ sbc hl, de
	.db 4 \ add hl, hl \ add hl, hl \ add hl, hl \ add hl, hl \ .db 0,0,0,0,0
	.db 7 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, hl \ add hl, hl \ add hl, de \ .db 0,0
	.db 7 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, hl \ add hl, de \ add hl, hl \ .db 0,0
	.db 8 \ ld bc, 19 \ call __imulu \ .db 0
	.db 7 \ push hl \ pop de \ add hl, hl \ add hl, hl \ add hl, de \ add hl, hl \ add hl, hl \ .db 0
	
InputRoutine:
	call _ClrScrn
	call _HomeUp
	xor	a, a
	ld (ioPrompt), a
	ld	(curUnder), a
	call _GetStringInput
	ld	hl, (editSym)
	call _VarNameToOP1HL
	call _ChkFindSym
	ld a, (de)
	inc de
	inc de
	ld b, a
	sbc hl, hl
_:	push bc
		add hl, hl
		push hl
		pop bc
		add hl, hl
		add hl, hl
		add hl, bc
		ld a, (de)
		sub t0
		ld bc, 0
		ld c, a
		add hl, bc
		inc de
	pop bc
	djnz -_
InputOffset = $+2
	ld (ix+0), hl
	jp _DeleteTempEditEqu
InputRoutineEnd:

RandRoutine:
	ld hl, (ix+rand1)
	ld de, (ix+rand1+3)
	ld b, h
	ld c, l
	add hl, hl
	rl e
	rl d
	add hl, hl
	rl e
	rl d
	inc l
	add hl, bc
	ld (ix+rand1), hl
	adc hl, de
	ld (ix+rand1+3), hl
	ex de, hl
	ld hl, (ix+rand1+6)
	ld bc, (ix+rand1+9)
	add hl, hl
	rl c
	rl b
	ld (ix+rand1+9), bc
	sbc a, a
	and %11000101
	xor l
	ld l, a
	ld (ix+rand1+6), hl
	ex de, hl
	add hl, bc
	ret
RandRoutineEnd:

DispNumberRoutine:
	ld a, 18
	ld (curCol), a
	call _DispHL
	call _NewLine
	
DispStringRoutine:
	xor a
	ld (curCol), a
	call _PutS
	call _NewLine

PauseRoutine:
	dec hl
PauseRoutine2:
	ld c, 110
_:	ld b, 32
	djnz $
	dec c
	jr nz, -_
	or a
	ld de, -1
	add hl, de
	jr c, PauseRoutine2
	ret
PauseRoutineEnd:

MeanRoutine:
	ld ix, 0
	add ix, sp
	add hl, de
	push hl
		rr (ix-1)
	pop hl
	rr h
	rr l
	ld ix, cursorImage
	ret
MeanRoutineEnd:

KeypadRoutine:
	di
	ld hl, 0F50000h
	ld (hl), 2
	xor a
_:	cp a, (hl)
	jr nz, -_
	ei
	ld l, b
	ld a, (hl)
	sbc hl, hl
	and c
	ret z
	inc l
	ret
KeypadRoutineEnd:

XORANDData:
	ld bc, -1
	add hl, bc
	sbc a, a
	ex de, hl
	ld d, a
	add hl, bc
	sbc a, a
XORANDSMC:
	and a, d
	sbc hl, hl
	and 1
	ld l, a
	
BackgroundData:
	ld a, l
	ld hl, vRAM
	ld (hl), a
	push hl
	pop de
	inc de
	ld bc, 320*240-1
	ldir

CData:
	ld ix, cursorImage
	ld hl, 0D1A8DEh						; LibLoadAppVar
	call _Mov9ToOP1
	ld a, AppVarObj
	ld (OP1), a
_:	call _ChkFindSym
	jr c, NotFound
	call _ChkInRAM
	jr nz, InArc
	call _PushOP1
	call _Arc_UnArc
	call _PopOP1
	jr -_
InArc:
	ex de, hl
	ld de, 9
	add hl, de
	ld e, (hl)
	add hl, de
	inc hl
	inc hl
	inc hl
	ld de, 0D1A8FCh						; RelocationStart
	jp (hl)
NotFound:
	call _ClrScrn
	call _HomeUp
	ld hl, 0D1A8DAh						; MissingAppVar
	call _PutS
	call _NewLine
	call _PutS
	jp _GetKey
MissingAppVar:
	.db "Need"
LibLoadAppVar:
	.db " LibLoad", 0
	.db "http://tiny.cc/clibs", 0
RelocationStart:
	.db 0C0h, "GRAPHX", 0, 2
CData2:
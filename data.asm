operators_booleans:
	.db tStore, tAnd, tXor, tOr, tEQ, tLT, tGT, tLE, tGE, tNE, tMul, tDiv, tAdd, tSub
	
precedence:
	.db 4,4,5,5,3,3,3,3,3,3,2, 2,  1,  0
	;   - + / * ≠ ≥ ≤ > < = or xor and →
	
precedence2:
	.db 4,4,5,5,3,3,3,3,3,3,2, 2,  1,  6
	;   - + / * ≠ ≥ ≤ > < = or xor and →
	
operator_start:
	.dl functionSub, functionAdd, functionDiv, functionMul, functionNE, functionGE, functionLE, functionGT, functionLT, functionEQ, functionOr
	.dl functionXor, functionAnd, functionSto
	
functions:
	.db tPause, tDisp, t2ByteTok, tRepeat, tWhile, tIf, tClLCD, tISG, tDSL, tLbl, tGoto, tInput
functions_stop:
	
functions_start:
	.dl functionInput, functionGoto, functionLabel, functionDec, functionInc
	.dl functionClrHome, functionIf, functionWhile, functionRepeat, functionAsm, functionDisp, functionPause
	
hexadecimals:
	.db tF, tE, tD, tC, tB, tA, t9, t8, t7, t6, t5, t4, t3, t2, t1, t0
	
varname:
	.db ProtProgObj, "LOL", 0

varname2:
	.db ProtProgObj, "A",0,0,0,0,0,0,0
	
ICEName:
	.db ProgObj, "ICE", 0

mul_table:
	.db $87, 0,   0,   0,   0,   0	, 0				; 2
	.db $67, $87, $84, 0,   0,   0	, 0				; 3
	.db $87, $87, 0,   0,   0,   0	, 0				; 4
	.db $67, $87, $87, $84, 0,   0	, 0				; 5
	.db $67, $87, $84, $87, 0,   0	, 0				; 6
	.db $67, $87, $84, $87, $84, 0	, 0				; 7
	.db $87, $87, $87, 0,   0,   0	, 0				; 8
	.db $67, $87, $87, $87, $84, 0	, 0				; 9
	.db $67, $87, $87, $84, $87, 0	, 0				; 10
	.db $67, $87, $87, $84, $87, $84, 0				; 11
	.db $67, $87, $84, $87, $87, 0	, 0				; 12
	.db $67, $87, $84, $87, $87, $84, 0				; 13
	.db $67, $87, $84, $87, $84, $87, 0				; 14
	.db $67, $87, $87, $87, $87, $94, 0				; 15
	.db $87, $87, $87, $87, 0,   0	, 0				; 16
	.db $67, $87, $87, $87, $87, $84, 0				; 17
	.db $67, $87, $87, $87, $84, $87, 0				; 18
	.db $6F, $26, 19,  $ED, $6C, $7D, 0				; 19
	.db $67, $87, $87, $84, $87, $87, 0				; 20
	.db $6F, $26, 21,  $ED, $6C, $7D, 0				; 21
	.db $6F, $26, 22,  $ED, $6C, $7D, 0				; 22
	.db $6F, $26, 23,  $ED, $6C, $7D, 0				; 23
	.db $67, $87, $84, $87, $87, $87, 0				; 24
	
div_table:
	.db 2,  $CB, $3F, 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0   ; 2
	.db 8,  $6F, $26, 171, $ED, $6C, $7C, $CB, $3F, 0  , 0  , 0  , 0  , 0   ; 3
	.db 4,  $CB, $3F, $CB, $3F, 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0   ; 4
	.db 9,  $3C, $26, 51,  $28, $03, $6F, $ED, $6C, $7C, 0  , 0  , 0  , 0   ; 5
	.db 10, $6F, $26, 171, $ED, $6C, $7C, $CB, $3F, $CB, $3F, 0  , 0  , 0   ; 6
	.db 10, $6F, $26, 37,  $ED, $6C, $7C, $CB, $3F, $CB, $3F, 0  , 0  , 0   ; 7
	.db 5,  $1F, $1F, $1F, $E6, $1F, 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0   ; 8
	.db 8,  $26, 57,  $6F, $ED, $6C, $7C, $CB, $3F, 0  , 0  , 0  , 0  , 0   ; 9
	.db 11, $3C, $26, 51,  $28, $03, $6F, $ED, $6C, $7C, $CB, $3F, 0  , 0   ; 10
	.db 12, $6F, $26, 117, $ED, $6C, $84, $1F, $1F, $1F, $1F, $E6, $0F, 0   ; 11
	.db 11, $6F, $26, 171, $ED, $6C, $7C, $1F, $1F, $1F, $E6, $0F, 0  , 0   ; 12
	.db 10, $6F, $26, 79,  $ED, $6C, $7D, $CB, $3F, $CB, $3F, 0  , 0  , 0   ; 13
	.db 11, $6F, $26, 37,  $ED, $6C, $7C, $1F, $1F, $1F, $E6, $0F, 0  , 0   ; 14
	.db 9,  $3C, $26, 17,  $28, $03, $6F, $ED, $6C, $7C, 0  , 0  , 0  , 0   ; 15
	.db 6,  $1F, $1F, $1F, $1F, $E6, $0F, 0  , 0  , 0  , 0  , 0  , 0  , 0   ; 16
	.db 9,  $3C, $26, 15,  $28, $03, $6F, $ED, $6C, $7C, 0  , 0  , 0  , 0   ; 17
	.db 10, $26, 57,  $6F, $ED, $6C, $7C, $CB, $3F, $CB, $3F, 0  , 0  , 0   ; 18
	.db 6,  $6F, $26, 13,  $ED, $6C, $7C, 0,  0,   0,    0,   0,   0  , 0	; 19
	.db 13, $3C, $26, 51,  $28, $03, $6F, $ED, $6C, $7C, $CB, $3F, $CB, $3F ; 20
	
StoErrorMessage:
	.db "You can't store to a      number", 0
	
EndErrorMessage:
	.db "Too many Ends!", 0
	
GoodCompileMessage:
	.db "Succesfully compiled!", 0
	
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
	ex	de, hl
	ld b, (hl)
	inc hl
	ld e, 0
_:	inc hl
	ld a, (hl)
	ld d, 10
	mlt de
	sub a, 030h
	add a, e
	ld e, a
	djnz -_
	ld (ix+3), e
	jp _DeleteTempEditEqu
InputRoutineEnd:
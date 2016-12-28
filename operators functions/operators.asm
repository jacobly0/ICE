ExecuteOperator:
	ld (tempToken2), a
	ld hl, ExprOutput2
	ld (hl), OutputIsInHL
	cp tLE
	call z, SwapLEToGE
	cp tLT
	call z, SwapLTToGT
	ld (tempToken), a
	push bc
		ld hl, operators_booleans
		ld bc, 14
		cpir
		ld hl, operators_special
		add hl, bc
		ld c, (hl)
		ld a, c
		cp 4
		jr nz, +_
		set ans_set_z_flag, (iy+fExpression1)
_:		ld b, 15
		mlt bc
		ld hl, operator_start											; start of bunch jumps
		add hl, bc														; start of right operator
		ld c, (ix-8)
		ld b, 3
		mlt bc
		add hl, bc														; start of right subelement
		ld hl, (hl)
	pop bc
	jp (hl)
	
SubNumberXXX:
	ld a, (ix-4)
	dec a
	jr z, SubNumberVariable
	dec a
	jr z, SubNumberChainPush
	dec a
	jr z, SubNumberChainAns
	dec a
	jr z, SubNumberFunction
SubNumberNumber:
	set output_is_number, (iy+fExpression1)
	push bc
	pop hl
	or a
	sbc hl, de
	ld (ix-7), hl
	ret
SubNumberVariable:
	ld a, 021h
	push bc
	pop hl
	call InsertAHL															; ld hl, *
	jp SubChainAnsVariable
SubNumberChainPush:
	jp UnknownError
SubNumberChainAns:
	call MaybeChangeHLToDE
	ld a, 021h
	push bc
	pop de
	ld hl, 052EDB7h
	jp InsertADEHL															; ld hl, * \ or a \ sbc hl, de
SubNumberFunction:
	ld a, e
	push bc
		ld b, OutputInDE
		call GetFunction
		ld a, 021h
	pop de
	ld hl, 052EDB7h
	jp InsertADEHL															; ld hl, * \ or a \ sbc hl, de
SubVariableXXX:
	ld a, (ix-4)
	dec a
	jr z, SubVariableVariable
	dec a
	jr z, SubVariableChainPush
	dec a
	jr z, SubVariableChainAns
	dec a
	jp z, SubVariableFunction
SubVariableNumber:
	call InsertHIXC															; ld hl, (ix+*)
	jp SubChainAnsNumber
SubVariableVariable:
	ld a, c
	cp e
	jr nz, +_
	set output_is_number, (iy+fExpression1)
	ld (hl), typeNumber
	ld de, 0
	inc hl
	ld (hl), de
	ret
_:	call InsertHIXC															; ld hl, (ix+*)
	jp SubChainAnsVariable
SubVariableChainPush:
	jp UnknownError
SubVariableChainAns:
	call MaybeChangeHLToDE
	call InsertHIXC															; ld hl, (ix+*)
	ld hl, 052EDB7h
	jp InsertHL																; or a \ sbc hl, de
SubVariableFunction:
	ld a, e
	ld b, OutputInDE
	call GetFunction
	call InsertHIXC															; ld hl, (ix+*)
	ld hl, 052EDB7h
	jp InsertHL																; or a \ sbc hl, de
SubChainPushXXX:
	ld a, (ix-4)
	dec a
	jr z, SubChainPushVariable
	dec a
	jr z, SubChainPushChainPush
	dec a
	jr z, SubChainPushChainAns
	dec a
	jr z, SubChainPushFunction
SubChainPushNumber:
	ld a, 0E1h
	call InsertA															; pop hl
	jr SubChainAnsNumber
SubChainPushVariable:
	ld a, 0E1h
	call InsertA															; pop hl
	jr SubChainAnsVariable
SubChainPushChainPush:
	jp UnknownError
SubChainPushChainAns:
	call MaybeChangeHLToDE
	ld a, 0E1h
	ld hl, 052EDB7h
	jp InsertAHL															; pop hl \ or a \ sbc hl, de
SubChainPushFunction:
	ld a, 0E1h
	call InsertA															; pop hl
	jr SubChainAnsFunction
SubChainAnsXXX:
	ld a, (ix-4)
	dec a
	jr z, SubChainAnsVariable
	dec a
	jr z, SubChainAnsChainPush
	dec a
	jr z, SubChainAnsChainAns
	dec a
	jr z, SubChainAnsFunction
SubChainAnsNumber:
	or a
	sbc hl, hl
	ex de, hl
	sbc hl, de
	ret z
	ld e, 7
	sbc hl, de
	jr nc, SubHLDE
	add hl, de
	ld b, l
	ld a, (ExprOutput)
	or a
	jr z, ++_
	ld a, 02Bh
_:	call InsertA															; dec hl/de
	djnz -_
	ret
_:	ld a, OutputIsInDE
	ld (ExprOutput2), a
	ld a, 01Bh
	jr --_
SubHLDE:
	add hl, de
	call __ineg
	ld a, (ExprOutput)
	or a
	ld a, 011h
	jr nz, +_
	ld a, 021h
_:	call InsertAHL															; ld hl/de, 16777215-*
	ld a, 019h
	jp InsertA																; add hl, de
SubChainAnsVariable:
	call MaybeChangeDEToHL
	call InsertIXE															; ld de, (ix+*)
	ld hl, 052EDB7h
	jp InsertHL																; or a \ sbc hl, de
SubChainAnsChainPush:
	jp UnknownError
SubChainAnsChainAns:
	jp UnknownError
SubChainAnsFunction:
	ld a, (ExprOutput)
	add a, a
	add a, a
	add a, a
	add a, a
	add a, 0D5h
	call InsertA															; push hl/de
	ld a, e
	ld b, OutputInDE
	call GetFunction
	ld a, 0E1h
	ld hl, 052EDB7h
	jp InsertAHL															; pop hl \ or a \ sbc hl, de
SubFunctionXXX:
	ld a, (ix-4)
	dec a
	jr z, SubFunctionVariable
	dec a
	jr z, SubFunctionChainPush
	dec a
	jr z, SubFunctionChainAns
	dec a
	jr z, SubFunctionFunction
SubFunctionNumber:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jp SubChainAnsNumber
SubFunctionVariable:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jp SubChainAnsVariable
SubFunctionChainPush:
	jp UnknownError
SubFunctionChainAns:
	ld a, (ExprOutput)
	add a, a
	add a, a
	add a, a
	add a, a
	add a, 0D5h
	call InsertA															; push hl/de
	ld a, c
	ld b, OutputInHL
	call GetFunction
	ld a, 0E1h
	ld hl, 052EDB7h
	jp InsertAHL															; pop de \ or a \ sbc hl, de
SubFunctionFunction:
	ld a, e
	ld b, OutputInHL
	call GetFunction
	ld a, c
	ld b, OutputInDE
	set need_push, (iy+fExpression1)
	call GetFunction
	ld hl, 052EDB7h
	jp InsertHL																; or a \ sbc hl, de

AddNumberXXX:
	ld a, (ix-4)
	dec a
	jr z, AddNumberVariable
	dec a
	jr z, AddNumberChainPush
	dec a
	jr z, AddNumberChainAns
	dec a
	jr z, AddNumberFunction
AddNumberNumber:
	set output_is_number, (iy+fExpression1)
	ex de, hl
	add hl, bc
	ld (ix-7), hl
	ret
AddNumberVariable:
	push bc
	pop hl
	ex de, hl
	push hl
	pop bc
	jr AddVariableNumber
AddNumberChainPush:
	jp UnknownError
AddNumberChainAns:
	push bc
	pop de
	jp AddChainAnsNumber
AddNumberFunction:
	push bc
	pop hl
	ex de, hl
	push hl
	pop bc
	jp AddFunctionNumber
AddVariableXXX:
	ld a, (ix-4)
	dec a
	jr z, AddVariableVariable
	dec a
	jr z, AddVariableChainPush
	dec a
	jr z, AddVariableChainAns
	dec a
	jp z, AddVariableFunction
AddVariableNumber:
	call InsertHIXC															; ld hl, (ix+*)
	jp AddChainAnsNumber
AddVariableVariable:
	call InsertHIXC															; ld hl, (ix+*)
	jp AddChainAnsVariable
AddVariableChainPush:
	ld a, 0E1h
	call InsertA															; pop hl
	ld e, c
	jp AddChainAnsVariable
AddVariableChainAns:
	ld e, c
	jp AddChainAnsVariable
AddVariableFunction:
	ld a, e
	ld b, OutputInHL
	call GetFunction
	ld e, c
	call InsertIXE															; ld de, (ix+*)
	ld a, 019h
	jp InsertA																; add hl, de
AddChainPushXXX:
	ld a, (ix-4)
	dec a
	jr z, AddChainPushVariable
	dec a
	jr z, AddChainPushChainPush
	dec a
	jr z, AddChainPushChainAns
	dec a
	jp z, AddChainPushFunction
AddChainPushNumber:
	ld a, 0E1h
	call InsertA															; pop hl
	jr AddChainAnsNumber
AddChainPushVariable:
	ld a, 0E1h
	call InsertA															; pop hl
	jr AddChainAnsVariable
AddChainPushChainPush:
	jp UnknownError
AddChainPushChainAns:
	ld a, (ExprOutput)
	or a
	ld a, 0D1h
	jr nz, +_
	ld a, 0E1h
_:	call InsertA															; pop hl/de
	ld a, 019h
	jp InsertA																; add hl, de
AddChainPushFunction:
	ld a, 0E1h
	call InsertA															; pop hl
	jr AddChainAnsFunction
AddChainAnsXXX:
	ld a, (ix-4)
	dec a
	jr z, AddChainAnsVariable
	dec a
	jr z, AddChainAnsChainPush
	dec a
	jr z, AddChainAnsChainAns
	dec a
	jp z, AddChainAnsFunction
AddChainAnsNumber:
	or a
	sbc hl, hl
	ex de, hl
	sbc hl, de
	ret z
	ld e, 6
	sbc hl, de
	jr nc, AddHLDE
	add hl, de
	ld b, l
	ld a, (ExprOutput)
	or a
	jr z, ++_
	ld a, 023h
_:	call InsertA															; inc hl/de
	djnz -_
	ret
_:	ld a, OutputIsInDE
	ld (ExprOutput2), a
	ld a, 013h
	jr --_
AddHLDE:
	add hl, de
	ld a, (ExprOutput)
	or a
	ld a, 011h
	jr nz, +_
	ld a, 021h
_:	call InsertAHL															; ld hl/de, *
	ld a, 019h
	jp InsertA																; add hl, de
AddChainAnsVariable:
	ld h, 017h
	ld d, 3
	mlt de
	ld a, e
	ld l, 0DDh
	call _SetHLUToA
	ld a, (ExprOutput)
	or a
	jr nz, +_
	ld h, 027h
_:	call InsertHL															; ld hl/de, (ix+*)
	ld a, 019h
	jp InsertA																; add hl, de
AddChainAnsChainPush:
	jp UnknownError
AddChainAnsChainAns:
	jp UnknownError
AddChainAnsFunction:
	ld a, (ExprOutput)
	add a, a
	add a, a
	add a, a
	add a, a
	add a, 0D5h
	call InsertA															; push hl/de
	ld a, e
	ld b, OutputInHL
	call GetFunction
	ld a, 0D1h
	call InsertA															; pop de
	ld a, 019h
	jp InsertA																; add hl, de
AddFunctionXXX:
	ld a, (ix-4)
	dec a
	jr z, AddFunctionVariable
	dec a
	jr z, AddFunctionChainPush
	dec a
	jr z, AddFunctionChainAns
	dec a
	jr z, AddFunctionFunction
AddFunctionNumber:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jp AddChainAnsNumber
AddFunctionVariable:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jp AddChainAnsVariable
AddFunctionChainPush:
	jp UnknownError
AddFunctionChainAns:
	ld a, (ExprOutput)
	add a, a
	add a, a
	add a, a
	add a, a
	add a, 0D5h
	call InsertA															; push hl/de
	ld a, c
	ld b, OutputInHL
	call GetFunction
	ld a, 0D1h
	call InsertA															; pop de
	ld a, 019h
	jp InsertA																; add hl, de
AddFunctionFunction:
	ld a, e
	ld b, OutputInHL
	call GetFunction
	ld a, c
	ld b, OutputInDE
	set need_push, (iy+fExpression1)
	call GetFunction
	ld a, 019h
	jp InsertA																; add hl, de
	
DivNumberXXX:
	ld a, (ix-4)
	dec a
	jr z, DivNumberVariable
	dec a
	jr z, DivNumberChainPush
	dec a
	jr z, DivNumberChainAns
	dec a
	jr z, DivNumberFunction
DivNumberNumber:
	set output_is_number, (iy+fExpression1)
	push bc
	pop hl
	push de
	pop bc
	call __idvrmu
	ld (ix-7), de
	ret
DivNumberVariable:
	ld a, 021h
	push bc
	pop hl
	call InsertAHL															; ld hl, *
	jp DivChainAnsVariable
DivNumberChainPush:
	jp UnknownError
DivNumberChainAns:
	ld de, 021C1E5h
	push bc
	pop hl
	ld a, (ExprOutput)
	or a
	jr nz, +_
	ld e, 0D5h
_:	call InsertDEHL															; push hl/de \ pop bc \ ld hl, *
	jr DivInsert
DivNumberFunction:
	ld a, e
	push bc
		ld b, OutputInBC
		call GetFunction
		ld a, 021h
	pop hl
	call InsertAHL															; ld hl, *
	jr DivInsert
DivVariableXXX:
	ld a, (ix-4)
	dec a
	jr z, DivVariableVariable
	dec a
	jr z, DivVariableChainPush
	dec a
	jr z, DivVariableChainAns
	dec a
	jp z, DivVariableFunction
DivVariableNumber:
	call InsertHIXC															; ld hl, (ix+*)
	jp DivChainAnsNumber
DivVariableVariable:
	ld a, c
	cp e
	jr nz, +_
	set output_is_number, (iy+fExpression1)
	ld (hl), typeNumber
	ld de, 1
	inc hl
	ld (hl), de
	ret
_:	call InsertHIXC															; ld hl, (ix+*)
	jp DivChainAnsVariable
DivVariableChainPush:
	jp UnknownError
DivVariableChainAns:
	ld a, (ExprOutput)
	add a, a
	add a, a
	add a, a
	add a, a
	add a, 0D5h
	call InsertA															; push hl/de
	ld a, 0C1h
	call InsertA															; pop bc
	call InsertHIXC															; ld hl, (ix+*)
	jr DivInsert
DivVariableFunction:
	ld a, e
	ld b, OutputInBC
	call GetFunction
	call InsertHIXC															; ld hl, (ix+*)
DivInsert:
	ld a, OutputIsInDE
	ld (ExprOutput2), a
	ld a, 0CDh
	ld hl, __idvrmu
	jp InsertAHL															; call __idvrmu
DivChainPushXXX:
	ld a, (ix-4)
	dec a
	jr z, DivChainPushVariable
	dec a
	jr z, DivChainPushChainPush
	dec a
	jr z, DivChainPushChainAns
	dec a
	jp z, DivChainPushFunction
DivChainPushNumber:
	ld a, 0E1h
	call InsertA															; pop hl
	jr DivChainAnsNumber
DivChainPushVariable:
	ld a, 0E1h
	call InsertA															; pop hl
	jr DivChainAnsVariable
DivChainPushChainPush:
	jp UnknownError
DivChainPushChainAns:
	ld hl, 0E1C1E5h
	ld a, (ExprOutput)
	add a, a
	add a, a
	add a, a
	add a, a
	add a, 0D5h
	ld l, a
	call InsertHL															; push hl/de \ pop bc \ pop hl
_:	jr DivInsert
DivChainPushFunction:
	ld a, 0E1h
	call InsertA															; pop hl
	jr DivChainAnsFunction
DivChainAnsXXX:
	ld a, (ix-4)
	dec a
	jr z, DivChainAnsVariable
	dec a
	jr z, DivChainAnsChainPush
	dec a
	jr z, DivChainAnsChainAns
	dec a
	jp z, DivChainAnsFunction
DivChainAnsNumber:
	call MaybeChangeDEToHL
	ld a, 001h
	ex de, hl
	ld de, 256
	or a
	sbc hl, de
	jr nc, +_
	add hl, de
	ld c, l
	ld a, 03Eh
	call InsertA															; ld a, *
	ld a, c
	call InsertA															; ld a, *
	ld a, 0CDh
	ld hl, _DivHLByA
	jp InsertAHL															; call _DivHLByA
_:	add hl, de
	call InsertAHL															; ld bc, *
_:	jr ---_
DivChainAnsVariable:
	call MaybeChangeDEToHL
	call InsertIXC
	jr -_
DivChainAnsChainPush:
	jp UnknownError
DivChainAnsChainAns:
	jp UnknownError
DivChainAnsFunction:
	ld a, (ExprOutput)
	add a, a
	add a, a
	add a, a
	add a, a
	add a, 0D5h
	call InsertA															; push hl/de
	ld a, e
	ld b, OutputInBC
	call GetFunction
	ld a, 0E1h
	call InsertA															; pop hl
_:	jr --_
DivFunctionXXX:
	ld a, (ix-4)
	dec a
	jr z, DivFunctionVariable
	dec a
	jr z, DivFunctionChainPush
	dec a
	jr z, DivFunctionChainAns
	dec a
	jr z, DivFunctionFunction
DivFunctionNumber:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jp DivChainAnsNumber
DivFunctionVariable:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jp DivChainAnsVariable
DivFunctionChainPush:
	jp UnknownError
DivFunctionChainAns:
	ld a, (ExprOutput)
	add a, a
	add a, a
	add a, a
	add a, a
	add a, 0D5h
	call InsertA															; push hl/de
	ld a, 0C1h
	call InsertA															; pop bc
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jr -_
DivFunctionFunction:
	ld a, e
	ld b, OutputInBC
	call GetFunction
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jr -_
	
MulNumberXXX:
	ld a, (ix-4)
	dec a
	jr z, MulNumberVariable
	dec a
	jr z, MulNumberChainPush
	dec a
	jr z, MulNumberChainAns
	dec a
	jr z, MulNumberFunction
MulNumberNumber:
	set output_is_number, (iy+fExpression1)
	ex de, hl
	call __imulu
	ld (ix-7), hl
	ret
MulNumberVariable:
	push bc
	pop hl
	ex de, hl
	push hl
	pop bc
	jr MulVariableNumber
MulNumberChainPush:
	jp UnknownError
MulNumberChainAns:
	push bc
	pop de
	jp MulChainAnsNumber
MulNumberFunction:
	push bc
	pop hl
	ex de, hl
	push hl
	pop bc
	jp MulFunctionNumber
MulVariableXXX:
	ld a, (ix-4)
	dec a
	jr z, MulVariableVariable
	dec a
	jr z, MulVariableChainPush
	dec a
	jr z, MulVariableChainAns
	dec a
	jp z, MulVariableFunction
MulVariableNumber:
	call InsertHIXC															; ld hl, (ix+*)
	jp MulChainAnsNumber
MulVariableVariable:
	call InsertHIXC															; ld hl, (ix+*)
	jp MulChainAnsVariable
MulVariableChainPush:
	ld a, 0E1h
	call InsertA															; pop hl
	ld e, c
	jp MulChainAnsVariable
MulVariableChainAns:
	ld e, c
	jp MulChainAnsVariable
MulVariableFunction:
	ld a, e
	ld b, OutputInHL
	call GetFunction
	call InsertIXC
	ld hl, __imulu
	jp InsertCallHL															; call __imulu
MulChainPushXXX:
	ld a, (ix-4)
	dec a
	jr z, MulChainPushVariable
	dec a
	jr z, MulChainPushChainPush
	dec a
	jr z, MulChainPushChainAns
	dec a
	jp z, MulChainPushFunction
MulChainPushNumber:
	ld a, 0E1h
	call InsertA															; pop hl
	jr MulChainAnsNumber
MulChainPushVariable:
	ld a, 0E1h
	call InsertA															; pop hl
	jp MulChainAnsVariable
MulChainPushChainPush:
	jp UnknownError
MulChainPushChainAns:
	call MaybeChangeDEToHL
	ld a, 0C1h
	call InsertA															; pop bc
	ld a, 0CDh
	ld hl, __imulu
	jp InsertAHL															; call __imulu
MulChainPushFunction:
	ld a, 0E1h
	call InsertA															; pop hl
	jp MulChainAnsFunction
MulChainAnsXXX:
	ld a, (ix-4)
	dec a
	jr z, MulChainAnsVariable
	dec a
	jp z, MulChainAnsChainPush
	dec a
	jp z, MulChainAnsChainAns
	dec a
	jp z, MulChainAnsFunction
MulChainAnsNumber:
	call _ChkDEIs0
	jr nz, +_
	ld hl, 062EDB7h
	jp InsertHL																; or a \ sbc hl, hl
_:	call MaybeChangeDEToHL
	dec de
	call _ChkDEIs0
	ret z
	inc de
	ex de, hl
	ld de, 21
	or a
	sbc hl, de
	jr nc, ++_
	add hl, de
	dec l
	dec l
	ld h, 10
	mlt hl
	ld de, MulTable
	add hl, de
	ld b, (hl)
_:	inc hl
	ld a, (hl)
	call InsertA
	djnz -_
	ret
_:	add hl, de
	ld de, 256
	or a
	sbc hl, de
	jr c, +_
	ld a, 001h
	call InsertAHL															; ld bc, *
	ld a, 0CDh
	ld hl, __imulu
	jp InsertAHL															; call __imulu
_:	add hl, de
	ld c, l
	ld a, 03Eh
	call InsertA															; ld a, *
	ld a, c
	call InsertA															; ld a, *
	ld a, 0CDh
	ld hl, __imul_b
	jp InsertAHL															; call __imul_b
MulChainAnsVariable:
	call MaybeChangeDEToHL
	call InsertIXC															; ld bc, (ix+*)
	ld a, 0CDh
	ld hl, __imulu
	jp InsertAHL															; call __imulu
MulChainAnsChainPush:
	jp UnknownError
MulChainAnsChainAns:
	jp UnknownError
MulChainAnsFunction:
	ld a, (ExprOutput)
	add a, a
	add a, a
	add a, a
	add a, a
	add a, 0D5h
	call InsertA															; push hl/de
	ld a, e
	ld b, OutputInBC
	call GetFunction
	ld a, 0E1h
	call InsertA															; pop hl
	ld a, 0CDh
	ld hl, __imulu
	jp InsertAHL															; call __imulu
MulFunctionXXX:
	ld a, (ix-4)
	dec a
	jr z, MulFunctionVariable
	dec a
	jr z, MulFunctionChainPush
	dec a
	jr z, MulFunctionChainAns
	dec a
	jr z, MulFunctionFunction
MulFunctionNumber:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jp MulChainAnsNumber
MulFunctionVariable:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jp MulChainAnsVariable
MulFunctionChainPush:
	jp UnknownError
MulFunctionChainAns:
	ld e, c
	jr MulChainAnsFunction
MulFunctionFunction:
	ld a, e
	ld b, OutputInBC
	call GetFunction
	ld a, c
	ld b, OutputInHL
	call GetFunction
	ld a, 0CDh
	ld hl, __imulu
	jp InsertAHL															; call __imulu

SwapLEToGE:
	ld a, tGE
	jr $+4	
SwapLTToGT:
	ld a, tGT
	push bc
	pop hl
	ld c, (ix-4)
	ld b, (ix-8)
	ld (ix-4), b
	ld (ix-8), c
	ex de, hl
	push hl
	pop bc
	ret

GLETNumberXXX:
	ld a, (ix-4)
	dec a
	jr z, GLETNumberVariable
	dec a
	jr z, GLETNumberChainPush
	dec a
	jr z, GLETNumberChainAns
	dec a
	jr z, GLETNumberFunction
GLETNumberNumber:
	set output_is_number, (iy+fExpression1)
	push bc
	pop hl
	ld a, (tempToken)
	cp tGE
	jr z, +_
	scf
_:	sbc hl, de
	sbc hl, hl
	inc hl
	ld (ix-7), hl
	ret
GLETNumberVariable:
	ld a, 021h
	push bc
	pop hl
	call InsertAHL															; ld hl, *
	jp GLETChainAnsVariable
GLETNumberChainPush:
	jp UnknownError
GLETNumberChainAns:
	call MaybeChangeHLToDE
	ld a, 021h
	push bc
	pop hl
	call InsertAHL															; ld hl, *
	jr GLETInsert
GLETNumberFunction:
	ld a, e
	push bc
		ld b, OutputInDE
		call GetFunction
		ld a, 021h
	pop hl
	call InsertAHL															; ld hl, *
	jr GLETInsert
GLETVariableXXX:
	ld a, (ix-4)
	dec a
	jr z, GLETVariableVariable
	dec a
	jr z, GLETVariableChainPush
	dec a
	jr z, GLETVariableChainAns
	dec a
	jp z, GLETVariableFunction
GLETVariableNumber:
	call InsertHIXC															; ld hl, (ix+*)
	jp GLETChainAnsNumber
GLETVariableVariable:
	ld a, c
	cp e
	jr nz, ++_
	set output_is_number, (iy+fExpression1)
	ld a, (tempToken)
	sub tGT
	jr z, +_
	ld a, 1
_:	ld de, 0
	ld e, a
	ld (hl), typeNumber
	inc hl
	ld (hl), de
	ret
_:	call InsertHIXC															; ld hl, (ix+*)
	jp GLETChainAnsVariable
GLETVariableChainPush:
	jp UnknownError
GLETVariableChainAns:
	call MaybeChangeHLToDE
	call InsertHIXC															; ld hl, (ix+*)
	jr GLETInsert
GLETVariableFunction:
	ld a, e
	ld b, OutputInDE
	call GetFunction
	call InsertHIXC															; ld hl, (ix+*)
GLETInsert:
	ld de, 052ED37h
	ld hl, 02362EDh
	ld a, (tempToken)
	cp tGT
	jr z, +_
	ld e, 0B7h
_:	jp InsertDEHL															; scf/or a \ sbc hl, de \ sbc hl, hl \ inc hl
GLETChainPushXXX:
	ld a, (ix-4)
	dec a
	jr z, GLETChainPushVariable
	dec a
	jr z, GLETChainPushChainPush
	dec a
	jr z, GLETChainPushChainAns
	dec a
	jp z, GLETChainPushFunction
GLETChainPushNumber:
	ld a, 0E1h
	call InsertA															; pop hl
	jr GLETChainAnsNumber
GLETChainPushVariable:
	ld a, 0E1h
	call InsertA															; pop hl
	jr GLETChainAnsVariable
GLETChainPushChainPush:
	jp UnknownError
GLETChainPushChainAns:
	call MaybeChangeHLToDE
	ld a, 0E1h
	call InsertA															; pop hl
	jr GLETInsert
GLETChainPushFunction:
	ld a, e
	ld b, OutputInDE
	call GetFunction
	ld a, 0E1h
	call InsertA															; pop hl
_:	jr GLETInsert
GLETChainAnsXXX:
	ld a, (ix-4)
	dec a
	jr z, GLETChainAnsVariable
	dec a
	jr z, GLETChainAnsChainPush
	dec a
	jr z, GLETChainAnsChainAns
	dec a
	jp z, GLETChainAnsFunction
GLETChainAnsNumber:
	call MaybeChangeDEToHL
	ld hl, tempToken
	ld a, (hl)
	cp tGT
	jr nz, +_
	inc de
	ld a, tGE
	ld (hl), a
_:	ld a, 011h
	ex de, hl
	call InsertAHL															; ld de, *
	jr --_
GLETChainAnsVariable:
	call MaybeChangeDEToHL
	call InsertIXE															; ld de, (ix+*)
	jr --_
GLETChainAnsChainPush:
	jp UnknownError
GLETChainAnsChainAns:
	jp UnknownError
GLETChainAnsFunction:
	ld a, (ExprOutput)
	add a, a
	add a, a
	add a, a
	add a, a
	add a, 0D5h
	call InsertA															; push hl/de
	ld a, e
	ld b, OutputInDE
	call GetFunction
	ld a, 0E1h
	call InsertA															; pop hl
_:	jr --_
GLETFunctionXXX:
	ld a, (ix-4)
	dec a
	jr z, GLETFunctionVariable
	dec a
	jr z, GLETFunctionChainPush
	dec a
	jr z, GLETFunctionChainAns
	dec a
	jr z, GLETFunctionFunction
GLETFunctionNumber:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jp GLETChainAnsNumber
GLETFunctionVariable:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jp GLETChainAnsVariable
GLETFunctionChainPush:
	jp UnknownError
GLETFunctionChainAns:
	ld a, (ExprOutput)
	add a, a
	add a, a
	add a, a
	add a, a
	add a, 0D5h
	call InsertA															; push hl/de
	ld a, c
	ld b, OutputInHL
	call GetFunction
	ld a, 0D1h
	call InsertA															; pop de
	jr -_
GLETFunctionFunction:
	ld a, e
	ld b, OutputInDE
	call GetFunction
	ld a, c
	ld b, OutputInHL
	set need_push, (iy+fExpression1)
	call GetFunction
	jr -_
	
NEQNumberXXX:
	ld a, (ix-4)
	dec a
	jr z, NEQNumberVariable
	dec a
	jr z, NEQNumberChainPush
	dec a
	jr z, NEQNumberChainAns
	dec a
	jr z, NEQNumberFunction
NEQNumberNumber:
	set output_is_number, (iy+fExpression1)
	ex de, hl
	ld a, (tempToken)
	cp tEq
	jr z, +_
	or a
	sbc hl, bc
	ld hl, 0
	jr z, $+3
	inc hl
	jr ++_
_:	sbc hl, bc
	ld hl, 0
	jr nz, $+3
	inc hl
_:	ld (ix-7), hl
	ret
NEQNumberVariable:
	push bc
	pop hl
	ex de, hl
	push hl
	pop bc
	jr NEQVariableNumber
NEQNumberChainPush:
	jp UnknownError
NEQNumberChainAns:
	push bc
	pop de
	jp NEQChainAnsNumber
NEQNumberFunction:
	push bc
	pop hl
	ex de, hl
	push hl
	pop bc
	jp NEQFunctionNumber
NEQVariableXXX:
	ld a, (ix-4)
	dec a
	jr z, NEQVariableVariable
	dec a
	jr z, NEQVariableChainPush
	dec a
	jr z, NEQVariableChainAns
	dec a
	jp z, NEQVariableFunction
NEQVariableNumber:
	call InsertHIXC															; ld hl, (ix+*)
	jp NEQChainAnsNumber
NEQVariableVariable:
	ld a, c
	cp e
	jr nz, ++_
	set output_is_number, (iy+fExpression1)
	ld a, (tempToken)
	sub tNE
	jr z, +_
	ld a, 1
_:	ld de, 0
	ld e, a
	ld (hl), typeNumber
	inc hl
	ld (hl), de
	ret
_:	call InsertHIXC															; ld hl, (ix+*)
	jp NEQChainAnsVariable
NEQVariableChainPush:
	jp UnknownError
NEQVariableChainAns:
	ld e, c
	jp NEQChainAnsVariable
NEQVariableFunction:
	ld a, e
	ld b, OutputInDE
	call GetFunction
	call InsertHIXC															; ld hl, (ix+*)
NEQInsert:
	ld hl, 052EDB7h
	call InsertHL															; or a \ sbc hl, de
	ld de, 0
	ld hl, 0230120h
	ld a, (tempToken)
	cp tEq
	jr z, +_
	ld l, 028h
_:	ld a, 021h
	jp InsertADEHL															; ld hl, 0 \ jr [n]z, $+3 \ inc hl
NEQChainPushXXX:
	ld a, (ix-4)
	dec a
	jr z, NEQChainPushVariable
	dec a
	jr z, NEQChainPushChainPush
	dec a
	jr z, NEQChainPushChainAns
	dec a
	jp z, NEQChainPushFunction
NEQChainPushNumber:
	ld a, 0E1h
	call InsertA															; pop hl
	jr NEQChainAnsNumber
NEQChainPushVariable:
	ld a, 0E1h
	call InsertA															; pop hl
	jr NEQChainAnsVariable
NEQChainPushChainPush:
	jp UnknownError
NEQChainPushChainAns:
	ld a, (ExprOutput)
	or a
	ld a, 0D1h
	jr nz, +_
	ld a, 0E1h
_:	call InsertA															; pop hl/de
	jr NEQInsert
NEQChainPushFunction:
	ld a, e
	ld b, OutputInHL
	call GetFunction
	ld a, 0D1h
	call InsertA															; pop de
_:	jr NEQInsert
NEQChainAnsXXX:
	ld a, (ix-4)
	dec a
	jr z, NEQChainAnsVariable
	dec a
	jr z, NEQChainAnsChainPush
	dec a
	jr z, NEQChainAnsChainAns
	dec a
	jp z, NEQChainAnsFunction
NEQChainAnsNumber:
	bit op_is_last_one, (iy+fExpression1)
	jr z, +_
	ld a, (tempToken)
	cp tNE
	jr nz, +_
	res ans_set_z_flag, (iy+fExpression1)
_:	ld a, (ExprOutput)
	or a
	ld a, 011h
	jr nz, $+4
	ld a, 021h
	ex de, hl
	call InsertAHL															; ld hl/de, *
	jr --_
NEQChainAnsVariable:
	ld h, 017h
	ld d, 3
	mlt de
	ld a, e
	ld l, 0DDh
	call _SetHLUToA
	ld a, (ExprOutput)
	or a
	jr nz, $+4
	ld h, 027h
	call InsertHL															; ld hl/de, (ix+*)
	jr --_
NEQChainAnsChainPush:
	jp UnknownError
NEQChainAnsChainAns:
	jp UnknownError
NEQChainAnsFunction:
	ld a, (ExprOutput)
	add a, a
	add a, a
	add a, a
	add a, a
	add a, 0D5h
	call InsertA															; push hl/de
	ld a, e
	ld b, OutputInHL
	call GetFunction
	ld a, 0D1h
	call InsertA															; pop de
_:	jp NEQInsert
NEQFunctionXXX:
	ld a, (ix-4)
	dec a
	jr z, NEQFunctionVariable
	dec a
	jr z, NEQFunctionChainPush
	dec a
	jr z, NEQFunctionChainAns
	dec a
	jr z, NEQFunctionFunction
NEQFunctionNumber:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jp NEQChainAnsNumber
NEQFunctionVariable:
	ld a, c
	ld c, e
	ld e, a
	jp NEQVariableFunction
NEQFunctionChainPush:
	jp UnknownError
NEQFunctionChainAns:
	ld e, c
	jr NEQChainAnsFunction
NEQFunctionFunction:
	ld a, e
	ld b, OutputInDE
	call GetFunction
	ld a, c
	ld b, OutputInHL
	set need_push, (iy+fExpression1)
	call GetFunction
	jr -_

XORANDNumberXXX:
	ld a, (ix-4)
	dec a
	jr z, XORANDNumberVariable
	dec a
	jr z, XORANDNumberChainPush
	dec a
	jr z, XORANDNumberChainAns
	dec a
	jr z, XORANDNumberFunction
XORANDNumberNumber:
	set output_is_number, (iy+fExpression1)
	or a
	sbc hl, hl
	ld a, (tempToken)
	cp tXor
	jr z, ++_
	cp tOr
	jr z, +_
	call _ChkBCIs0
	jr z, +++_
	call _ChkDEIs0
	jr z, +++_
	inc hl
	jr +++_
_:	call _ChkBCIs0
	ld b, a
	call _ChkDEIs0
	or a, b
	jr z, ++_
	inc hl
	jr ++_
_:	call _ChkDEIs0
	ld de, 0
	jr z, $+3
	inc de
	call _ChkBCIs0
	ld a, 0
	jr z, $+3
	inc a
	xor e
	ld e, a
	ex de, hl
_:	ld (ix-7), hl
	ret
XORANDNumberVariable:
	push bc
	pop hl
	ex de, hl
	push hl
	pop bc
	jr XORANDVariableNumber
XORANDNumberChainPush:
	jp UnknownError
XORANDNumberChainAns:
	push bc
	pop de
	jp XORANDChainAnsNumber
XORANDNumberFunction:
	push bc
	pop hl
	ex de, hl
	push hl
	pop bc
	jp XORANDFunctionNumber
XORANDVariableXXX:
	ld a, (ix-4)
	dec a
	jr z, XORANDVariableVariable
	dec a
	jr z, XORANDVariableChainPush
	dec a
	jr z, XORANDVariableChainAns
	dec a
	jp z, XORANDVariableFunction
XORANDVariableNumber:
	call InsertHIXC															; ld hl, (ix+*)
	jp XORANDChainAnsNumber
XORANDVariableVariable:
	ld a, c
	cp e
	jr nz, ++_
	set output_is_number, (iy+fExpression1)
	ld a, (tempToken)
	sub tNE
	jr z, +_
	ld a, 1
_:	ld de, 0
	ld e, a
	ld (hl), typeNumber
	inc hl
	ld (hl), de
	ret
_:	call InsertHIXC															; ld hl, (ix+*)
	jp XORANDChainAnsVariable
XORANDVariableChainPush:
	jp UnknownError
XORANDVariableChainAns:
	ld e, c
	jp XORANDChainAnsVariable
XORANDVariableFunction:
	ld a, e
	ld b, OutputInDE
	call GetFunction
	call InsertHIXC															; ld hl, (ix+*)
XORANDInsert:
	ld de, (programPtr)
	ld hl, XORANDData
	ld b, 0A2h
	ld a, (tempToken)
	cp tXor
	jr nz, +_
	ld b, 0AAh
_:	cp tOr
	jr nz, +_
	ld b, 0B2h
_:	ld a, b
	ld (XORANDSMC), a
	ld bc, 16
	ldir
	ld (programPtr), de
	ret
XORANDChainPushXXX:
	ld a, (ix-4)
	dec a
	jr z, XORANDChainPushVariable
	dec a
	jr z, XORANDChainPushChainPush
	dec a
	jr z, XORANDChainPushChainAns
	dec a
	jp z, XORANDChainPushFunction
XORANDChainPushNumber:
	ld a, 0E1h
	call InsertA															; pop hl
	jr XORANDChainAnsNumber
XORANDChainPushVariable:
	ld a, 0E1h
	call InsertA															; pop hl
	jr XORANDChainAnsVariable
XORANDChainPushChainPush:
	jp UnknownError
XORANDChainPushChainAns:
	ld a, 0D1h
	call InsertA															; pop de
	jr XORANDInsert
XORANDChainPushFunction:
	ld a, e
	ld b, OutputInHL
	call GetFunction
	ld a, 0D1h
	call InsertA															; pop de
_:	jr XORANDInsert
XORANDChainAnsXXX:
	ld a, (ix-4)
	dec a
	jr z, XORANDChainAnsVariable
	dec a
	jr z, XORANDChainAnsChainPush
	dec a
	jr z, XORANDChainAnsChainAns
	dec a
	jp z, XORANDChainAnsFunction
XORANDChainAnsNumber:
	ld a, (ExprOutput)
	or a
	ld a, 011h
	jr nz, +_
	ld a, 021h
_:	ex de, hl
	call InsertAHL															; ld hl/de, *
	jr --_
XORANDChainAnsVariable:
	ld h, 017h
	ld d, 3
	mlt de
	ld a, e
	ld l, 0DDh
	call _SetHLUToA
	ld a, (ExprOutput)
	or a
	jr nz, $+4
	ld h, 027h
	call InsertHL															; ld hl/de, (ix+*)
	jr --_
XORANDChainAnsChainPush:
	jp UnknownError
XORANDChainAnsChainAns:
	jp UnknownError
XORANDChainAnsFunction:
	ld a, (ExprOutput)
	add a, a
	add a, a
	add a, a
	add a, a
	add a, 0D5h
	call InsertA															; push hl/de
	ld a, e
	ld b, OutputInHL
	call GetFunction
	ld a, 0D1h
	call InsertA															; pop de
_:	jr ---_
XORANDFunctionXXX:
	ld a, (ix-4)
	dec a
	jr z, XORANDFunctionVariable
	dec a
	jr z, XORANDFunctionChainPush
	dec a
	jr z, XORANDFunctionChainAns
	dec a
	jr z, XORANDFunctionFunction
XORANDFunctionNumber:
	ld a, c
	ld b, OutputInHL
	call GetFunction
	jp XORANDChainAnsNumber
XORANDFunctionVariable:
	ld a, c
	ld c, e
	ld e, a
	jp XORANDVariableFunction
XORANDFunctionChainPush:
	jp UnknownError
XORANDFunctionChainAns:
	ld e, c
	jr XORANDChainAnsFunction
XORANDFunctionFunction:
	ld a, e
	ld b, OutputInDE
	call GetFunction
	ld a, c
	ld b, OutputInHL
	set need_push, (iy+fExpression1)
	call GetFunction
	jr -_
	
StoNumberXXX:
	ld a, (ix-4)
	cp typeChainAns
	jr z, +_
	cp typeVariable
	jp nz, StoError
	push bc
	pop hl
	ld a, 021h
	call InsertAHL															; ld hl, XXXXXX
	ld hl, 0002FDDh
	ld d, 3
	mlt de
	ld a, e
	call _SetHLUToA
	jp InsertHL																; ld (ix+*), hl
_:	bit last_op_was_pointer, (iy+fExpression3)
	jp z, StoError
	call RemoveGetPointerAndCheckIfNumber
	jr nz, +_
	ld a, OutputIsInDE
	ld (ExprOutput2), a
	ld a, 011h
	push bc
	pop hl
	call InsertAHL															; ld de, XXXXXX
	ld a, 0EDh
	call InsertA															; ld (hl), de
	ld a, 01Fh
	jp InsertA																; ld (hl), de
_:	push hl
		ld a, 021h
		push bc
		pop hl
		call InsertAHL														; ld hl, XXXXXX
	pop hl
	ld a, 022h
	jp InsertAHL															; ld (XXXXXX), hl
StoVariableXXX:
	ld a, (ix-4)
	cp typeChainAns
	jr z, +_
	cp typeVariable
	jp nz, StoError
	call InsertHIXC															; ld hl, (ix+*)
	ld hl, 0002FDDh
	ld d, 3
	mlt de
	ld a, e
	call _SetHLUToA
	jp InsertHL																; ld (ix+*), hl
_:	bit last_op_was_pointer, (iy+fExpression3)
	jp z, StoError
	call RemoveGetPointerAndCheckIfNumber
	jr nz, +_
	ld a, OutputIsInDE
	ld (ExprOutput2), a
	ld hl, 00017DDh
	ld b, 3
	mlt bc
	ld a, c
	call _SetHLUToA
	call InsertHL															; ld de, (ix+*)
	ld a, 0EDh
	call InsertA															; ld (hl), de
	ld a, 01Fh
	jp InsertA																; ld (hl), de
_:	push hl
		call InsertHIXC														; ld hl, (ix+*)
	pop hl
	ld a, 022h
	jp InsertAHL															; ld (XXXXXX), hl
StoChainPushXXX:
	ld a, (ix-4)
	cp typeChainAns
	jp nz, StoError
	bit last_op_was_pointer, (iy+fExpression3)
	jp z, StoError
	ld hl, (programPtr)
	ld (hl), 01Fh
	inc hl
	ld (programPtr), hl
	dec hl
	dec hl
	ld (hl), 0EDh
	dec hl
	ld (hl), 0D1h
	ret
StoChainAnsXXX:
	ld a, (ix-4)
	cp typeChainAns
	jr z, +_
	cp typeVariable
	jp nz, StoError
	ld hl, 0002FDDh
	ld d, 3
	mlt de
	ld a, e
	call _SetHLUToA
	jp InsertHL																; ld (ix+*), hl
_:	bit last_op_was_pointer, (iy+fExpression3)
	jp z, StoError
	call RemoveGetPointerAndCheckIfNumber
	jp z, UnknownError
	ld a, 022h
	jp InsertAHL															; ld (XXXXXX), hl
StoFunctionXXX:
	ld a, (ix-4)
	cp typeChainAns
	jr z, +_
	cp typeVariable
	jp nz, StoError
	call InsertHIXC															; ld hl, (ix+*)
	ld hl, 0002FDDh
	ld d, 3
	mlt de
	ld a, e
	call _SetHLUToA
	jp InsertHL																; ld (ix+*), hl
_:	bit last_op_was_pointer, (iy+fExpression3)
	jp z, StoError
	call RemoveGetPointerAndCheckIfNumber
	jr nz, +_
	ld a, OutputIsInDE
	ld (ExprOutput2), a
	ld a, c
	ld b, OutputInDE
	set need_push, (iy+fExpression1)
	call GetFunction
	ld a, 0EDh
	call InsertA															; ld (hl), de
	ld a, 01Fh
	jp InsertA																; ld (hl), de
_:	push hl
		ld a, c
		ld b, OutputInHL
		call GetFunction
	pop hl
	ld a, 022h
	jp InsertAHL															; ld (XXXXXX), hl
	
	scf
	sbc hl, hl
	ld (hl), 2
	bit last_op_was_pointer, (iy+fExpression3)
	ret
	
	
	
#IF 1==0
	A+2->{2}
		00 010000 Number
		03 XXXXXX ChainAns
		1F 04XXXX Operator
		Bit set
	A+2->{A}
		00 010000 Number
		03 XXXXXX ChainAns
		1F 04XXXX Operator
		Bit set
	A+2->{A+2}
#ENDIF

RemoveGetPointerAndCheckIfNumber:
	ld hl, (programPtr)
	dec hl
	ld a, (hl)
	cp 027h
	jr nz, +_
	dec hl
	ld a, (hl)
	cp 0EDh
	jr nz, +_
	ld (programPtr), hl
	ret																	; Returns Z
_:	ld hl, (programPtr)
	dec hl
	dec hl
	dec hl
	dec hl
	ld a, (hl)
	cp 02Ah
	jp nz, UnknownError
	ld (programPtr), hl
	inc hl
	ld hl, (hl)
	inc a
	ret																	; Returns NZ
functionSub:
	ld a, b
	cp typeNumber
	jp z, SubNumberXXX
	cp typeVariable
	jp z, SubVariableXXX
	cp typeFunction
	jp z, SubFunctionXXX
SubChainXXX:
	ld a, c
	cp ChainFirst
	jr z, SubChainFirstXXX
SubChainPushXXX:
	ld a, d
	cp typeNumber
	jr z, SubChainPushNumber
	cp typeVariable
	jr z, SubChainPushVariable
	cp typeFunction
	jr z, SubChainPushFunction
SubChainPushChain:
	ld hl, 090F147h
	jp InsertHL							; ld b, a \ pop af \ sub a, b
SubChainPushNumber:
	ld a, 0F1h
	call InsertA						; pop af
	jr SubChainFirstNumber
SubChainPushVariable:
	ld a, 0F1h
	call InsertA						; pop af
	jr SubChainFirstVariable
SubChainPushFunction:
	ld a, 0D1h
	call InsertA						; pop de
	ld a, e
	call GetFunction
	ld hl, 0907A57h
	jp InsertHL							; ld b, a \ ld a, d \ sub a, b
SubChainFirstXXX:
	ld a, d
	cp typeNumber
	jr z, SubChainFirstNumber
	cp typeFunction
	jr z, SubChainFirstFunction
SubChainFirstVariable:
	ld a, 0DDh
	call InsertA						; sub a, (ix+*) (1)
	ld a, 096h
	call InsertA						; sub a, (ix+*) (2)
	ld a, e
	jp InsertA							; sub a, (ix+*) (3)
SubChainFirstNumber:
	ld a, e
	or a
	ret z
	cp 1
	jr nz, +_
	ld a, 03Dh
	jp InsertA							; dec a
_:	cp 255
	jr nz, +_
	ld a, 03Ch
	jp InsertA							; inc a
_:	ld a, 0D6h
	call InsertA						; sub a, **
	ld a, e
	jp InsertA							; sub a, XX
SubChainFirstFunction:
	ld a, 057h
	call InsertA						; ld d, a
	ld a, e
	call GetFunction
	ld hl, 0907A57h
	jp InsertHL							; ld b, a \ ld a, d \ sub a, b
SubNumberXXX:
	ld a, d
	cp typeNumber
	jr z, SubNumberNumber
	cp typeVariable
	jr z, SubNumberVariable
	cp typeFunction
	jp z, SubNumberFunction
SubNumberChain:
	ld a, c
	or a
	jr nz, +_
	ld a, 0EDh
	call InsertA						; neg (1)
	ld a, 044h
	jp InsertA							; neg (2)
_:	cp 253
	jr nz, +_
	ld hl, 03D3D2Fh
	jp InsertHL							; cpl \ dec a \ dec a
_:	cp 254
	jr nz, +_
	ld a, 02Fh
	call InsertA						; cpl
	ld a, 03Dh
	jp InsertA							; dec a
_:	cp 255
	jr nz, +_
	ld a, 02Fh
	jp InsertA							; cpl
_:	ld a, 047h
	call InsertA						; ld b, a
	ld hl, 090003Eh
	ld h, c
	jp InsertHL							; ld a, XX \ sub a, b
SubNumberNumber:
	ld a, c
	sub a, e
	inc hl
	ld (hl), a
	res chain_operators, (iy+myFlags)
	ret
SubNumberVariable:
	ld a, c
	or a
	jr nz, +_
	call InsertAIXE						; ld a, (ix+*)
	ld a, 0EDh
	call InsertA						; neg (1)
	ld a, 44h
	jp InsertA							; neg (2)
_:	cp 253
	jr nz, +_
	call InsertAIXE						; ld a, (ix+*)
	ld hl, 3D3D2Fh
	jp InsertHL							; cpl \ dec a \ dec a
_:	cp 254
	jr nz, +_
	call InsertAIXE						; ld a, (ix+*)
	ld a, 2Fh
	call InsertA						; cpl
	ld a, 3Dh
	jp InsertA							; dec a
_:	cp 255
	jr nz, +_
	call InsertAIXE						; ld a, (ix+*)
	ld a, 2Fh
	jp InsertA							; cpl
_:	ld a, 03Eh
	call InsertA						; ld a, **
	ld a, c
	call InsertA						; ld a, XX
	jp SubChainFirstVariable
SubNumberFunction:
	ld a, e
	call GetFunction
	ld a, c
	or a
	jr nz, +_
	ld a, 0EDh
	call InsertA						; neg (1)
	ld a, 44h
	jp InsertA							; neg (2)
_:	cp 253
	jr nz, +_
	ld hl, 3D3D2Fh
	jp InsertHL							; cpl \ dec a \ dec a
_:	cp 254
	jr nz, +_
	ld a, 2Fh
	call InsertA						; cpl
	ld a, 3Dh
	jp InsertA							; dec a
_:	cp 255
	jr nz, +_
	ld a, 2Fh
	jp InsertA							; cpl
_:	ld a, 047h
	call InsertA						; ld b, a
	ld a, 03Eh
	call InsertA						; ld a, **
	ld a, c
	call InsertA						; ld a, XX
	ld a, 090h
	jp InsertA							; sub a, b
SubVariableXXX:
	ld a, d
	cp typeNumber
	jr z, SubVariableNumber
	cp typeVariable
	jr z, SubVariableVariable
	cp typeFunction
	jr z, SubVariableFunction
SubVariableChain:
	ld a, 047h
	call InsertA						; ld b, a
	call InsertAIXC						; ld a, (ix+*)
	ld a, 090h
	jp InsertA							; sub a, b
SubVariableNumber:
	call InsertAIXC						; ld a, (ix+*)
	jp SubChainFirstNumber
SubVariableVariable:
	ld a, c
	cp e
	jr nz, +_
	ld (hl), typeNumber
	inc hl
	ld (hl), 0
	res chain_operators, (iy+myFlags)
	ret
_:	call InsertAIXC						; ld a, (ix+*)
	jp SubChainFirstVariable
SubVariableFunction:
	ld a, e
	call GetFunction
	ld a, 047
	call InsertA						; ld b, a
	call InsertAIXC						; ld a, (ix+*)
	ld a, 090h
	jp InsertA							; sub a, b
SubFunctionXXX:
	ld a, d
	cp typeNumber
	jr z, SubFunctionNumber
	cp typeVariable
	jr z, SubFunctionVariable
	cp typeFunction
	jr z, SubFunctionFunction
SubFunctionChain:
	ld a, 047h
	call InsertA						; ld b, a
	ld a, c
	call GetFunction
	ld a, 090h
	jp InsertA							; sub a, b
SubFunctionNumber:
	ld a, c
	call GetFunction
	jp SubChainFirstNumber
SubFunctionVariable:
	ld a, c
	call GetFunction
	jp SubChainFirstVariable
SubFunctionFunction:
	ld a, e
	call GetFunction
	ld a, 047h
	call InsertA						; ld b, a
	ld a, c
	call GetFunction
	ld a, 090h
	jp InsertA							; sub a, b
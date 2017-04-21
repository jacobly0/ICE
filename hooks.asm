InstallHooks:
	ld	hl, ICEAppvar
	call	_Mov9ToOP1
	call	_ChkFindSym
	jr	c, ++_
	call	_ChkInRAM
	jr	nc, +_
	call	_Arc_Unarc
	ld	bc, 5
	add	hl, bc
_:	call	_DelVar
_:	ld	hl, Hooks_end - KeyHook_start
	call	_CreateAppVar
	inc	de
	inc	de
	ld	hl, KeyHook_start
	ld	bc, Hooks_end - KeyHook_start
	ldir
	call	_OP4ToOP1
	call	_Arc_Unarc
	call	_ChkFindSym
	ld	hl, 19														;	archived program header+VAT entry
	add	hl, de
	call	_SetGetKeyHook
	
	ld	de, KeyHook_end - KeyHook_start
	add	hl, de
	call	_SetTokenHook
	
	ld	de, TokenHook_end - TokenHook_start
	add	hl, de
	jp	_SetCursorhook
	
KeyHook_start:
	.db	83h
	or	a, a
	ret	z
	ld	b, a
	ld	a, (cxCurApp)
	cp	a, cxPrgmEdit
	ld	a, b
	ret	nz
	push	af
		call	_os_ClearStatusBarLow
		res	displayed_det, (iy+fAlways1)
	pop	af
	cp	a, kTrace
	ret	nz
DisplayCustomTokensAndCFunctions:
	call	_CursorOff
	ld	d, 0
DisplayTabWithTokens:
	push	de
		call	_ClrLCDFull
	pop	de
	ld	hl, 30
	ld	(penRow), hl
	ld	hl, 12
	ld	(penCol), hl
	ld	b, 0
	ld	a, d
	ld	e, 3
	mlt	de
	ld	hl, TabData - KeyHook_start
	add	hl, de
	ld	de, (rawKeyHookPtr)
	add	hl, de
	ld	hl, (hl)
	add	hl, de
	ld	d, a
	ld	e, 0
	jr	DisplayTokensLoop
KeyIsLeft:
	ld	a, d
	or	a, a
	jr	z, KeyLoop
	dec	d
	jr	DisplayTabWithTokens
KeyIsRight:
	ld	a, d
	cp	a, 5
	jr	z, KeyLoop
	inc	d
	jr	DisplayTabWithTokens
DisplayTokensLoop:
	ld	a, b
	cp	a, 16
	jr	z, StopDisplayingTokens
	inc	b
	call	_VPutS
	push	hl
		push	de
			ld	hl, (penRow)
			ld	de, 13
			add	hl, de
			ld	(penRow), hl
			ld	hl, 12
			ld	(penCol), hl
		pop	de
	pop	hl
	ld	a, (hl)
	or	a, a
	jr	nz, DisplayTokensLoop
StopDisplayingTokens:
	ld	hl, 1
	ld	(penCol), hl
GetRightCustomToken:
	ld	a, e
	ld	b, d
	ld	d, 13
	mlt	de
	ld	hl, 30
	add	hl, de
	ld	d, b
	ld	e, a
	ld	(penRow), hl
	ld	hl, 1
	ld	(penCol), hl
	push	hl
		push	de
			ld	a, '>'
			call	_VPutMap
		pop	de
	pop	hl
	ld	(penCol), hl
KeyLoop:
	call	_GetCSC
	or	a, a
	jr	z, KeyLoop
	cp	a, skLeft
	jr	z, KeyIsLeft
	cp	a, skRight
	jr	z, KeyIsRight
	cp	a, skUp
	jr	nz, KeyNotUp
	ld	a, e
	or	a, a
	jr	z, KeyLoop
	dec	e
EraseCursor:
	push	de
		ld	a, ' '
		call	_VPutMap
		ld	a, ' '
		call	_VPutMap
		ld	a, ' '
		call	_VPutMap
	pop	de
	jr	GetRightCustomToken
KeyNotUp:
	cp	a, skDown
	jr	nz, KeyNotDown
	ld	a, d
	cp	a, 5
	ld	a, e
	jr	nz, +_
	cp	a, (AMOUNT_OF_C_FUNCTIONS + AMOUNT_OF_CUSTOM_TOKENS)%16 - 1
	jr	z, KeyLoop
_:	ld	a, e
	cp	a, 16-1
	jr	z, KeyLoop
	inc	e
	jr	EraseCursor
KeyNotDown:
	cp	a, skClear
	jr	z, KeyIsClear
	cp	a, skEnter
	jr	nz, KeyLoop
	ld	a, e
	ld	e, 16
	mlt	de
	add	a, e
	sub	a, AMOUNT_OF_CUSTOM_TOKENS
	jr	c, InsertCustomToken
	ld	hl, saveSScreen
	ld	(hl), tDet
	inc	hl
	cp	a, 10
	jr	c, +_
	ld	d, a
	ld	e, 10
	xor	a, a
	ld	b, 8
_loop:
	sla	d
	rla
	cp	a, e
	jr	c, $+4
	sub	a, e
	inc	d
	djnz	_loop
	ld	e, a
	ld	a, d
	add	a, t0
	ld	(hl), a
	inc	hl
	ld	a, e
_:	add	a, t0
	ld	(hl), a
	inc	hl
	ld	(hl), 0
	ld	hl, saveSScreen
InsertCFunctionLoop:
	ld	a, (hl)
	or	a, a
	jr	z, BufferSearch
	ld	de, (editTail)
	ld	a, (de)
	cp	a, tEnter
	ld	d, 0
	ld	e, (hl)
	jr	z, +_
	push	hl
		call	_BufReplace
	pop	hl
	inc	hl
	jr	InsertCFunctionLoop
_:	push	hl
		call	_BufInsert
	pop	hl
	inc	hl
	jr	InsertCFunctionLoop
InsertCustomToken:
	add	a, 10+AMOUNT_OF_CUSTOM_TOKENS
	ld	e, a
	ld	d, tVarOut
	ld	hl, (editCursor)
	ld	a, (hl)
	cp	a, tEnter
	jr	z, +_
	call	_BufReplace
	jr	BufferSearch
_:	call	_BufInsert
KeyIsClear:
BufferSearch:
	ld	bc, 0
_:	call	_BufLeft
	jr	z, BufferFound
	ld	a, e
	cp	a, tEnter
	jr	z, +_
	inc	bc
	jr	-_
_:	call	_BufRight
BufferFound:
	push	bc
		call	_ClrLCDFull
		call	_ClrTxtShd
		ld	de, CustomTokensProgramText - KeyHook_start
		ld	hl, (rawKeyHookPtr)
		add	hl, de
		xor	a, a
		ld	(curCol), a
		ld	(curRow), a
		call	_PutS
		ld	hl, progToEdit
		ld	b, 8
_:		ld	a, (hl)
		or	a, a
		jr	z, +_
		call	_PutC
		inc	hl
		djnz	-_
_:		call	_NewLine
		ld	a, ':'
		call	_PutC
		call	_DispEOW
	pop	bc
MoveCursorOnce:
	ld	a, b
	or	a, c
	jr	z, ReturnToEditor
	call	_CursorRight
	dec	bc
	jr	MoveCursorOnce
ReturnToEditor:
	call	_CursorOn
	inc	a																	;	reset zero flag
	ld	a, 0
	ret
CustomTokensData:
Tab1:
C1:		.db	"ExecHex(", 0
C2:		.db	"DefineSprite(", 0
C3:		.db	"Call ", 0
C4:		.db	"CompilePrgm(", 0

#define AMOUNT_OF_C_FUNCTIONS 84

C6:		.db	"Begin", 0
C7:		.db	"End", 0
C8:		.db	"SetColor", 0
C9:		.db	"SetDefaultPalette", 0
C10:	.db	"SetPalette", 0
C11:	.db	"FillScreen", 0
C12:	.db	"SetPixel", 0
C13:	.db	"GetPixel", 0
C14:	.db	"GetDraw", 0
C15:	.db	"SetDraw", 0
C16:	.db	"SwapDraw", 0
C17:	.db	"Blit", 0
Tab2:
C18:	.db	"BlitLines", 0
C19:	.db	"BlitArea", 0
C20:	.db	"PrintChar", 0
C21:	.db	"PrintInt", 0
C22:	.db	"PrintUInt", 0
C23:	.db	"PrintString", 0
C24:	.db	"PrintStringXY", 0
C25:	.db	"SetTextXY", 0
C26:	.db	"SetTextBGColor", 0
C27:	.db	"SetTextFGColor", 0
C28:	.db	"SetTextTransparentColor", 0
C29:	.db	"SetCustomFontData", 0
C30:	.db	"SetCustomFontSpacing", 0
C31:	.db	"SetMonospaceFont", 0
C32:	.db	"GetStringWidth", 0
C33:	.db	"GetCharWidth", 0
Tab3:
C34:	.db	"GetTextX", 0
C35:	.db	"GetTextY", 0
C36:	.db	"Line", 0
C37:	.db	"HorizLine", 0
C38:	.db	"VertLine", 0
C39:	.db	"Circle", 0
C40:	.db	"FillCircle", 0
C41:	.db	"Rectangle", 0
C42:	.db	"FillRectangle", 0
C43:	.db	"Line_NoClip", 0
C44:	.db	"HorizLine_NoClip", 0
C45:	.db	"VertLine_NoClip", 0
C46:	.db	"FillCircle_NoClip", 0
C47:	.db	"Rectangle_NoClip", 0
C48:	.db	"FillRectangle_NoClip", 0
C49:	.db	"SetClipRegion", 0
Tab4:
C50:	.db	"GetClipRegion", 0
C51:	.db	"ShiftDown", 0
C52:	.db	"ShiftUp", 0
C53:	.db	"ShiftLeft", 0
C54:	.db	"ShiftRight", 0
C55:	.db	"Tilemap", 0
C56:	.db	"Tilemap_NoClip", 0
C57:	.db	"TransparentTilemap", 0
C58:	.db	"TransparentTilemap_NoClip", 0
C59:	.db	"TilePtr", 0
C60:	.db	"TilePtrMapped", 0
C61:	.db	"LZDecompress", 0
C62:	.db	"AllocSprite", 0
C63:	.db	"Sprite", 0
C64:	.db	"TransparentSprite", 0
C65:	.db	"Sprite_NoClip", 0
Tab5:
C66:	.db	"TransparentSprite_NoClip", 0
C67:	.db	"GetSprite_NoClip", 0
C68:	.db	"ScaledSprite_NoClip", 0
C69:	.db	"ScaledTransparentSprite_NoClip", 0
C70:	.db	"FlipSpriteY", 0
C71:	.db	"FlipSpriteX", 0
C72:	.db	"RotateSpriteC", 0
C73:	.db	"RotateSpriteCC", 0
C74:	.db	"RotateSpriteHalf", 0
C75:	.db	"Polygon", 0
C76:	.db	"Polygon_NoClip", 0
C77:	.db	"FillTriangle", 0
C78:	.db	"FillTriangle_NoClip", 0
C79:	.db	"LZDecompressSprite", 0
C80:	.db	"SetTextScale", 0
C81:	.db	"SetTransparentColor", 0
Tab6:
C82:	.db	"ZeroScreen", 0
C83:	.db	"SetTextConfig", 0
C84:	.db	"GetSpriteChar", 0
C85:	.db	"Lighten", 0
C86	.db	"Darken", 0
C87:	.db	"SetFontHeight", 0
C88:	.db	"ScaledSprite", 0
C89:	.db	"FloodFill", 0
	.db	0
TabData:
	.dl	Tab1 - KeyHook_start
	.dl	Tab2 - KeyHook_start
	.dl	Tab3 - KeyHook_start
	.dl	Tab4 - KeyHook_start
	.dl	Tab5 - KeyHook_start
	.dl	Tab6 - KeyHook_start
	
CData5:
	.dl	C6 - KeyHook_start
	.dl	C7 - KeyHook_start
	.dl	C8 - KeyHook_start
	.dl	C9 - KeyHook_start
	.dl	C10 - KeyHook_start
	.dl	C11 - KeyHook_start
	.dl	C12 - KeyHook_start
	.dl	C13 - KeyHook_start
	.dl	C14 - KeyHook_start
	.dl	C15 - KeyHook_start
	.dl	C16 - KeyHook_start
	.dl	C17 - KeyHook_start
	.dl	C18 - KeyHook_start
	.dl	C19 - KeyHook_start
	.dl	C20 - KeyHook_start
	.dl	C21 - KeyHook_start
	.dl	C22 - KeyHook_start
	.dl	C23 - KeyHook_start
	.dl	C24 - KeyHook_start
	.dl	C25 - KeyHook_start
	.dl	C26 - KeyHook_start
	.dl	C27 - KeyHook_start
	.dl	C28 - KeyHook_start
	.dl	C29 - KeyHook_start
	.dl	C30 - KeyHook_start
	.dl	C31 - KeyHook_start
	.dl	C32 - KeyHook_start
	.dl	C33 - KeyHook_start
	.dl	C34 - KeyHook_start
	.dl	C35 - KeyHook_start
	.dl	C36 - KeyHook_start
	.dl	C37 - KeyHook_start
	.dl	C38 - KeyHook_start
	.dl	C39 - KeyHook_start
	.dl	C40 - KeyHook_start
	.dl	C41 - KeyHook_start
	.dl	C42 - KeyHook_start
	.dl	C43 - KeyHook_start
	.dl	C44 - KeyHook_start
	.dl	C45 - KeyHook_start
	.dl	C46 - KeyHook_start
	.dl	C47 - KeyHook_start
	.dl	C48 - KeyHook_start
	.dl	C49 - KeyHook_start
	.dl	C50 - KeyHook_start
	.dl	C51 - KeyHook_start
	.dl	C52 - KeyHook_start
	.dl	C53 - KeyHook_start
	.dl	C54 - KeyHook_start
	.dl	C55 - KeyHook_start
	.dl	C56 - KeyHook_start
	.dl	C57 - KeyHook_start
	.dl	C58 - KeyHook_start
	.dl	C59 - KeyHook_start
	.dl	C60 - KeyHook_start
	.dl	C61 - KeyHook_start
	.dl	C62 - KeyHook_start
	.dl	C63 - KeyHook_start
	.dl	C64 - KeyHook_start
	.dl	C65 - KeyHook_start
	.dl	C66 - KeyHook_start
	.dl	C67 - KeyHook_start
	.dl	C68 - KeyHook_start
	.dl	C69 - KeyHook_start
	.dl	C70 - KeyHook_start
	.dl	C71 - KeyHook_start
	.dl	C72 - KeyHook_start
	.dl	C73 - KeyHook_start
	.dl	C74 - KeyHook_start
	.dl	C75 - KeyHook_start
	.dl	C76 - KeyHook_start
	.dl	C77 - KeyHook_start
	.dl	C78 - KeyHook_start
	.dl	C79 - KeyHook_start
	.dl	C80 - KeyHook_start
	.dl	C81 - KeyHook_start
	.dl	C82 - KeyHook_start
	.dl	C83 - KeyHook_start
	.dl	C84 - KeyHook_start
	.dl	C85 - KeyHook_start
	.dl	C86 - KeyHook_start
	.dl	C87 - KeyHook_start
	.dl	C88 - KeyHook_start
	.dl	C89 - KeyHook_start
	
#define	AMOUNT_OF_CUSTOM_TOKENS 4
Token1: .db 8,  "ExecHex(", 0
Token2: .db 13, "DefineSprite(", 0
Token3: .db 5,  "Call ", 0
Token4: .db 12, "CompilePrgm(", 0
CustomTokensProgramText:
	.db	"PROGRAM:", 0
KeyHook_end:

TokenHook_start:
	.db	83h
	ld	a, d
	cp	a, 4
	ret	nz
	ld	a, e
	cp	a, 5+3+(AMOUNT_OF_CUSTOM_TOKENS*3)
	ret	nc
	sub	a, 5
	ld	de, (rawKeyHookPtr)
	ld	hl, TokenHook_data - KeyHook_start
	add	hl, de
	ld	bc, 0
	ld	c, a
	add	hl, bc
	ld	hl, (hl)
	add	hl, de
	ret
TokenHook_data:
	.dl	Token1 - KeyHook_start - 1
	.dl	Token2 - KeyHook_start - 1
	.dl	Token3 - KeyHook_start - 1
	.dl	Token4 - KeyHook_start - 1
TokenHook_end:

CursorHook_start:
	.db	83h
	cp	a, 24h
	jr	nz, +_
	inc	a
	ld	a, (curUnder)
	ret
_:	cp	a, 22h
	ret	nz
	ld	a, (cxCurApp)
	cp	a, cxPrgmEdit
	ret	nz
	ld	hl, (editCursor)
	ld	a, (hl)
	cp	a, tDet
	ret	nz
DrawDetText:
	bit	displayed_det, (iy+fAlways1)
	ret	nz
	ld	hl, (editTail)
	inc	hl
	ld	a, (hl)
	sub	a, t0
	ret	c
	cp	a, t9-t0+1
	ld	bc, (editBtm)
	ld	de, 0
	ld	e, a
	jr	c, GetDetValueLoop
WrongDetValue:
	inc	a
	ret
GetDetValueLoop:
	inc	hl
	or	a, a
	sbc	hl, bc
	jr	z, GetDetValueStop
	add	hl, bc
	ld	a, (hl)
	sub	a, t0
	jr	c, GetDetValueStop
	cp	a, t9-t0+1
	jr	nc, GetDetValueStop
	push	hl
		ex	de, hl
		add	hl, hl
		push	hl
		pop	de
		add	hl, hl
		add	hl, hl
		add	hl, de
		ld	de, 0
		ld	e, a
		add	hl, de
		ex	de, hl
	pop	hl
	jr	GetDetValueLoop
GetDetValueStop:
	ex	de, hl
	ld	de, AMOUNT_OF_C_FUNCTIONS
	or	a, a
	sbc	hl, de
	jr	nc, WrongDetValue
	add	hl, de
	ld	h, 3
	mlt	hl
	ld	de, CData5 - KeyHook_start
	add	hl, de
	ld	de, (rawKeyHookPtr)
	add	hl, de
	ld	hl, (hl)
	add	hl, de
	ld	de, 000E71Ch
	ld.sis	(drawFGColor - 0D00000h), de
	ld.sis	de, (statusBarBGColor - 0D00000h)
	ld.sis	(drawBGColor - 0D00000h), de
	ld	a, 14
	ld	(penRow),a
	ld	de, 2
	ld.sis	(penCol - 0D00000h), de
	call	_VPutS
	ld	de, $FFFF
	ld.sis	(drawBGColor - 0D00000h), de
	set	displayed_det, (iy+fAlways1)
	ret
CursorHook_end:

Hooks_end:
#include "hooks/ti84pce.inc"

#define AMOUNT_OF_CUSTOM_TOKENS 8
#define AMOUNT_OF_GRAPHX_FUNCTIONS 89
#define AMOUNT_OF_FILEIOC_FUNCTIONS 21

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
	res	0, (iy-41h)
	pop	af
	cp	a, kTrace
	ret	nz
	ld	a, (progToEdit)
	or	a, a
	jr	z, DisplayCustomTokensAndCFunctions
; Stupid OS, we need to copy it to safe RAM, because the OS clears progToEdit when opening another OS context
	ld	de, saveSScreen
	ld	hl, progToEdit
	call	_Mov9b
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
	ld	hl, TabData
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
	cp	a, 7
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
	cp	a, 7
	ld	a, e
	jr	nz, +_
	cp	a, (AMOUNT_OF_CUSTOM_TOKENS + AMOUNT_OF_GRAPHX_FUNCTIONS + AMOUNT_OF_FILEIOC_FUNCTIONS)%16 - 1
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
; Check ignored C functions
	cp	a, 55 + AMOUNT_OF_FILEIOC_FUNCTIONS
	jr	c, +_
	inc	a
	inc	a
_:	cp	a, 74 + AMOUNT_OF_FILEIOC_FUNCTIONS
	jr	c, +_
	inc	a
_:	ld	hl, saveSScreen + 9
	cp	a, AMOUNT_OF_FILEIOC_FUNCTIONS
	jr	nc, +_
	ld	(hl), tSum
	jr	++_
_:	ld	(hl), tDet
	sub	a, AMOUNT_OF_FILEIOC_FUNCTIONS
_:	inc	hl
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
	ld	hl, saveSScreen + 9
	jr	InsertCFunctionLoop
InsertCFunctionLoop:
	ld	a, (hl)
	or	a, a
KeyIsClear:
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
	add	a, 10 + AMOUNT_OF_CUSTOM_TOKENS
	ld	e, a
	ld	d, tVarOut
	ld	hl, (editCursor)
	ld	a, (hl)
	cp	a, tEnter
	jr	z, +_
	call	_BufReplace
	jr	BufferSearch
_:	call	_BufInsert
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
	ld	de, CustomTokensProgramText
	ld	hl, (rawKeyHookPtr)
	add	hl, de
	xor	a, a
	ld	(curCol), a
	ld	(curRow), a
	call	_PutS
	ld	hl, saveSScreen
	ld	b, 8
_:	ld	a, (hl)
	or	a, a
	jr	z, +_
	call	_PutC
	inc	hl
	djnz	-_
_:	call	_NewLine
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
	inc	a			;    reset zero flag
	ld	a, 0
	ret
KeyHook_end:

.echo "Key hook: ",$-KeyHook_start, " bytes"

TokenHook_start:
	.db	83h
	ld	a, d
	cp	a, 4
	ret	nz
	ld	a, e
	cp	a, 3
	ret	c
	cp	a, 5 + (AMOUNT_OF_CUSTOM_TOKENS * 3)
	ret	nc
	sub	a, 5
	ld	de, (rawKeyHookPtr)
	ld	hl, TokenHook_data
	add	hl, de
	ld	bc, 0
	ld	c, a
	add	hl, bc
	ld	hl, (hl)
	add	hl, de
	ret
TokenHook_end:

.echo "Token hook: ",$-TokenHook_start, " bytes"

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
	cp	a, tSum
	jr	z, DrawDetText
	cp	a, tDet
	ret	nz
DrawDetText:
	bit	0, (iy-41h)
	ret	nz
	ld	b, a
	ld	hl, (editTail)
	inc	hl
	ld	a, (hl)
	sub	a, t0
	ret	c
	cp	a, t9-t0+1
	jr	c, GetDetValue
WrongDetValue:
	or	a, 1
	ret
GetDetValue:
	ld	iyl, b
	ld	bc, (editBtm)
	ld	de, 0
	ld	e, a
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
	ld	a, iyl
	ld	iy, flags
	cp	a, tDet
	jr	z, +_
	ld	de, AMOUNT_OF_FILEIOC_FUNCTIONS
	ld	bc, CData4
	jr	++_
_:	ld	de, AMOUNT_OF_GRAPHX_FUNCTIONS
	ld	bc, CData5
	ld	a, l
	cp	a, 55
	jr	z, WrongDetValue
	cp	a, 56
	jr	z, WrongDetValue
	cp	a, 73
	jr	z, WrongDetValue
_:	or	a, a
	sbc	hl, de
	jr	nc, WrongDetValue
	add	hl, de
	ld	h, 3
	mlt	hl
	add	hl, bc
	push	hl
	call	_os_ClearStatusBarLow
	pop	hl
	ld	de, (rawKeyHookPtr)
	add	hl, de
	ld	hl, (hl)
	add	hl, de
	ld	de, 000E71Ch
	ld.sis	(drawFGColor & 0FFFFh), de
	ld.sis	de, (statusBarBGColor & 0FFFFh)
	ld.sis	(drawBGColor & 0FFFFh), de
	ld	a, 14
	ld	(penRow),a
	ld	de, 2
	ld.sis	(penCol & 0FFFFh), de
	call	_VPutS
	ld	de, 0FFFFh
	ld.sis	(drawBGColor & 0FFFFh), de
	set	0, (iy-41h)
	inc	a
	ret
    
.echo "Cursor hook: ",$-CursorHook_start, " bytes"

DataStart:

Tab1:
C1:   .db "DefineSprite(W,H[,DATA])", 0
C2:   .db "Call LABEL", 0
C3:   .db "Data(SIZE,CONST...)", 0
C4:   .db "Copy(PTR_OUT,PTR_IN,SIZE)", 0
C5:   .db "Alloc(BYTES)", 0
C6:   .db "DefineTilemap()", 0
C7:   .db "CopyData(PTR_OUT,SIZE,CONST...)", 0
C8:   .db "LoadData(TILEMAP,OFFSET,SIZE)", 0

F01:  .db "CloseAll()", 0
F02:  .db "Open(NAME,MODE)", 0
F03:  .db "OpenVar(NAME,MODE,TYPE)", 0
F04:  .db "Close(SLOT)", 0
F05:  .db "Write(DATA,SIZE,COUNT,SLOT)", 0
F06:  .db "Read(DATA,SIZE,COUNT,SLOT)", 0
F07:  .db "GetChar(SLOT)", 0
F08:  .db "PutChar(CHAR,SLOT)", 0
Tab2:
F09:  .db "Delete(NAME)", 0
F10:  .db "DeleteVar(NAME,TYPE)", 0
F11:  .db "Seek(OFFSET,ORIGIN,SLOT)", 0
F12:  .db "Resize(SIZE,SLOT)", 0
F13:  .db "IsArchived(SLOT)", 0
F14:  .db "SetArchiveStatus(ARCHIVED,SLOT)", 0
F15:  .db "Tell(SLOT)", 0
F16:  .db "Rewind(SLOT)", 0
F17:  .db "GetSize(SLOT)", 0
F18:  .db "GetTokenString(", 014h, "PTR,", 014h, "L_TOK,", 014h, "L_STRING)", 0
F19:  .db "GetDataPtr(SLOT)", 0
F20:  .db "Detect(", 014h, "PTR,DATA)", 0
F21:  .db "DetectVar(", 014h, "PTR,DATA,TYPE)", 0
      
G01:  .db "Begin()", 0
G02:  .db "End()", 0
G03:  .db "SetColor(COLOR)", 0
Tab3:
G04:  .db "SetDefaultPalette()", 0
G05:  .db "SetPalette(PALETTE)", 0
G06:  .db "FillScreen(COLOR)", 0
G07:  .db "SetPixel(X,Y)", 0
G08:  .db "GetPixel(X,Y)", 0
G09:  .db "GetDraw()", 0
G10:  .db "SetDraw(LOC)", 0
G11:  .db "SwapDraw()", 0
G12:  .db "Blit(LOC)", 0
G13:  .db "BlitLines(LOC,Y,NUM)", 0
G14:  .db "BlitArea(LOC,X,Y,W,H)", 0
G15:  .db "PrintChar(CHAR)", 0
G16:  .db "PrintInt(N,CHARS)", 0
G17:  .db "PrintUInt(N,CHARS)", 0
G18:  .db "PrintString(STRING)", 0
G19:  .db "PrintStringXY(STRING,X,Y)", 0
Tab4:
G20:  .db "SetTextXY(X,Y)", 0
G21:  .db "SetTextBGColor(COLOR)", 0
G22:  .db "SetTextFGColor(COLOR)", 0
G23:  .db "SetTextTransparentColor(COLOR)", 0
G24:  .db "SetCustomFontData(DATA)", 0
G25:  .db "SetCustomFontSpacing(DATA)", 0
G26:  .db "SetMonospaceFont(SPACE)", 0
G27:  .db "GetStringWidth(STRING)", 0
G28:  .db "GetCharWidth(CHAR)", 0
G29:  .db "GetTextX()", 0
G30:  .db "GetTextY()", 0
G31:  .db "Line(X1,Y1,X2,Y2)", 0
G32:  .db "HorizLine(X,Y,LENGTH)", 0
G33:  .db "VertLine(X,Y,LENGTH)", 0
G34:  .db "Circle(X,Y,R)", 0
G35:  .db "FillCircle(X,Y,R)", 0
Tab5:
G36:  .db "Rectangle(X,Y,W,H)", 0
G37:  .db "FillRectangle(X,Y,W,H)", 0
G38:  .db "Line_NoClip(X1,Y1,X2,Y2)", 0
G39:  .db "HorizLine_NoClip(X,Y,LENGTH)", 0
G40:  .db "VertLine_NoClip(X,Y,LENGTH)", 0
G41:  .db "FillCircle_NoClip(X,Y,R)", 0
G42:  .db "Rectangle_NoClip(X,Y,W,H)", 0
G43:  .db "FillRectangle_NoClip(X,Y,W,H)", 0
G44:  .db "SetClipRegion(XMIN,YMIN,XMAX,YMAX)", 0
G45:  .db "GetClipRegion(PTR)", 0
G46:  .db "ShiftDown(PIXELS)", 0
G47:  .db "ShiftUp(PIXELS)", 0
G48:  .db "ShiftLeft(PIXELS)", 0
G49:  .db "ShiftRight(PIXELS)", 0
G50:  .db "Tilemap(PTR,X,Y)", 0
G51:  .db "Tilemap_NoClip(PTR,X,Y)", 0
Tab6:
G52:  .db "TransparentTilemap(PTR,X,Y)", 0
G53:  .db "TransparentTilemap_NoClip(PTR,X,Y)", 0
G54:  .db "TilePtr(PTR,X,Y)", 0
G55:  .db "TilePtrMapped(PTR,ROW,COL)", 0
G56:
G57:
G58:  .db "Sprite(PTR,X,Y)", 0
G59:  .db "TransparentSprite(PTR,X,Y)", 0
G60:  .db "Sprite_NoClip(PTR,X,Y)", 0
G61:  .db "TransparentSprite_NoClip(PTR,X,Y)", 0
G62:  .db "GetSprite_NoClip(PTR,X,Y)", 0
G63:  .db "ScaledSprite_NoClip(PTR,X,Y)", 0
G64:  .db "ScaledTransparentSprite_NoClip(PTR,X,Y)", 0
G65:  .db "FlipSpriteY(PTR_IN,PTR_OUT)", 0
G66:  .db "FlipSpriteX(PTR_IN,PTR_OUT)", 0
G67:  .db "RotateSpriteC(PTR_IN,PTR_OUT)", 0
G68:  .db "RotateSpriteCC(PTR_IN,PTR_OUT)", 0
G69:  .db "RotateSpriteHalf(PTR_IN,PTR_OUT)", 0
Tab7:
G70:  .db "Polygon(POINTS,NUM)", 0
G71:  .db "Polygon_NoClip(POINTS,NUM)", 0
G72:  .db "FillTriangle(X1,Y1,X2,Y2,X3,Y3)", 0
G73:  .db "FillTriangle_NoClip(X1,Y1,X2,Y2,X3,Y3)", 0
G74:
G75:  .db "SetTextScale(W_SCALE,H_SCALE)", 0
G76:  .db "SetTransparentColor(COLOR)", 0
G77:  .db "ZeroScreen()", 0
G78:  .db "SetTextConfig(CONFIG)", 0
G79:  .db "GetSpriteChar(CHAR)", 0
G80:  .db "Lighten(COLOR,AMOUNT)", 0
G81:  .db "Darken(COLOR,AMOUNT)", 0
G82:  .db "SetFontHeight(HEIGHT)", 0
G83:  .db "ScaledSprite(PTR_IN,PTR_OUT)", 0
G84:  .db "FloodFill(X,Y,COLOR)", 0
G85:  .db "RLETSprite(PTR,X,Y)", 0
G86:  .db "RLETSprite_NoClip(PTR,X,Y)", 0
Tab8:
G87:  .db "ConvertFromRLETSprite(PTR_IN,PTR_OUT)", 0
G88:  .db "ConvertToRLETSprite(PTR_IN,PTR_OUT)", 0
G89:  .db "ConvertToNewRLETSprite()", 0
G90:  .db "Rot.Sc.Spr.(PTR_IN,PTR_OUT,ANGLE,SCALE)", 0
G91:  .db "Rot.Sc.Tr.Spr._NC(PTR,X,Y,ANGLE,SCALE)", 0
G92:  .db "Rot.Sc.Spr._NC(PTR,X,Y,ANGLE,SCALE)", 0
      .db 0
      
; First token is $62 $09
; These magic bytes can be found with _GetKeyPress
Tok1: .db 090h, 13, "DefineSprite("
Tok2: .db 0EEh, 5,  "Call "
Tok3: .db 038h, 5,  "Data("
Tok4: .db 084h, 5,  "Copy("
Tok5: .db 087h, 6,  "Alloc("
Tok6: .db 042h, 14, "DefineTilemap("
Tok7: .db 043h, 9,  "CopyData("
Tok8: .db 0FFh, 9,  "LoadData("

TabData:
	.dl Tab1
	.dl Tab2
	.dl Tab3
	.dl Tab4
	.dl Tab5
	.dl Tab6
	.dl Tab7
	.dl Tab8
    
CData4:
	.dl F01
	.dl F02
	.dl F03
	.dl F04
	.dl F05
	.dl F06
	.dl F07
	.dl F08
	.dl F09
	.dl F10
	.dl F11
	.dl F12
	.dl F13
	.dl F14
	.dl F15
	.dl F16
	.dl F17
	.dl F18
	.dl F19
	.dl F20
	.dl F21
    
CData5:
	.dl G01
	.dl G02
	.dl G03
	.dl G04
	.dl G05
	.dl G06
	.dl G07
	.dl G08
	.dl G09
	.dl G10
	.dl G11
	.dl G12
	.dl G13
	.dl G14
	.dl G15
	.dl G16
	.dl G17
	.dl G18
	.dl G19
	.dl G20
	.dl G21
	.dl G22
	.dl G23
	.dl G24
	.dl G25
	.dl G26
	.dl G27
	.dl G28
	.dl G29
	.dl G30
	.dl G31
	.dl G32
	.dl G33
	.dl G34
	.dl G35
	.dl G36
	.dl G37
	.dl G38
	.dl G39
	.dl G40
	.dl G41
	.dl G42
	.dl G43
	.dl G44
	.dl G45
	.dl G46
	.dl G47
	.dl G48
	.dl G49
	.dl G50
	.dl G51
	.dl G52
	.dl G53
	.dl G54
	.dl G55
	.dl G56
	.dl G57
	.dl G58
	.dl G59
	.dl G60
	.dl G61
	.dl G62
	.dl G63
	.dl G64
	.dl G65
	.dl G66
	.dl G67
	.dl G68
	.dl G69
	.dl G70
	.dl G71
	.dl G72
	.dl G73
	.dl G74
	.dl G75
	.dl G76
	.dl G77
	.dl G78
	.dl G79
	.dl G80
	.dl G81
	.dl G82
	.dl G83
	.dl G84
	.dl G85
	.dl G86
	.dl G87
	.dl G88
	.dl G89
	.dl G90
	.dl G91
	.dl G92
    
TokenHook_data:
	.dl Tok1
	.dl Tok2
	.dl Tok3
	.dl Tok4
	.dl Tok5
	.dl Tok6
	.dl Tok7
	.dl Tok8

CustomTokensProgramText:
	.db "PROGRAM:", 0
    
.echo "Data size: ",$-DataStart, " bytes"
.echo "Total size: ", $-KeyHook_start, " bytes"

Hooks_end:
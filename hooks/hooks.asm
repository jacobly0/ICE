#include "hooks/ti84pce.inc"

#define AMOUNT_OF_CUSTOM_TOKENS 12
#define AMOUNT_OF_GRAPHX_FUNCTIONS 93
#define AMOUNT_OF_FILEIOC_FUNCTIONS 24

#define SQUARE_WIDTH 18
#define SQUARE_HEIGHT 13
#define SQUARES_START_POS 30

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
; Stupid OS, we need to copy it to safe RAM, because the OS clears progToEdit when opening another OS context
	ld	a, (progToEdit)
	or	a, a
	jr	z, +_
	ld	de, saveSScreen
	ld	hl, progToEdit
	call	_Mov9b
_:	pop	af
	cp	a, kTrace
	jr	z, +_
	cp	a, kGraph
	ret	nz
	ld	de, DisplayPalette
	ld	hl, (rawKeyHookPtr)
	add	hl, de
	jp	(hl)
_:	call	_CursorOff
	ld	d, 0
DisplayTabWithTokens:
	push	de
	call	_ClrLCDFull
	pop	de
	ld	a, 30
	ld	(penRow), a
	ld	hl, 12
	ld	(penCol), hl
	ld	b, h
	ld	a, d
	ld	e, 3
	mlt	de
	ld	hl, TabPointers
	add	hl, de
	ld	de, (rawKeyHookPtr)
	add	hl, de
	ld	hl, (hl)
	add	hl, de
	ld	d, a
	ld	e, b
	jr	DisplayTokensLoop
KeyIsLeft:
	ld	a, d
	or	a, a
	jr	z, KeyLoop
	dec	d
	jr	DisplayTabWithTokens
KeyIsRight:
	ld	a, d
	cp	a, 8
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
	ld	a, (penRow)
	add	a, 13
	ld	(penRow), a
	ld	hl, 12
	ld	(penCol), hl
	pop	hl
	ld	a, (hl)
	or	a, a
	jr	nz, DisplayTokensLoop
StopDisplayingTokens:
	ld	a, 1
	ld	(penCol), a
GetRightCustomToken:
; penRow = 30+e*13
	ld	a, e
	add	a, a
	add	a, a
	ld	b, a
	add	a, a
	add	a, b
	add	a, e
	add	a, 30
	ld	(penRow), a
	ld	a, 1
	ld	(penCol), a
	push	de
	ld	a, '>'
	call	_VPutMap
	pop	de
	ld	a, 1
	ld	(penCol), a
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
	cp	a, 8
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
	ld	hl, saveSScreen + 9
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
	xor	a, a
	ld	(curCol), a
	ld	(curRow), a
	ld	a, (winTop)
	or	a, a
	jr	z, DisplayProgramText		; This is apparently a Cesium feature
	ld	de, ProgramText
	ld	hl, (rawKeyHookPtr)
	add	hl, de
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
DisplayProgramText:
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
	
DisplayPalette:
	call	_RunIndicOff
	call	_boot_ClearVRAM
; Display the header strings
	ld	hl, 120
	ld	(penCol), hl
	ld	a, 1
	ld	(penRow), a
	ld	de, (rawKeyHookPtr)
	ld	hl, ColorText
	add	hl, de
	call	_VPutS
	ld	hl, SQUARES_START_POS + 6
	ld	(penCol), hl
	ld	a, 14
	ld	(penRow), a
	push	de
	ld	hl, ColumnHeaderText
	add	hl, de
	call	_VPutS
	ld	hl, vRAM + ((SQUARES_START_POS * lcdWidth + SQUARES_START_POS) * 2)
	ld	b, 0
; Display all the squares
DisplaySquaresRows:
; Display the entire square
	ld	c, SQUARE_HEIGHT
DisplaySquareRowLoop:
; Display one row of a square
	ld	a, SQUARE_WIDTH
	ld	de, (lcdWidth - SQUARE_WIDTH) * 2
DisplaySquareRowInnerLoop:
	ld	(hl), b
	inc	hl
	ld	(hl), b
	inc	hl
	dec	a
	jr	nz, DisplaySquareRowInnerLoop
	add	hl, de
	dec	c
	jr	nz, DisplaySquareRowLoop
	inc	b
	ld	a, b
	and	a, 000001111b
	jr	nz, DontAddNewRow
; We need to switch to the next row = update pointer + display index in front of row
; (Index / 16) * 13 + 30 = Y pos
	ld	a, b
	sub	a, 16
	push	hl
	ld	e, a
	push	bc
	or	a, a
	rra
	rra
	rra
	rra
	ld	b, a
	ld	c, SQUARE_HEIGHT
	mlt	bc
	ld	a, SQUARES_START_POS
	add	a, c
	ld	(penRow), a
; Do some magic stuff to get X pos
	ld	a, e
	ld	de, 8
	ld	hl, 4
	cp	a, 100
	jr	nc, +_
	add	hl, de
	cp	a, 10
	jr	nc, +_
	dec	de
	dec	de
	add	hl, de
_:	ld	(penCol), hl
	ld	e, a
	ex	de, hl
	ld	b, 3
	call	_VDispHL
	pop	bc
	pop	hl
; Set pointer to new row
	ld	de, (-SQUARE_WIDTH * 15) * 2
	jr	$+6
DontAddNewRow:
; Add square to pointer
	ld	de, (-SQUARE_HEIGHT * lcdWidth + SQUARE_WIDTH) * 2
	add	hl, de
	ld	a, b
	or	a, a
	jr	nz, DisplaySquaresRows
; Done, wait and return
_:	call	_GetCSC
	or	a, a
	jr	z, -_
	call	_DrawStatusBar
	pop	de
	ld	hl, BufferSearch
	add	hl, de
	jp	(hl)
KeyHook_end:

.echo $-KeyHook_start

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
	sbc	hl, hl
	ld	l, a
	ld	de, CustomTokensPointers
	add	hl, de
	ld	de, (rawKeyHookPtr)
	add	hl, de
	ld	hl, (hl)
	add	hl, de
	ret
TokenHook_end:

.echo $-TokenHook_start

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
	ld	bc, FileiocFunctionsPointers
	jr	++_
_:	ld	de, AMOUNT_OF_GRAPHX_FUNCTIONS
	ld	bc, GraphxFunctionsPointers
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

Tab1:
C1:	.db "DefineSprite(W,H[,PTR])", 0
C2:	.db "Call LABEL", 0
C3:	.db "Data(SIZE,CONST...)", 0
C4:	.db "Copy(PTR_OUT,PTR_IN,SIZE)", 0
C5:	.db "Alloc(BYTES)", 0
C6:	.db "DefineTilemap()", 0
C7:	.db "CopyData(PTR_OUT,SIZE,CONST...)", 0
C8:	.db "LoadData(TILEMAP,OFFSET,SIZE)", 0
C9:	.db "SetBrightness(LEVEL)", 0
C10:	.db "SetByte(VAR1[,VAR2...])", 0
C11:	.db "SetInt(VAR1[,VAR2...])", 0
C12:	.db "SetFloat(VAR1[,VAR2...])", 0

F01:	.db "CloseAll()", 0
F02:	.db "Open(NAME,MODE)", 0
F03:	.db "OpenVar(NAME,MODE,TYPE)", 0
F04:	.db "Close(SLOT)", 0
Tab2:
F05:	.db "Write(DATA,SIZE,COUNT,SLOT)", 0
F06:	.db "Read(PTR,SIZE,COUNT,SLOT)", 0
F07:	.db "GetChar(SLOT)", 0
F08:	.db "PutChar(CHAR,SLOT)", 0
F09:	.db "Delete(NAME)", 0
F10:	.db "DeleteVar(NAME,TYPE)", 0
F11:	.db "Seek(OFFSET,ORIGIN,SLOT)", 0
F12:	.db "Resize(SIZE,SLOT)", 0
F13:	.db "IsArchived(SLOT)", 0
F14:	.db "SetArchiveStatus(ARCHIVED,SLOT)", 0
F15:	.db "Tell(SLOT)", 0
F16:	.db "Rewind(SLOT)", 0
F17:	.db "GetSize(SLOT)", 0
F18:	.db "GetTokenString(", 014h, "PTR,", 014h, "L_TOK,", 014h, "L_STRING)", 0
F19:	.db "GetDataPtr(SLOT)", 0
F20:	.db "Detect(", 014h, "PTR,DATA)", 0
Tab3:
F21:	.db "DetectVar(", 014h, "PTR,DATA,TYPE)", 0
F22:	.db "SetVar(TYPE,NAME,DATA)", 0
F23:	.db "StoVar(TYPE_O,PTR_O,TYPE_I,PTR_I)", 0
F24:	.db "RclVar(TYPE,NAME,PTR)", 0

G01:	.db "Begin()", 0
G02:	.db "End()", 0
G03:	.db "SetColor(COLOR)", 0
G04:	.db "SetDefaultPalette()", 0
G05:	.db "SetPalette(PALETTE)", 0
G06:	.db "FillScreen(COLOR)", 0
G07:	.db "SetPixel(X,Y)", 0
G08:	.db "GetPixel(X,Y)", 0
G09:	.db "GetDraw()", 0
G10:	.db "SetDraw(LOC)", 0
G11:	.db "SwapDraw()", 0
G12:	.db "Blit(LOC)", 0
Tab4:
G13:	.db "BlitLines(LOC,Y,NUM)", 0
G14:	.db "BlitArea(LOC,X,Y,W,H)", 0
G15:	.db "PrintChar(CHAR)", 0
G16:	.db "PrintInt(N,CHARS)", 0
G17:	.db "PrintUInt(N,CHARS)", 0
G18:	.db "PrintString(STRING)", 0
G19:	.db "PrintStringXY(STRING,X,Y)", 0
G20:	.db "SetTextXY(X,Y)", 0
G21:	.db "SetTextBGColor(COLOR)", 0
G22:	.db "SetTextFGColor(COLOR)", 0
G23:	.db "SetTextTransparentColor(COLOR)", 0
G24:	.db "SetCustomFontData(DATA)", 0
G25:	.db "SetCustomFontSpacing(DATA)", 0
G26:	.db "SetMonospaceFont(SPACE)", 0
G27:	.db "GetStringWidth(STRING)", 0
G28:	.db "GetCharWidth(CHAR)", 0
Tab5:
G29:	.db "GetTextX()", 0
G30:	.db "GetTextY()", 0
G31:	.db "Line(X1,Y1,X2,Y2)", 0
G32:	.db "HorizLine(X,Y,LENGTH)", 0
G33:	.db "VertLine(X,Y,LENGTH)", 0
G34:	.db "Circle(X,Y,R)", 0
G35:	.db "FillCircle(X,Y,R)", 0
G36:	.db "Rectangle(X,Y,W,H)", 0
G37:	.db "FillRectangle(X,Y,W,H)", 0
G38:	.db "Line_NoClip(X1,Y1,X2,Y2)", 0
G39:	.db "HorizLine_NoClip(X,Y,LENGTH)", 0
G40:	.db "VertLine_NoClip(X,Y,LENGTH)", 0
G41:	.db "FillCircle_NoClip(X,Y,R)", 0
G42:	.db "Rectangle_NoClip(X,Y,W,H)", 0
G43:	.db "FillRectangle_NoClip(X,Y,W,H)", 0
G44:	.db "SetClipRegion(XMIN,YMIN,XMAX,YMAX)", 0
Tab6:
G45:	.db "GetClipRegion(PTR)", 0
G46:	.db "ShiftDown(PIXELS)", 0
G47:	.db "ShiftUp(PIXELS)", 0
G48:	.db "ShiftLeft(PIXELS)", 0
G49:	.db "ShiftRight(PIXELS)", 0
G50:	.db "Tilemap(PTR,X,Y)", 0
G51:	.db "Tilemap_NoClip(PTR,X,Y)", 0
G52:	.db "TransparentTilemap(PTR,X,Y)", 0
G53:	.db "TransparentTilemap_NoClip(PTR,X,Y)", 0
G54:	.db "TilePtr(PTR,X,Y)", 0
G55:	.db "TilePtrMapped(PTR,ROW,COL)", 0
G56:	.db "NOT USED", 0
G57:	.db "NOT USED", 0
G58:	.db "Sprite(PTR,X,Y)", 0
G59:	.db "TransparentSprite(PTR,X,Y)", 0
G60:	.db "Sprite_NoClip(PTR,X,Y)", 0
Tab7:
G61:	.db "TransparentSprite_NoClip(PTR,X,Y)", 0
G62:	.db "GetSprite_NoClip(PTR,X,Y)", 0
G63:	.db "ScaledSprite_NoClip(PTR,X,Y)", 0
G64:	.db "ScaledTransparentSprite_NoClip(PTR,X,Y)", 0
G65:	.db "FlipSpriteY(PTR_IN,PTR_OUT)", 0
G66:	.db "FlipSpriteX(PTR_IN,PTR_OUT)", 0
G67:	.db "RotateSpriteC(PTR_IN,PTR_OUT)", 0
G68:	.db "RotateSpriteCC(PTR_IN,PTR_OUT)", 0
G69:	.db "RotateSpriteHalf(PTR_IN,PTR_OUT)", 0
G70:	.db "Polygon(POINTS,NUM)", 0
G71:	.db "Polygon_NoClip(POINTS,NUM)", 0
G72:	.db "FillTriangle(X1,Y1,X2,Y2,X3,Y3)", 0
G73:	.db "FillTriangle_NoClip(X1,Y1,X2,Y2,X3,Y3)", 0
G74:	.db "NOT USED", 0
G75:	.db "SetTextScale(W_SCALE,H_SCALE)", 0
G76:	.db "SetTransparentColor(COLOR)", 0
Tab8:
G77:	.db "ZeroScreen()", 0
G78:	.db "SetTextConfig(CONFIG)", 0
G79:	.db "GetSpriteChar(CHAR)", 0
G80:	.db "Lighten(COLOR,AMOUNT)", 0
G81:	.db "Darken(COLOR,AMOUNT)", 0
G82:	.db "SetFontHeight(HEIGHT)", 0
G83:	.db "ScaledSprite(PTR_IN,PTR_OUT)", 0
G84:	.db "FloodFill(X,Y,COLOR)", 0
G85:	.db "RLETSprite(PTR,X,Y)", 0
G86:	.db "RLETSprite_NoClip(PTR,X,Y)", 0
G87:	.db "ConvertFromRLETSprite(PTR_IN,PTR_OUT)", 0
G88:	.db "ConvertToRLETSprite(PTR_IN,PTR_OUT)", 0
G89:	.db "ConvertToNewRLETSprite()", 0
G90:	.db "Rot.Sc.Spr.(PTR_IN,PTR_OUT,ANGLE,SCALE)", 0
G91:	.db "Rot.Sc.Tr.Spr._NC(PTR,X,Y,ANGLE,SCALE)", 0
G92:	.db "Rot.Sc.Spr._NC(PTR,X,Y,ANGLE,SCALE)", 0
Tab9:
G93:	.db "SetCharData(INDEX,DATA)", 0
	.db 0
      
; These magic bytes can be found with _GetKeyPress (D is 2-byte token, E is token, A is output)
Tok1:	.db 090h, 13, "DefineSprite("	; 62 0A
Tok2:	.db 0EEh, 5,  "Call "		; 62 0B
Tok3:	.db 038h, 5,  "Data("		; 62 0C
Tok4:	.db 084h, 5,  "Copy("		; 62 0D
Tok5:	.db 087h, 6,  "Alloc("		; 62 0E
Tok6:	.db 042h, 14, "DefineTilemap("	; 62 0F
Tok7:	.db 043h, 9,  "CopyData("	; 62 10
Tok8:	.db 0FFh, 9,  "LoadData("	; 62 11
Tok9:	.db 0BAh, 14, "SetBrightness("	; 62 12
Tok10:	.db 0C0h, 8,  "SetByte("	; 62 13
Tok11:	.db 0BFh, 7,  "SetInt("		; 62 14
Tok12:	.db 0C1h, 9,  "SetFloat("	; 62 15

TabPointers:
	.dl Tab1, Tab2, Tab3, Tab4, Tab5, Tab6, Tab7, Tab8
	.dl Tab9
    
FileiocFunctionsPointers:
	.dl F01, F02, F03, F04, F05, F06, F07, F08
	.dl F09, F10, F11, F12, F13, F14, F15, F16
	.dl F17, F18, F19, F20, F21, F22, F23, F24
    
GraphxFunctionsPointers:
	.dl G01, G02, G03, G04, G05, G06, G07, G08
	.dl G09, G10, G11, G12, G13, G14, G15, G16
	.dl G17, G18, G19, G20, G21, G22, G23, G24
	.dl G25, G26, G27, G28, G29, G30, G31, G32
	.dl G33, G34, G35, G36, G37, G38, G39, G40
	.dl G41, G42, G43, G44, G45, G46, G47, G48
	.dl G49, G50, G51, G52, G53, G54, G55, G56
	.dl G57, G58, G59, G60, G61, G62, G63, G64
	.dl G65, G66, G67, G68, G69, G70, G71, G72
	.dl G73, G74, G75, G76, G77, G78, G79, G80
	.dl G81, G82, G83, G84, G85, G86, G87, G88
	.dl G89, G90, G91, G92, G93
    
CustomTokensPointers:
	.dl Tok1, Tok2, Tok3, Tok4, Tok5, Tok6, Tok7, Tok8
	.dl Tok9, Tok10, Tok11, Tok12

ProgramText:
	.db "PROGRAM:", 0
ColorText:
	.db "COLOR=COL+ROW", 0
ColumnHeaderText:
	.db "0", 0EFh
	.db "1", 0EFh
	.db "2", 0EFh
	.db "3", 0EFh
	.db "4", 0EFh
	.db "5", 0EFh
	.db "6", 0EFh
	.db "7", 0EFh
	.db "8", 0EFh
	.db "9", 0EEh
	.db "10", " "
	.db "11", " "
	.db "12", " "
	.db "13", " "
	.db "14", " "
	.db "15", 0

Hooks_end:
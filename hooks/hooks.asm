#include "hooks/ti84pce.inc"

#define AMOUNT_OF_CUSTOM_TOKENS 2
#define AMOUNT_OF_GRAPHX_FUNCTIONS 92
#define AMOUNT_OF_FILEIOC_FUNCTIONS 21

KeyHook_start:
    .db    83h
    or      a, a
    ret     z
    ld      b, a
    ld      a, (cxCurApp)
    cp      a, cxPrgmEdit
    ld      a, b
    ret     nz
    push    af
            call    _os_ClearStatusBarLow
            res     0, (iy-41h)
    pop     af
    cp      a, kTrace
    ret     nz
DisplayCustomTokensAndCFunctions:
    call    _CursorOff
    ld      d, 0
DisplayTabWithTokens:
    push    de
            call    _ClrLCDFull
    pop     de
    ld      hl, 30
    ld      (penRow), hl
    ld      hl, 12
    ld      (penCol), hl
    ld      b, 0
    ld      a, d
    ld      e, 3
    mlt     de
    ld      hl, TabData - KeyHook_start
    add     hl, de
    ld      de, (rawKeyHookPtr)
    add     hl, de
    ld      hl, (hl)
    add     hl, de
    ld      d, a
    ld      e, 0
    jr      DisplayTokensLoop
KeyIsLeft:
    ld      a, d
    or      a, a
    jr      z, KeyLoop
    dec     d
    jr      DisplayTabWithTokens
KeyIsRight:
    ld      a, d
    cp      a, 6
    jr      z, KeyLoop
    inc     d
    jr      DisplayTabWithTokens
DisplayTokensLoop:
    ld      a, b
    cp      a, 16
    jr      z, StopDisplayingTokens
    inc     b
    call    _VPutS
    push    hl
            push    de
                    ld    hl, (penRow)
                    ld    de, 13
                    add    hl, de
                    ld    (penRow), hl
                    ld    hl, 12
                    ld    (penCol), hl
            pop     de
    pop     hl
    ld      a, (hl)
    or      a, a
    jr      nz, DisplayTokensLoop
StopDisplayingTokens:
    ld      hl, 1
    ld      (penCol), hl
GetRightCustomToken:
    ld      a, e
    ld      b, d
    ld      d, 13
    mlt     de
    ld      hl, 30
    add     hl, de
    ld      d, b
    ld      e, a
    ld      (penRow), hl
    ld      hl, 1
    ld      (penCol), hl
    push    hl
            push    de
                    ld      a, '>'
                    call    _VPutMap
            pop     de
    pop     hl
    ld      (penCol), hl
KeyLoop:
    call    _GetCSC
    or      a, a
    jr      z, KeyLoop
    cp      a, skLeft
    jr      z, KeyIsLeft
    cp      a, skRight
    jr      z, KeyIsRight
    cp      a, skUp
    jr      nz, KeyNotUp
    ld      a, e
    or      a, a
    jr      z, KeyLoop
    dec     e
EraseCursor:
    push    de
            ld      a, ' '
            call    _VPutMap
            ld      a, ' '
            call    _VPutMap
            ld      a, ' '
            call    _VPutMap
    pop     de
    jr      GetRightCustomToken
KeyNotUp:
    cp      a, skDown
    jr      nz, KeyNotDown
    ld      a, d
    cp      a, 7
    ld      a, e
    jr      nz, +_
    cp      a, (AMOUNT_OF_GRAPHX_FUNCTIONS + AMOUNT_OF_CUSTOM_TOKENS)%16 - 1
    jr      z, KeyLoop
_:  ld      a, e
    cp      a, 16-1
    jr      z, KeyLoop
    inc     e
    jr      EraseCursor
KeyNotDown:
    cp      a, skClear
    jr      z, KeyIsClear
    cp      a, skEnter
    jr      nz, KeyLoop
    ld      a, e
    ld      e, 16
    mlt     de
    add     a, e
    sub     a, AMOUNT_OF_CUSTOM_TOKENS
    jr      c, InsertCustomToken
    ld      hl, saveSScreen
    cp      a, 21
    jr      nc, +_
    ld      (hl), tSum
    jr      ++_
_:  ld      (hl), tDet
    sub     a, 21
_:  inc     hl
    cp      a, 10
    jr      c, +_
    ld      d, a
    ld      e, 10
    xor     a, a
    ld      b, 8
_loop:
    sla     d
    rla
    cp      a, e
    jr      c, $+4
    sub     a, e
    inc     d
    djnz    _loop
    ld      e, a
    ld      a, d
    add     a, t0
    ld      (hl), a
    inc     hl
    ld      a, e
_:  add     a, t0
    ld      (hl), a
    inc     hl
    ld      (hl), 0
    ld      hl, saveSScreen
InsertCFunctionLoop:
    ld      a, (hl)
    or      a, a
    jr      z, BufferSearch
    ld      de, (editTail)
    ld      a, (de)
    cp      a, tEnter
    ld      d, 0
    ld      e, (hl)
    jr      z, +_
    push    hl
            call    _BufReplace
    pop     hl
    inc     hl
    jr      InsertCFunctionLoop
_:  push    hl
            call    _BufInsert
    pop     hl
    inc     hl
    jr      InsertCFunctionLoop
InsertCustomToken:
    add     a, 10+AMOUNT_OF_CUSTOM_TOKENS
    ld      e, a
    ld      d, tVarOut
    ld      hl, (editCursor)
    ld      a, (hl)
    cp      a, tEnter
    jr      z, +_
    call    _BufReplace
    jr      BufferSearch
_:  call    _BufInsert
KeyIsClear:
BufferSearch:
    ld      bc, 0
_:  call    _BufLeft
    jr      z, BufferFound
    ld      a, e
    cp      a, tEnter
    jr      z, +_
    inc     bc
    jr      -_
_:  call    _BufRight
BufferFound:
    push    bc
            call    _ClrLCDFull
            call    _ClrTxtShd
            ld      de, CustomTokensProgramText - KeyHook_start
            ld      hl, (rawKeyHookPtr)
            add     hl, de
            xor     a, a
            ld      (curCol), a
            ld      (curRow), a
            call    _PutS
            ld      hl, progToEdit
            ld      b, 8
_:          ld      a, (hl)
            or      a, a
            jr      z, +_
            call    _PutC
            inc     hl
            djnz    -_
_:          call    _NewLine
            ld      a, ':'
            call    _PutC
            call    _DispEOW
            pop     bc
MoveCursorOnce:
    ld      a, b
    or      a, c
    jr      z, ReturnToEditor
    call    _CursorRight
    dec     bc
    jr      MoveCursorOnce
ReturnToEditor:
    call    _CursorOn
    inc     a                                                                    ;    reset zero flag
    ld      a, 0
    ret
KeyHook_end:

.echo "Key hook: ",$-KeyHook_start, " bytes"

TokenHook_start:
    .db     83h
    ld      a, d
    cp      a, 4
    ret     nz
    ld      a, e
    cp      a, 5+3+(AMOUNT_OF_CUSTOM_TOKENS*3)
    ret     nc
    sub     a, 5
    ld      de, (rawKeyHookPtr)
    ld      hl, TokenHook_data - KeyHook_start
    add     hl, de
    ld      bc, 0
    ld      c, a
    add     hl, bc
    ld      hl, (hl)
    add     hl, de
    ret
TokenHook_end:

.echo "Token hook: ",$-TokenHook_start, " bytes"

CursorHook_start:
    .db     83h
    cp      a, 24h
    jr      nz, +_
    inc     a
    ld      a, (curUnder)
    ret
_:  cp      a, 22h
    ret     nz
    ld      a, (cxCurApp)
    cp      a, cxPrgmEdit
    ret     nz
    ld      hl, (editCursor)
    ld      a, (hl)
    cp      a, tSum
    jr      z, DrawDetText
    cp      a, tDet
    ret     nz
DrawDetText:
    bit     0, (iy-41h)
    ret     nz
    ld      iyl, a
    ld      hl, (editTail)
    inc     hl
    ld      a, (hl)
    sub     a, t0
    ret     c
    cp      a, t9-t0+1
    ld      bc, (editBtm)
    ld      de, 0
    ld      e, a
    jr      c, GetDetValueLoop
WrongDetValue:
    or      a, 1
    ret
GetDetValueLoop:
    inc     hl
    or      a, a
    sbc     hl, bc
    jr      z, GetDetValueStop
    add     hl, bc
    ld      a, (hl)
    sub     a, t0
    jr      c, GetDetValueStop
    cp      a, t9-t0+1
    jr      nc, GetDetValueStop
    push    hl
            ex      de, hl
            add     hl, hl
            push    hl
            pop     de
            add     hl, hl
            add     hl, hl
            add     hl, de
            ld      de, 0
            ld      e, a
            add     hl, de
            ex      de, hl
    pop     hl
    jr     GetDetValueLoop
GetDetValueStop:
    ex      de, hl
    ld      a, iyl
    ld      iy, flags
    cp      a, tDet
    jr      z, +_
    ld      de, AMOUNT_OF_FILEIOC_FUNCTIONS
    ld      bc, CData4
    jr      ++_
_:  ld      de, AMOUNT_OF_GRAPHX_FUNCTIONS
    ld      bc, CData5
_:  or      a, a
    sbc     hl, de
    jr      nc, WrongDetValue
    add     hl, de
    ld      h, 3
    mlt     hl
    add     hl, bc
    ld      de, (rawKeyHookPtr)
    add     hl, de
    ld      hl, (hl)
    add     hl, de
    ld      de, 000E71Ch
    ld.sis  (drawFGColor & 0FFFFh), de
    ld.sis  de, (statusBarBGColor & 0FFFFh)
    ld.sis  (drawBGColor & 0FFFFh), de
    ld      a, 14
    ld      (penRow),a
    ld      de, 2
    ld.sis  (penCol & 0FFFFh), de
    call    _VPutS
    ld      de, 0FFFFh
    ld.sis  (drawBGColor & 0FFFFh), de
    set     0, (iy-41h)
    ret
    
.echo "Cursor hook: ",$-CursorHook_start, " bytes"

DataStart:

Tab1:
C1:   .db "DefineSprite(", 0
C2:   .db "Call ", 0

F01:  .db "CloseAll", 0
F02:  .db "Open(", 0
F03:  .db "OpenVar(", 0
F04:  .db "Close(", 0
F05:  .db "Write(", 0
F06:  .db "Read(", 0
F07:  .db "GetChar(", 0
F08:  .db "PutChar(", 0
F09:  .db "Delete(", 0
F10:  .db "DeleteVar(", 0
F11:  .db "Seek(", 0
F12:  .db "Resize(", 0
F13:  .db "IsArchived(", 0
F14:  .db "SetArchiveStatus(", 0
Tab2:
F15:  .db "Tell(", 0
F16:  .db "Rewind(", 0
F17:  .db "GetSize(", 0
F18:  .db "GetTokenString(", 0
F19:  .db "GetDataPtr(", 0
F20:  .db "Detect(", 0
F21:  .db "DetectVar(", 0
      
G01:  .db "Begin", 0
G02:  .db "End", 0
G03:  .db "SetColor", 0
G04:  .db "SetDefaultPalette", 0
G05:  .db "SetPalette", 0
G06:  .db "FillScreen", 0
G07:  .db "SetPixel", 0
G08:  .db "GetPixel", 0
G09:  .db "GetDraw", 0
Tab3:
G10:  .db "SetDraw", 0
G11:  .db "SwapDraw", 0
G12:  .db "Blit", 0
G13:  .db "BlitLines", 0
G14:  .db "BlitArea", 0
G15:  .db "PrintChar", 0
G16:  .db "PrintInt", 0
G17:  .db "PrintUInt", 0
G18:  .db "PrintString", 0
G19:  .db "PrintStringXY", 0
G20:  .db "SetTextXY", 0
G21:  .db "SetTextBGColor", 0
G22:  .db "SetTextFGColor", 0
G23:  .db "SetTextTransparentColor", 0
G24:  .db "SetCustomFontData", 0
G25:  .db "SetCustomFontSpacing", 0
Tab4:
G26:  .db "SetMonospaceFont", 0
G27:  .db "GetStringWidth", 0
G28:  .db "GetCharWidth", 0
G29:  .db "GetTextX", 0
G30:  .db "GetTextY", 0
G31:  .db "Line", 0
G32:  .db "HorizLine", 0
G33:  .db "VertLine", 0
G34:  .db "Circle", 0
G35:  .db "FillCircle", 0
G36:  .db "Rectangle", 0
G37:  .db "FillRectangle", 0
G38:  .db "Line_NoClip", 0
G39:  .db "HorizLine_NoClip", 0
G40:  .db "VertLine_NoClip", 0
G41:  .db "FillCircle_NoClip", 0
Tab5:
G42:  .db "Rectangle_NoClip", 0
G43:  .db "FillRectangle_NoClip", 0
G44:  .db "SetClipRegion", 0
G45:  .db "GetClipRegion", 0
G46:  .db "ShiftDown", 0
G47:  .db "ShiftUp", 0
G48:  .db "ShiftLeft", 0
G49:  .db "ShiftRight", 0
G50:  .db "Tilemap", 0
G51:  .db "Tilemap_NoClip", 0
G52:  .db "TransparentTilemap", 0
G53:  .db "TransparentTilemap_NoClip", 0
G54:  .db "TilePtr", 0
G55:  .db "TilePtrMapped", 0
G56:  .db "LZDecompress", 0
G57:  .db "AllocSprite", 0
Tab6:
G58:  .db "Sprite", 0
G59:  .db "TransparentSprite", 0
G60:  .db "Sprite_NoClip", 0
G61:  .db "TransparentSprite_NoClip", 0
G62:  .db "GetSprite_NoClip", 0
G63:  .db "ScaledSprite_NoClip", 0
G64:  .db "ScaledTransparentSprite_NoClip", 0
G65:  .db "FlipSpriteY", 0
G66:  .db "FlipSpriteX", 0
G67:  .db "RotateSpriteC", 0
G68:  .db "RotateSpriteCC", 0
G69:  .db "RotateSpriteHalf", 0
G70:  .db "Polygon", 0
G71:  .db "Polygon_NoClip", 0
G72:  .db "FillTriangle", 0
G73:  .db "FillTriangle_NoClip", 0
Tab7:
G74:  .db "LZDecompressSprite", 0
G75:  .db "SetTextScale", 0
G76:  .db "SetTransparentColor", 0
G77:  .db "ZeroScreen", 0
G78:  .db "SetTextConfig", 0
G79:  .db "GetSpriteChar", 0
G80:  .db "Lighten", 0
G81:  .db "Darken", 0
G82:  .db "SetFontHeight", 0
G83:  .db "ScaledSprite", 0
G84:  .db "FloodFill", 0
G85:  .db "RLETSprite", 0
G86:  .db "RLETSprite_NoClip", 0
G87:  .db "ConvertFromRLETSprite", 0
G88:  .db "ConvertToRLETSprite", 0
G89:  .db "ConvertToNewRLETSprite", 0
Tab8:
G90:  .db "RotateScaleSprite(", 0
G91:  .db "RotatedScaledTransparentSprite_NoClip(", 0
G92:  .db "RotatedScaledSprite_NoClip(", 0
      .db 0
      
; First token is $62 $0A
Tok1: .db 13, "DefineSprite("
Tok2: .db 5,  "Call "

TabData:
    .dl Tab1 - KeyHook_start
    .dl Tab2 - KeyHook_start
    .dl Tab3 - KeyHook_start
    .dl Tab4 - KeyHook_start
    .dl Tab5 - KeyHook_start
    .dl Tab6 - KeyHook_start
    .dl Tab7 - KeyHook_start
    .dl Tab8 - KeyHook_start
    
CData4:
    .dl F01 - KeyHook_start
    .dl F02 - KeyHook_start
    .dl F03 - KeyHook_start
    .dl F04 - KeyHook_start
    .dl F05 - KeyHook_start
    .dl F06 - KeyHook_start
    .dl F07 - KeyHook_start
    .dl F08 - KeyHook_start
    .dl F09 - KeyHook_start
    .dl F10 - KeyHook_start
    .dl F11 - KeyHook_start
    .dl F12 - KeyHook_start
    .dl F13 - KeyHook_start
    .dl F14 - KeyHook_start
    .dl F15 - KeyHook_start
    .dl F16 - KeyHook_start
    .dl F17 - KeyHook_start
    .dl F18 - KeyHook_start
    .dl F19 - KeyHook_start
    .dl F20 - KeyHook_start
    .dl F21 - KeyHook_start
    
CData5:
    .dl G01 - KeyHook_start
    .dl G02 - KeyHook_start
    .dl G03 - KeyHook_start
    .dl G04 - KeyHook_start
    .dl G05 - KeyHook_start
    .dl G06 - KeyHook_start
    .dl G07 - KeyHook_start
    .dl G08 - KeyHook_start
    .dl G09 - KeyHook_start
    .dl G10 - KeyHook_start
    .dl G11 - KeyHook_start
    .dl G12 - KeyHook_start
    .dl G13 - KeyHook_start
    .dl G14 - KeyHook_start
    .dl G15 - KeyHook_start
    .dl G16 - KeyHook_start
    .dl G17 - KeyHook_start
    .dl G18 - KeyHook_start
    .dl G19 - KeyHook_start
    .dl G20 - KeyHook_start
    .dl G21 - KeyHook_start
    .dl G22 - KeyHook_start
    .dl G23 - KeyHook_start
    .dl G24 - KeyHook_start
    .dl G25 - KeyHook_start
    .dl G26 - KeyHook_start
    .dl G27 - KeyHook_start
    .dl G28 - KeyHook_start
    .dl G29 - KeyHook_start
    .dl G30 - KeyHook_start
    .dl G31 - KeyHook_start
    .dl G32 - KeyHook_start
    .dl G33 - KeyHook_start
    .dl G34 - KeyHook_start
    .dl G35 - KeyHook_start
    .dl G36 - KeyHook_start
    .dl G37 - KeyHook_start
    .dl G38 - KeyHook_start
    .dl G39 - KeyHook_start
    .dl G40 - KeyHook_start
    .dl G41 - KeyHook_start
    .dl G42 - KeyHook_start
    .dl G43 - KeyHook_start
    .dl G44 - KeyHook_start
    .dl G45 - KeyHook_start
    .dl G46 - KeyHook_start
    .dl G47 - KeyHook_start
    .dl G48 - KeyHook_start
    .dl G49 - KeyHook_start
    .dl G50 - KeyHook_start
    .dl G51 - KeyHook_start
    .dl G52 - KeyHook_start
    .dl G53 - KeyHook_start
    .dl G54 - KeyHook_start
    .dl G55 - KeyHook_start
    .dl G56 - KeyHook_start
    .dl G57 - KeyHook_start
    .dl G58 - KeyHook_start
    .dl G59 - KeyHook_start
    .dl G60 - KeyHook_start
    .dl G61 - KeyHook_start
    .dl G62 - KeyHook_start
    .dl G63 - KeyHook_start
    .dl G64 - KeyHook_start
    .dl G65 - KeyHook_start
    .dl G66 - KeyHook_start
    .dl G67 - KeyHook_start
    .dl G68 - KeyHook_start
    .dl G69 - KeyHook_start
    .dl G70 - KeyHook_start
    .dl G71 - KeyHook_start
    .dl G72 - KeyHook_start
    .dl G73 - KeyHook_start
    .dl G74 - KeyHook_start
    .dl G75 - KeyHook_start
    .dl G76 - KeyHook_start
    .dl G77 - KeyHook_start
    .dl G78 - KeyHook_start
    .dl G79 - KeyHook_start
    .dl G80 - KeyHook_start
    .dl G81 - KeyHook_start
    .dl G82 - KeyHook_start
    .dl G83 - KeyHook_start
    .dl G84 - KeyHook_start
    .dl G85 - KeyHook_start
    .dl G86 - KeyHook_start
    .dl G87 - KeyHook_start
    .dl G88 - KeyHook_start
    .dl G89 - KeyHook_start
    .dl G90 - KeyHook_start
    .dl G91 - KeyHook_start
    .dl G92 - KeyHook_start
    
TokenHook_data:
    .dl Tok1 - KeyHook_start - 1
    .dl Tok2 - KeyHook_start - 1

CustomTokensProgramText:
    .db "PROGRAM:", 0
    
.echo "Data size: ",$-DataStart, " bytes"
.echo "Total size: ", $-KeyHook_start, " bytes"

Hooks_end:
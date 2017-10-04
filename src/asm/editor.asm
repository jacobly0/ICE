.assume adl = 1
segment data
.def _GotoEditor

_GotoEditor:
	ld	iy, 0D00080h		; flags
	call	0021A3Ch		; _DrawStatusBar
	call	00004F0h		; _usb_DisableTimer
	pop	bc
	pop	hl
	pop	de
	ld.sis	(0D008E3h & 0FFFFh), de	; errOffset
	ld	de, 0D0065Bh		; progToEdit
	ld	bc, 8
	ldir
	ld	de, 0D0EA1Fh		; plotSScreen
	ld	hl, StartProgramEditor
	ld	bc, StopProgramEditor - StartProgramEditor
	ldir
	jp	0D0EA1Fh		; plotSScreen
StartProgramEditor:
	ld	hl, 0D1A881h		; userMem
	ld	de, (0D0118Ch)		; asm_prgm_size
	call	0020590h		; _DelMem
	ld	a, 046h
	call	002016Ch		; _NewContext
	ld	sp, (0D007FAh)		; onSP
	call	002103Ch		; _resetStacks
	ld.sis	bc, (0D008E3h & 0FFFFh)	; errOffset
	ld	hl, (0D0243Dh)		; editTail
	ld	de, (0D0243Ah)		; editCursor
	ldir
	ld	(0D0243Dh), hl		; editTail
	ld	(0D0243Ah), de		; editCursor
FindPreviousEnter:
	call	0020CF8h		; _BufLeft
	jr	z, AtTopOfProgram
	ld	a, e
	cp	a, 03Fh
	jr	z, AtStartOfLine
	inc	bc
	jr	FindPreviousEnter
AtStartOfLine:
	call	0020CFCh
AtTopOfProgram:
	push	bc
	call	002081Ch
	ld	hl, 0000001h
	ld	(0D00595h), hl
	ld	a, ':'
	call	00207B8h
	call	0020D68h
	pop	bc
FindCursor:
	ld	a, b
	or	a, c
	jp	z, 0020154h
	call	0020D4Ch
	dec	bc
	jr	FindCursor
StopProgramEditor:
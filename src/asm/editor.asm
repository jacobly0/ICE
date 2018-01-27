.assume adl = 1
segment data
.def _GotoEditor

.ref __errsp

_GotoEditor:
	ld	iy, 0D00080h		; flags
	call	0021A3Ch		; _DrawStatusBar
	pop	bc
	pop	hl
	pop	de
	ld.sis	(008E3h), de		; errOffset
	ld	de, 0D0065Bh		; progToEdit
	ld	bc, 8
	ldir
	
; Cleanup C things
	ld	sp, (__errsp + 1);
	pop	af
	pop	de
	ld	(de), a
	pop	iy
	call	00004F0h		; usb_ResetTimers
	
	ld	de, 0D0EA1Fh		; plotSScreen
	ld	hl, StartProgramEditor
	ld	bc, StopProgramEditor - StartProgramEditor
	ldir
	jp	0D0EA1Fh		; plotSScreen
StartProgramEditor:
	ld	hl, 0D1A881h		; userMem
	ld	de, (0D0118Ch)		; asm_prgm_size
	call	0020590h		; _DelMem
	ld	a, 058h			; kExtApps
	ld	(0D007E0h), a		; cxCurApp
	ld	a, 046h			; kPrgmEd
	call	00208BCh		; _PullDownChk
	call	0020ED4h		; _ClrTR
	ld	a, 046h			; kPrgmEd
	call	0020170h		; _NewContext0
	ld	sp, (0D007FAh)		; onSP
	call	002103Ch		; _resetStacks
	ld.sis	bc, (008E3h)		; errOffset
	ld	hl, (0D0243Dh)		; editTail
	ld	de, (0D0243Ah)		; editCursor
	ld	a, b
	or	a, c
	jr	z, FindPreviousEnter
	ldir
	ld	(0D0243Dh), hl		; editTail
	ld	(0D0243Ah), de		; editCursor
FindPreviousEnter:
	call	0020CF8h		; _BufLeft
	jr	z, AtTopOfProgram
	ld	a, e
	cp	a, 03Fh			; tEnter
	jr	z, AtStartOfLine
	inc	bc
	jr	FindPreviousEnter
AtStartOfLine:
	call	0020CFCh		; _BufRight
AtTopOfProgram:
	push	bc
	call	002081Ch		; _ClrWindow
	ld	hl, 0000001h
	ld	(0D00595h), hl		; curRow
	ld	a, ':'
	call	00207B8h		; _PutC
	call	0020D68h		; _DispEOW
	pop	bc
FindCursor:
	ld	a, b
	or	a, c
	jp	z, 0020154h		; _Mon
	call	0020D4Ch		; _CursorRight
	dec	bc
	jr	FindCursor
StopProgramEditor:
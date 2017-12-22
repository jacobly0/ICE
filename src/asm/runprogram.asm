.assume adl = 1
segment data
.def _RunPrgm

_RunPrgm:
	scf
	sbc	hl, hl
	ld	(hl), 2
	ld	iy, 0D00080h		; flags
	;call	0021A3Ch		; _DrawStatusBar
	call	00004F0h		; _usb_DisableTimer
	pop	de
	pop	hl
	ld	de, 0D0065Bh		; progToEdit
	ld	bc, 8
	ldir
	ld	de, 0D0EA1Fh		; plotSScreen
	ld	hl, StartRunProgram
	ld	bc, EndRunProgram - StartRunProgram
	ldir
	jp	0D0EA1Fh		; plotSScreen
	
StartRunProgram:
	ld	hl, 0D1A881h		; userMem
	ld	de, (0D0118Ch)		; asm_prgm_size
	call	0020590h		; _DelMem
	ld	sp, (0D007FAh)		; onSP
	ld	a, 040h			; cxCmd
	jp	002016Ch		; _NewContext
EndRunProgram:
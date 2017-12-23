.assume adl = 1
segment data
.def _RunPrgm

_RunPrgm:
	call	0021A3Ch		; _DrawStatusBar
	ld	hl, 0005B05h
	ld	(0D005F8h), hl		; OP1
	ld	hl, 3
	add	hl, sp
	ld	hl, (hl)
	push	hl
	call	00000D4h		; __strlen
	push	hl
	inc	hl
	inc	hl
	inc	hl
	call	0020568h		; __CreateProg
	inc	de
	inc	de
	ex	de, hl
	ld	(hl), 0BBh		; t2ByteTok
	inc	hl
	ld	(hl), 06Ah		; tAsm
	inc	hl
	ld	(hl), 05Fh		; tPrgm
	inc	hl
	ex	de, hl
	pop	bc
	pop	hl
	ldir
	push	ix
	ld	hl, (0D0118Ch)		; asm_prgm_size
	push	hl
	call	00202C8h		; _OP4ToOP1
	call	0020F00h		; _ParseOP1
	ld	hl, 0005B05h
	ld	(0D005F8h), hl		; OP1
	call	002050Ch		; _ChkFindSym
	call	0020588h		; _DelVar
	pop	hl
	ld	(0D0118Ch), hl		; asm_prgm_size
	pop	ix
	ret
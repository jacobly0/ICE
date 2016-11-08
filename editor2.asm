OpenEditBuffer:
	ret
;	scf
;	sbc hl, hl
;	ld (hl), 2
;	ld de, progToEdit
;	ld hl, OP1+1
;	ld bc, 8
;	ldir
;	call _DeleteTempEditEqu
;	ld de, saveSScreen
;	ld hl, +_
;	ld c, ++_ - +_
;	ldir
;	jp saveSScreen
;_:	ld hl, UserMem
;	ld de, (asm_prgm_size)
;	call _DelMem
;	ld a, 046h
;	call _NewContext0
;	call _CallMain
;	jp _Mon
;_:




;	ld a, 088h
;	jp _JError
;	ld (errNo), a
;	res 7, (iy+4Bh)
;	res 2, (iy+12h)
;	res 4, (iy+24h)
;	res 1, (iy+49h)
;	ld sp, (errSP)
;	pop af
;	ret
;	ld hl, (OPBase)
;	pop de
;	add hl, de
;	ld (OPS), hl
;	pop de
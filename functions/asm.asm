functionAsm:
	call _IncFetch
	cp tasm
	ret nz
InsertASM:
	call _IncFetch
	ret c
	ld hl, hexadecimals
	ld bc, 16
	cpir
	ret nz
	ld d, c
	call _IncFetch
	ret c
	ld hl, hexadecimals
	ld bc, 16
	cpir
	ret nz
	ld a, d
	add a, a
	add a, a
	add a, a
	add a, a
	add a, c
	call InsertA
	jr InsertASM
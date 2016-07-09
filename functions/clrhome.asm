functionClrHome:
	ld a, 0CDh
	call InsertA											; call ******
	ld hl, _HomeUp
	call InsertHL											; call _HomeUp
	ld a, 0CDh
	call InsertA											; call ******
	ld hl, _ClrLCDFull
	call InsertHL											; call _ClrLCDFull
	jp _IncFetch
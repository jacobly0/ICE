functionPause:
	call _IncFetch
	call ParseExpression
	ld hl, 006140Eh
	call InsertHL											; ld c, 20 \ ld b, *
	ld hl, 0FE10B7h
	call InsertHL											; ld b, 183 \ djnz, 0
	ld hl, 0F9200Dh
	call InsertHL											; dec c \ jr nz, -5
	ld hl, 0F4203Dh
	jp InsertHL												; dec a \ jr nz, -10
functionIf:
	call _IncFetch
	call ParseExpression
	ld a, 0B7h
	call InsertA											; or a
	ld a, 0CAh
	call InsertA											; jp z, ******
	ld hl, (programPtr)
	push hl
		call InsertHL										; jp z, RANDOM
		call _IncFetch
		cp tThen
		jr nz, IfStatementOneCommand
IfStatementMoreCommands:
_:		call _IncFetch
		cp tEnter
		jr nz, -_
		call _IncFetch
		ld hl, AmountOfEnds
		inc (hl)
		call ParseToEnd
		jr IfStatementStop
IfStatementOneCommand:
		call ParseLine
IfStatementStop:
	pop hl
	ld de, (programPtr)										; jp z, XXXXXX
	ld (hl), de										
	ret
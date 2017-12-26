.assume adl = 1
segment data
.def _TimerData

_TimerData:
	ld	hl, 0F20031h
	set	1, (hl)			; Set count up
	dec	hl
	ld	a, (hl)
	and	a, 011111000b
	or	a, 000000011b
	ld	(hl), a			; Set 32768Hz crystal, enable
	ld	hl, (0F20000h)		; Return current time
	ret

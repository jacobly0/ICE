.assume adl = 1
segment data
.def _DispData

_DispData:
	ld	iy, 0D00080h
	res	1, (iy + 00Dh)
	jp	00207C0h
	ld	iy, 0D00080h
	res	1, (iy + 00Dh)
	jp	0021EE0h
.assume adl = 1
segment data
.def _DispData

_DispData:
	ld	iy, 0D00080h
	jp	00207C0h
	ld	iy, 0D00080h
	jp	0021EE0h
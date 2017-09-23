.assume adl = 1
segment data
.def _PrgmData

_PrgmData:
	call	0020798h
	set	1, (iy+8)
	call	0020F00h
	call	002079Ch
	res	1, (iy+8)

#include "relocation.inc"
#include "..\..\..\Assembly\ti84pce.inc"

 .libraryAppVar     "ICELIB"       ; Name of library on the calc
 .libraryName       "icelib"       ; Name of library
 .libraryVersion    1               ; Version information (1-255)
 
;-------------------------------------------------------------------------------
; v1 functions
;-------------------------------------------------------------------------------
 .function "ice_Compile",_Compile
 .function "ice_Register",_Register

 .beginDependencies
 .endDependencies

;-------------------------------------------------------------------------------
_Compile:
; Allocates space for a real list
; Arguments:
;  arg0 : dim
;  arg1 : pointer to malloc routine
	ret

;-------------------------------------------------------------------------------
_Register:
; Registers a program as the current ICE editor
; Arguments:
;  arg0 : program name
;  arg1 : struct with output
	ret

;-------------------------------------------------------------------------------
; Internal library data
;-------------------------------------------------------------------------------

ICEName:
	.db ProtProgObj, "ICE", 0
	
ICEEditName:
	.db AppvarObj, "ICEEDIT", 0

 .endLibrary

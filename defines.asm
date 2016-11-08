stackPtr				.equ saveSScreen+0000
outputPtr				.equ saveSScreen+0003
programPtr				.equ saveSScreen+0006
programNamesPtr			.equ saveSScreen+0009
labelPtr				.equ saveSScreen+0012
gotoPtr					.equ saveSScreen+0015
programDataOffsetPtr	.equ saveSScreen+0018
tempStringsPtr			.equ saveSScreen+0021
tempListsPtr			.equ saveSScreen+0024
programDataDataPtr		.equ saveSScreen+0027
amountOfPrograms		.equ saveSScreen+0030
openedParens			.equ saveSScreen+0031
amountOfArguments		.equ saveSScreen+0032
amountOfCRoutines		.equ saveSScreen+0033
amountOfEnds			.equ saveSScreen+0034

tempToken				.equ saveSScreen+0059
tempToken2				.equ saveSScreen+0060
InputStartData			.equ saveSScreen+0061
RandStartData			.equ saveSScreen+0064
backupSP				.equ saveSScreen+0067
backupCurPC				.equ saveSScreen+0070
backupEndPC				.equ saveSScreen+0073

arg1					.equ saveSScreen+0079
arg2					.equ saveSScreen+0082
arg3					.equ saveSScreen+0085
arg4					.equ saveSScreen+0088

stack					.equ saveSScreen+0100
output					.equ saveSScreen+0400
programNamesStack		.equ saveSScreen+1000
labelStack				.equ saveSScreen+1000
gotoStack				.equ saveSScreen+1200
programDataOffsetStack	.equ saveSScreen+1400
tempStringsStack		.equ saveSScreen+1800
tempListsStack			.equ saveSScreen+2700
spriteStack				.equ saveSScreen+4500
programDataData			.equ saveSScreen+6000

tempArg1				.equ pixelShadow2+0000
tempArg2				.equ pixelShadow2+1000
tempArg3				.equ pixelShadow2+2000
tempArg4				.equ pixelShadow2+3000
tempArg5				.equ pixelShadow2+4000
tempArg6				.equ pixelShadow2+5000

startTab				.equ saveSScreen

program					.equ vRAM+(320*240)

myFlags					.equ -30h
prev_is_number			.equ 0										; used if a number consists of more digits
chain_operators			.equ 1										; used if the calculation is needed for another calculation
triggered_a_comma		.equ 2										; used if a comma is entered outside parens, so only useful for "Disp"
multiple_arguments		.equ 3										; "Disp" can display more than one number/string
output_is_number		.equ 4										; used if the calculation is only a single number, loop-statements can be 
ans_set_z_flag			.equ 5										; used if the previous calculation (re)sets the zero flag, useful for loop/conditions
need_push				.equ 6										; used if Ans may not be overwritten
good_compilation		.equ 7										; used if compilation is succesfull

myFlags2				.equ -31h
has_already_input		.equ 0										; only once time "Input" routine
has_already_rand		.equ 1										; only once time "rand" routine
output_is_string		.equ 2										; used for strings
negative_for_step		.equ 3										; used for the step in a For loop
for_step_is_number		.equ 4										; used for the step in a For loop
end_point_is_number		.equ 5										; used for the end point in a For loop
last_token_is_ret		.equ 6										; used if last token is "Return"

myFlags3				.equ -32h
comp_with_libs			.equ 0										; used if compile with C libs
displayed_det			.equ 1										; used if the text of a det( function is displayed in the statusbar

myFlags4				.equ -33h
arg1_is_small			.equ 4										; used in C functions
function_implemented	.equ 4										; used if C function is implemented
arg2_is_small			.equ 3										; used in C functions
arg3_is_small			.equ 2										; used in C functions
arg4_is_small			.equ 1										; used in C functions
arg5_is_small			.equ 0										; used in C functions

typeNumber				.equ 0
typeVariable			.equ 1
typeChainPush			.equ 2
typeChainAns			.equ 3
typeReturnValue			.equ 4										; getKey, rand
typeList				.equ 5
typeOSList				.equ 6
typeString				.equ 7
typeFunction			.equ 00111111b
typeOperator			.equ 00011111b

ChainPush				.equ 0
ChainAns				.equ 1
ChainListPush			.equ 2
ChainListAns			.equ 3

OutputInBC				.equ 0
OutputInDE				.equ 1
OutputInHL				.equ 2

;---------------------------;
;----- User Variables ------;
;---------------------------;
Str0					.equ saveSScreen+15500
Str1					.equ saveSScreen+16000
Str2					.equ saveSScreen+16500
Str3					.equ saveSScreen+17000
Str4					.equ saveSScreen+17500
Str5					.equ saveSScreen+18000
Str6					.equ saveSScreen+18500
Str7					.equ saveSScreen+19000
Str8					.equ saveSScreen+19500
Str9					.equ saveSScreen+20000

L1						.equ saveSScreen
L2						.equ cmdPixelShadow
L3						.equ pixelShadow2
L4						.equ plotSScreen
L5						.equ pixelShadow
L6						.equ cursorImage

; IX offsets
vA						.equ 0
vB						.equ 3
vC						.equ 6
vD						.equ 9
vE						.equ 12
vF						.equ 15
vG						.equ 18
vH						.equ 21
vI						.equ 24
vJ						.equ 27
vK						.equ 30
vL						.equ 33
vM						.equ 36
vN						.equ 39
vO						.equ 42
vP						.equ 45
vQ						.equ 48
vR						.equ 51
vS						.equ 54
vT						.equ 57
vU						.equ 60
vV						.equ 63
vW						.equ 66
vX						.equ 69
vY						.equ 72
vZ						.equ 75
vtheta					.equ 78

rand1					.equ 81
rand2					.equ 84

tReadByte				.equ 10
tReplaceByte			.equ 11
tAddByte				.equ 12
tDeleteByte				.equ 13
tSprite					.equ 14
tRectangle				.equ 15
tBackground				.equ 16
tSetBPP					.equ 17
tExecHex				.equ 18
tSwitch					.equ 19
tcase					.equ 20
tCall					.equ 21
tSetuPrgm				.equ 22
tCreateVar				.equ 23
tArchiveVar				.equ 24
tUnArchiveVar			.equ 25
tDeleteVar				.equ 26
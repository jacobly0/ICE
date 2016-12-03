tempToken				.equ saveSScreen+00
tempToken2				.equ saveSScreen+01
InputStartData			.equ saveSScreen+02
RandStartData			.equ saveSScreen+05
PauseStartData			.equ saveSScreen+08
MeanStartData			.equ saveSScreen+11
KeypadStartData			.equ saveSScreen+14

stack					.equ saveSScreen+00100
output					.equ saveSScreen+01100
programNamesStack		.equ saveSScreen+03000
labelStack				.equ saveSScreen+03000
gotoStack				.equ saveSScreen+04000
programDataOffsetStack	.equ saveSScreen+05000
tempStringsStack		.equ saveSScreen+06500
tempListsStack			.equ saveSScreen+08000
spriteStack				.equ saveSScreen+10000
programDataData			.equ saveSScreen+12000

tempArg1				.equ pixelShadow2+0000
tempArg2				.equ pixelShadow2+1000
tempArg3				.equ pixelShadow2+2000
tempArg4				.equ pixelShadow2+3000
tempArg5				.equ pixelShadow2+4000
tempArg6				.equ pixelShadow2+5000

startTab				.equ saveSScreen

program					.equ vRAM+(320*240)

fProgram1				.equ -30h
good_compilation		.equ 0										; used if compilation is succesfull
comp_with_libs			.equ 1										; used if compile with C libs
has_already_input		.equ 2										; only once time "Input" routine
has_already_rand		.equ 3										; only once time "rand" routine
has_already_pause		.equ 4										; only once time "Pause" routine
has_already_mean		.equ 5										; only once time "mean()" routine
has_already_keypad		.equ 6										; only once time scanning the keypad

fExpression1			.equ -40h
prev_is_number			.equ 0										; used if a number consists of more digits
chain_operators			.equ 1										; used if the calculation is needed for another calculation
last_token_is_ret		.equ 2										; used if last token is "Return"
op_is_last_one			.equ 3										; used if the operator is the last of that line
output_is_number		.equ 4										; used if the calculation is only a single number, loop-statements can be 
ans_set_z_flag			.equ 5										; used if the previous calculation sets the zero flag, useful for loop/conditions
need_push				.equ 6										; used if Ans may not be overwritten
output_is_string		.equ 7										; used for strings

fExpression2			.equ -41h
use_mean_routine		.equ 0										; used if the function is mean()
triggered_a_comma		.equ 1										; used if a comma is entered outside parens, used by functions

fFunction1				.equ -50h
arg1_is_small			.equ 4										; used in C functions
function_implemented	.equ 4										; used if C function is implemented
arg2_is_small			.equ 3										; used in C functions
arg3_is_small			.equ 2										; used in C functions
arg4_is_small			.equ 1										; used in C functions
arg5_is_small			.equ 0										; used in C functions

fFunction2				.equ -51h
negative_for_step		.equ 0										; used for the step in a For loop
for_step_is_number		.equ 1										; used for the step in a For loop
end_point_is_number		.equ 2										; used for the end point in a For loop

fAlways1				.equ -50h
displayed_det			.equ 0										; used if the text of a det( function is displayed in the statusbar

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

OutputIsInDE			.equ 0
OutputIsInHL			.equ 1

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
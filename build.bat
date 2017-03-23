@ECHO OFF
if not exist bin\ mkdir bin
tools\spasm -E -T -L ice.asm bin\ICE.8xp
tools\convhex -x bin\ICE.8xp
del "bin\ICE.8xp"
ren bin\ICE_.8xp ICE.8xp
Pause
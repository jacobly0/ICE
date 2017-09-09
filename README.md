# ICE Compiler
![](http://i.imgur.com/yLPnSG7.png)
ICE Compiler is a program that compiles TI-BASIC-like language to ASM, the machine code. Create insane games, use the color screen, and make the step to ASM smaller! Speed up your BASIC programs and it's super easy to use! Type your program in the normal editor, compile it within seconds, and you have hours of fun! It doesn't require any shell.

### Build
If you want to compile it for the TI-84 Plus CE, be sure you have the [C toolchain](http://tiny.cc/clibs) installed. Then type `make` from the command line. It should take a few seconds, and then the `ICE.8xp` is created in the `bin` folder. If you want to compile it for the computer, type `make -f makefile.computer` from the command line. That should create `ICE.exe` in the main folder.

### Navigation
Run `Asm(prgmICE` from the homescreen. Now it shows you a list of unprotected programs. Press `[UP]` and `[DOWN]` to select the right program, and press `[ENTER]`. It now compiles your program!

### Bugs
If you find a bug, or when you see the message `ICE ERROR: please report it!`, be sure to make an issue here, or post it in the [Cemetech thread](https://www.cemetech.net/forum/viewtopic.php?t=12616).
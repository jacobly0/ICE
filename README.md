# ICE Compiler
![](http://i.imgur.com/yLPnSG7.png)

ICE Compiler is a program that compiles TI-BASIC-like language to assembler. Create insane games, take full advanage of the color screen, and close the gap to using other languages! Not only will your Basic programs enjoy increased speed, it is also easy to use. Type your program in the normal editor, compile within seconds, for hours of fun! No shell is required.

### Navigation
Run `Asm(prgmICE` from the homescreen. This will show a list of compilable programs. Use the `[UP]` and `[DOWN]` keys to select the right program, and press `[ENTER]`. Your program will now be compiled!

### Build
If you want to compile it for the TI-84 Plus CE, be sure you have the [C toolchain](http://tiny.cc/clibs) installed. Then run `make` from the command line. After a few seconds, `ICE.8xp` will be placed in the `bin` folder.

If you want to compile it for the computer, be sure to install [mingw](https://sourceforge.net/projects/mingw/files/), and run `make -f makefile.computer` from the command line. That should create `ICE.exe` in the main folder.

### Bugs
If you find a bug, or if you see the message `ICE ERROR: please report it!`, be sure to make an issue here, or post it on the relevant [Cemetech topic](https://www.cemetech.net/forum/viewtopic.php?t=12616).

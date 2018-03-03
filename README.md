# ICE Compiler
![](http://i.imgur.com/yLPnSG7.png)

ICE Compiler is a program that compiles TI-BASIC-like language to assembler. Create insane games, take full advanage of the color screen, and close the gap to using other languages! Not only will your Basic programs enjoy increased speed, it is also easy to use. Type your program in the normal editor, compile within seconds, for hours of fun! No shell is required.

### Usage
#### Calculator
Run `Asm(prgmICE` from the homescreen. This will show a list of compilable programs. Use the `[UP]` and `[DOWN]` keys to select the right program, and press `[ENTER]` or `[2ND]`. Your program will now be compiled!  You can now run your program with `Asm(prgmNAME` or press `[Y=]` after compiling.

#### Computer
From the command line, run `ice <input program>` to compile the input programs. Note that if you want to compile subprograms too, put them in the same folder as `ice.exe`, so preferable put `ice.exe` in the same folder as all your input programs.

### Build
If you want to compile it for the TI-84 Plus CE, be sure you have the [C toolchain](http://tiny.cc/clibs) installed. Then run `make` from the command line (or run `build_calc.bat`). After a few seconds, `ICE.8xp` will be placed in the `bin` folder.

If you want to compile it for the computer, be sure to install [mingw](https://sourceforge.net/projects/mingw/files/), and run `make -f makefile.computer` from the command line (or run `build_comp.bat`). That should create `ice.exe` in the main folder.

If you want to compile the hooks for the calculator, download [spasm-ng](https://github.com/alberthdev/spasm-ng/releases) and run `spasm -E hooks\hooks.asm bin\ICEAPPV.8xv` from the command line (or run the `build_hooks.bat` file). The `build_hooks.bat` file will create the `bin/` folder for you if it does not exist.

### Bugs
If you find a bug, or if you see the message `ICE ERROR: please report it!`, be sure to make an issue here, or post it on the relevant [Cemetech topic](https://www.cemetech.net/forum/viewtopic.php?t=12616).

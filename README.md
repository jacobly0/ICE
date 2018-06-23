# ICE Compiler
![](http://i.imgur.com/yLPnSG7.png)

ICE Compiler is a program that compiles TI-BASIC-like language to assembler. Create insane games, take full advanage of the color screen, and close the gap to using other languages! Not only will your Basic programs enjoy increased speed, it is also easy to use. Type your program in the normal editor, compile within seconds, for hours of fun! No shell is required.

## Build
To build ICE for the calculator, be sure to have the [C SDK](https://github.com/CE-Programming/toolchain/releases) installed. Then run `make` from the command line. To build ICE for the computer, run `make -f makefile.computer` from the command line.

## Download
See the [Cemetech Archives](https://www.cemetech.net/programs/index.php?mode=file&path=/84pce/asm/programs/ICECompiler.zip) for the latest download and documentation.

## Usage
### Calculator
Run `Asm(prgmICE` from the homescreen. This will show a list of compilable programs. Use the `[UP]` and `[DOWN]` keys to select the right program, and press `[ENTER]` or `[2ND]`. Your program will now be compiled!  You can now run your program with `Asm(prgmNAME` or press `[Y=]` after compiling.

### Computer
From the command line, run `ice <input program>` to compile the input programs. Note that if you want to compile subprograms too, put them in the same folder as `ice.exe`, so preferable put `ice.exe` in the same folder as all your input programs.

## Bugs
If you find a bug, or if you see the message `ICE ERROR: please report it!`, be sure to make an issue here, or post it on the relevant [Cemetech topic](https://www.cemetech.net/forum/viewtopic.php?t=12616). **Note:** please providing necessary code as well, that is quite useful for debugging.

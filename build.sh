#!/bin/bash
# By Peter PT_ Tillema
make debug
make -f makefile.computer
cd src/asm
for f in *.bin
do
  xxd -i $f | sed 's+unsigned char \(.\)\(.*\)\_bin+const uint8\_t \U\1\E\2Data+' > ${f%.*}.c
done
printf "#include <stdint.h>\n#ifdef __EMSCRIPTEN__\n" > a.txt
printf "#endif" > b.txt
cat a.txt *.c b.txt > ../data.c
rm *.c *.txt
cd ..
cp ../include/tice.h tice.h
emcc data.c errors.c export.c functions.c ice_sc.c main_sc.c operator.c parse.c routines.c stack.c -o ../bin/compiler.js -s EXPORTED_FUNCTIONS="['_ice_open_first_prog', '_ice_open', '_ice_open', '_ice_close', '_ice_error', '_ice_export']"
rm tice.h

#!/bin/bash
# By Peter PT_ Tillema
make
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

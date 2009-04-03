#!/bin/sh -ex
# this script is meant to be executed by running "make check" 

../src/drawtiming -o sample.ps $srcdir/sample.txt
../src/drawtiming -o statement1.ps $srcdir/statement1.txt
../src/drawtiming -x 1.5 -o memory.ps $srcdir/memory.txt
../src/drawtiming -p 640x480 -o sample640x480.ps $srcdir/sample.txt
../src/drawtiming -o guenter.ps $srcdir/guenter.txt

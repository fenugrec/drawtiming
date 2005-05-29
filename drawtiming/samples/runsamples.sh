#!/bin/sh -ex
# this script is meant to be executed by running "make check" 

../src/drawtiming -o sample.gif $srcdir/sample.txt
../src/drawtiming -o statement1.gif $srcdir/statement1.txt
../src/drawtiming -x 1.5 -o memory.gif $srcdir/memory.txt
../src/drawtiming -p 640x480 -o sample640x480.gif $srcdir/sample.txt

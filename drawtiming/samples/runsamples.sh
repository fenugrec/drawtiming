#!sh -ex
# this script is meant to be run by issuing "make check" 

../src/drawtiming -o sample.gif $srcdir/sample.txt
../src/drawtiming -o turnup.gif $srcdir/turnup.txt
../src/drawtiming -o statement1.gif $srcdir/statement1.txt

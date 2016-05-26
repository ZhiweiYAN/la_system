#!/bin/sh
#file:do_release.sh
#author:ssurui
#date:2007-9-19

make clean
make REL=1 LIAN=1
rm -f *.c
rm -f *.h
rm -f *.o
rm Makefile
mv Makefile_1 Makefile

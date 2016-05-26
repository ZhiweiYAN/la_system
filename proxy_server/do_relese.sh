#!/bin/sh
#file:do_release.sh
#author:ssurui
#date:2007-9-19

make clean
make
rm -f *.c
rm -f *.h
rm Makefile
mv Makefile_1 Makefile

#!/bin/sh
#file:do_release.sh
#author:ssurui
#date:2007-9-19

gcc -c shmsem.c
gcc -c config.c
gcc -c common.c
gcc -c ctl_channel.c
gcc -c longlink.c
gcc -c check.c
gcc -c multi_recvsend.c
gcc -c database.c
rm -f shmsem.c
rm -f config.c
rm -f common.c
rm -f ctl_channel.c
rm -f longlink.c
rm -f check.c
rm -f multi_recvsend.c
rm -f database.c
rm Makefile
mv Makefile_1 Makefile

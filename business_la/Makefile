# #####################################################
#This software is designed for Guang Dong LIAN Company.
#Makefile for BUSINESS_SERVER
#It begins in JAN 25, 2013
# ####################################################
PROJECT=GD_LIAN_BUSINESS_SERVER


CC=g++
#CC=gcc
AR=ar

ifdef REL
CFLAGS= -O2 -s -DNDEBUG
TARGET=run_gdlian_business_release_version
else
CFLAGS= -g -Wall -DDEBUG -O0
TARGET=run_gdlian_business_debug_version
endif

GLOG_INC=-L /usr/include 
GLOG_LIB=-L /usr/lib

INC_DIR_FLAGS=$(GLOG_INC)  

OBJS=shmsem.o common.o ctl_channel.o longlink.o check.o \
			serial_number.o  multi_recvsend.o affair.o \
			../config_h_c/config.o bank.o \
			middleware.o  analyse.o database.o  \
			business.o

$(TARGET): $(OBJS) 
	$(CC) -o $(TARGET)  $(GLOG_LIB) -lglog $(OBJS)

$(OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $(INC_DIR_FLAGS) $< -o $@

.PHONY:clean
clean:
	rm --force *.o
	rm --force $(OBJS)
	rm --force $(TARGET) 
	rm --force *.bak
	rm --force *.orig
	rm --force log_error.txt
#	rm --force *.log
#	rm --force ./log/*.*
	rm --force run_*
.PHONY:pp
pp:
	-find ./ -name "*" -print |egrep '\.cpp$$|\.c$$|\.h$$' |xargs astyle --style=linux -s -n;

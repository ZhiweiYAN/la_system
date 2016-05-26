# #####################################################
#This software is designed for Guang Dong LIAN Company.
#It begins in Nov 15, 2012
# ####################################################
PROJECT=GD_LIAN_DB_INVOICE


CC=g++
#CC=gcc
AR=ar

ifdef REL
CFLAGS= -O2 -s -DNDEBUG
TARGET=run_gdlian_bkground_release_version
else
CFLAGS= -g -Wall -DDEBUG -O0
TARGET=run_gdlian_bkground_debug_version
endif

ifdef LIAN
PG_INC=-I /usr/local/pgsql/include
PG_LIB=-L /usr/local/pgsql/lib
else
PG_INC=-I /usr/include/postgresql
PG_LIB=-L /usr/lib
endif
GLOG_INC=-L /usr/include
GLOG_LIB=-L /usr/lib

INC_DIR_FLAGS=$(GLOG_INC) $(PG_INC) 

OBJS=shmsem.o ../config_h_c/config.o longlink.o \
			 update_terminal.o bk_init.o bk_monitor.o bk_main.o  

$(TARGET): $(OBJS) 
	$(CC) -o $(TARGET) $(PG_LIB) -lpq  $(GLOG_LIB) -lglog $(OBJS)

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


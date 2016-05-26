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
TARGET=run_gdlian_db_release_version
else
CFLAGS= -g -Wall -DDEBUG -O0 -gdwarf-2 -g3
TARGET=run_gdlian_db_debug_version
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

OBJS=primary_db_main.o primary_db_start.o primary_db_init.o \
			business_data_analysis.o shmsem.o  \
			primary_db_monitor_process.o primary_db_sync.o \
			primary_db_watch_dog.o primary_db_comm_proxy.o \
			primary_db_comm_database.o primary_db_heart_beat.o \
			check.o ctl_channel.o longlink.o  \
			parse_pkt_common.o \
			parse_pkt_common_header.o  \
			primary_db_record.o primary_db_view.o \
			multi_recvsend.o primary_db_cas.o \
			primary_db_error.o  \
			../config_h_c/config.o 

$(TARGET): $(OBJS) 
	$(CC) -o $(TARGET) $(PG_LIB) -lpq  $(GLOG_LIB) -lglog $(OBJS)

$(OBJS):%.o:%.c
	$(CC) -c $(CFLAGS) $(INC_DIR_FLAGS) $< -o $@

.PHONY:clean
clean:
	rm --force *.o
	rm --force $(TARGET) 
	rm --force *.bak
	rm --force *.orig
	rm --force log_error.txt
	rm --force *.log
	rm --force ./log/*.*
	rm --force run_*
.PHONY:pp
pp:
	-find ./ -name "*" -print |egrep '\.cpp$$|\.c$$|\.h$$' |xargs astyle --style=linux -s -n;


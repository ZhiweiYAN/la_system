/* *************************************************
 * File name:
 * 		pri_db_watch_dog.cc
 * Description:
 * 		The program is run at the primary database server.
 * 		It receives the data request packet from Slave database.
 * Author:
 * 		Zhiwei Yan, jerod.yan@gmail.com
 * Date:
 * 		2012-12-14
 * *************************************************/
#ifndef PRI_DB_WATCH_DOG_H
#define PRI_DB_WATCH_DOG_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "primary_db_sync.h"
#include "shmsem.h"
#include "primary_db_common.h"
#include "primary_db_monitor_process.h"
#include "primary_db_comm_database.h"
#include "primary_db_init.h"
#include "libpq-fe.h"
#include "check.h"

int Init_db_check_watch_dog(void);
int Wake_up_check_watch_dog(void);
#endif


/* *************************************************
 * File name:
 * 		primary_db_start.h
 * Description:
 * 		The program is run at the primary database server.
 * 		It receives the data packet from business A or B.
 * 		Then it records the data in the databases both the
 * 		primary database and secondary database.
 * Author:
 * 		Zhiwei Yan, jero.yan@gmail.com
 * Date:
 * 		2007-01-03
 * *************************************************/
#ifndef PRIMARY_DB_START_H
#define PRIMARY_DB_START_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <assert.h>

#include "libpq-fe.h"

#include "shmsem.h"
#include "primary_db_monitor_process.h"
#include "primary_db_common.h"
#include "business_data_analysis.h"
#include "primary_db_sync.h"
#include "primary_db_watch_dog.h"
#include "primary_db_comm_proxy.h"
#include "primary_db_init.h"
#include "primary_db_record.h"
#include "primary_db_view.h"
#include "check.h"
#include "multi_recvsend.h"
#include "primary_db_cas.h"

int Init_db_server(void);
int Daemon_db_record_server(int welcome_sd,struct sockaddr_in *sa_business);
int Daemon_db_view_server(int welcome_sd,struct sockaddr_in *sa_business);
int Verify_peer_ip_valid(int sockfd);
#endif


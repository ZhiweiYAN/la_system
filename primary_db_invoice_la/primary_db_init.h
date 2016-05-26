/* *************************************************
 * File name:
 * 		primary_db_init.h
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
#ifndef PRIMARY_DB_INIT_H
#define PRIMARY_DB_INIT_H
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
#include "libpq-fe.h"
#include "shmsem.h"
#include "primary_db_monitor_process.h"
#include "primary_db_common.h"
#include "business_data_analysis.h"
#include "primary_db_sync.h"
#include "primary_db_watch_dog.h"
#include "primary_db_comm_proxy.h"
#include "primary_db_heart_beat.h"
#include "primary_db_monitor_process.h"
#include "check.h"

int Setup_config_parameters(void);

int Start_network_service(int port,int *welcome_sd, struct sockaddr_in *sa, const char* s);
int Init_balance_check_socket(int port,int *welcome_sd, struct sockaddr_in *sa);
int Init_db_view_socket(int port,int *welcome_sd, struct sockaddr_in *sa);
int Init_db_record_socket(int port,int *welcome_sd, struct sockaddr_in *sa);

int Init_server_mode_type_share_memory(int share_id);
int Init_process_manager_share_memory(int share_id);

int Start_life_time_counter_process(void);
int Start_monitor_process(void);
int Start_recv_heart_beat_pkt_process(void);
int Start_send_heart_beat_pkt_process(void);

#endif

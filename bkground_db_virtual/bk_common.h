/* *************************************************
 * File name:
 * 		bk_common.h
 * Description:
 * 		The program is run at the database server.
 * Author:
 * 		Zhiwei Yan, jero.yan@gmail.com
 * Date:
 * 		2007-07-7
 * *************************************************/
#ifndef BK_COMMON_H 
#define BK_COMMON_H 0

#include <stdio.h>
#include <stdlib.h> 
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include "../config_h_c/config.h"
#include "libpq-fe.h" 
#include <glog/logging.h>

#define SQL_STRING_LENGTH 500 
#define CONFIG_FILENAME "../cfg_files/global.cfg"
#define MAXSLEEP 30
#define COMM_LENGTH 512
#define DEPOSIT_LENGTH 15

#define MAX_PROCESS_NUMBRER 1024
#define MAX_PACKET_SIZE 1000
#define MIN_PACKET_LENGTH 4
#define SEM_PROCESS_FTOK_ID 200
#define SHM_PROCESS_FTOK_ID 201

#define BACKLOG 200
#define DELAY_MONITOR_TIME 5

#define OUTPUT_OK printf("\t[\033[32mOK\033[0m]\n");
#define OUTPUT_ERROR  printf("\t[\033[31mERROR\033[0m]\n");

#define _PRIMARY_FININACE_CONTROL_DEBUG_  30
#define _PRIMARY_DB_VIEW_DEBUG_ 30

#define _DEBUG_SHOW 1
#ifdef _DEBUG_SHOW
#define _DEBUG_FINANCIAL_INFO_ 1
#endif

//#define RENEW_DEPOSIT_PROCESS_REFRESH_INTERVAL  1200
//#define RENEW_DEPOSIT_PROCESS_DEADLINE  (RENEW_DEPOSIT_PROCESS_REFRESH_INTERVAL*6)

//#define RENEW_BALANCE_PROCESS_REFRESH_INTERVAL  7200*12
//#define RENEW_BALANCE_PROCESS_DEADLINE  (7200*36)

#define RENEW_DEPOSIT_PROCESS_REFRESH_INTERVAL  60
#define RENEW_DEPOSIT_PROCESS_DEADLINE  300

#define RENEW_BALANCE_PROCESS_REFRESH_INTERVAL  60
#define RENEW_BALANCE_PROCESS_DEADLINE  300

#define ACCOUNT_SERVER_PROCESS_DEADLINE  1200

/*define the structure of packet from financial*/
#define FINANCIAL_UPDATE_ALL_RECKONING_DATE_TYPE "00"
#define FINANCIAL_UPDATE_APPOINTED_RECKONING_DATE_TYPE "01"
#define FINANCIAL_UPDATE_APPOINTED_DEPOSIT_TYPE "02"
#define FINANCIAL_REQUEST_UPDATE_DEPOSIT_TYPE "03"
#define FINANCIAL_DAY_REPORT_TYPE "04"
#define FINANCIAL_MONTH_REPORT_TYPE "05"
#define FINANCIAL_CREATE_TERMINAL_MANAGE_VIEW "06CREATE_TERMINAL_MANAGE_VIEW"
#define FINANCIAL_MONTH_TERMINAL_STATISTICS_TYPE "07"
#define FINANCIAL_MONTH_WORKER_STATISTICS_TYPE "08"
#define FINANCIAL_CREATE_TERMINAL_MANAGE_VIEW_START_POSTION 0
#define FINANCIAL_CREATE_TERMINAL_MANAGE_VIEW_LENGTH 29
#define FINANCIAL_TYPE_IN_FORWARD_PACKET_START_POSTION 0
#define FINANCIAL_TYPE_IN_FORWARD_PACKET_LENGTH 2
#define FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_START_POSTION 2
#define FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_LENGTH 8
#define FINANCIAL_DEPOSIT_IN_FORWARD_PACKET_START_POSTION 10
#define FINANCIAL_DEPOSIT_IN_FORWARD_PACKET_LENGTH 10
#define FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_START_POSTION 10
#define FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_LENGTH 10
#define FINANCIAL_DAY_REPORT_DATE_IN_FORWARD_PACKET_START_POSITION 2
#define FINANCIAL_DAY_REPORT_DATE_IN_FORWARD_PACKET_LENGTH 10
#define FINANCIAL_MONTH_REPORT_START_DATE_IN_FORWARD_PACKET_START_POSITION 2
#define FINANCIAL_MONTH_REPORT_START_DATE_IN_FORWARD_PACKET_LENGTH 10
#define FINANCIAL_MONTH_REPORT_END_DATE_IN_FORWARD_PACKET_START_POSITION 12
#define FINANCIAL_MONTH_REPORT_END_DATE_IN_FORWARD_PACKET_LENGTH 10
#define FINANCIAL_MONTH_REPORT_COMPANY_IN_FORWARD_PACKET_START_POSITION 22

#define FINANCIAL_MONTH_TERMINAL_STATISTICS_START_DATE_IN_PACKET_START_POSITION 2
#define FINANCIAL_MONTH_TERMINAL_STATISTICS_START_DATE_IN_PACKET_LENGTH 10
#define FINANCIAL_MONTH_TERMINAL_STATISTICS_END_DATE_IN_PACKET_START_POSITION 12
#define FINANCIAL_MONTH_TERMINAL_STATISTICS_END_DATE_IN_PACKET_LENGTH 10

#define FINANCIAL_MONTH_WORKER_STATISTICS_START_DATE_IN_PACKET_START_POSITION 2
#define FINANCIAL_MONTH_WORKER_STATISTICS_START_DATE_IN_PACKET_LENGTH 10
#define FINANCIAL_MONTH_WORKER_STATISTICS_END_DATE_IN_PACKET_START_POSITION 12
#define FINANCIAL_MONTH_WORKER_STATISTICS_END_DATE_IN_PACKET_LENGTH 10


#define TERMINAL_ID_LENGTH 8

/*define time of auto update bank deposit in one day*/
#define AUTO_UPDATE_DEPOSIT_TIME_0 8
#define AUTO_UPDATE_DEPOSIT_TIME_1 13
#define AUTO_UPDATE_DEPOSIT_CLEAR_TIME_THRESHOLD 18

/* PROCESS TYPE in the SYSTEM */
enum ProcessType{
	NORMAL_PROCESS=0,	RENEW_DEPOSIT_PROCESS, RENEW_BALANCE_PROCESS,
	ACCOUNT_SERVER_PROCESS
};


struct ChildProcessStatus
{
	pid_t pid;
	int life_time;
	int deadline;
	enum ProcessType type;
};

struct ShareMemProcess
{
	struct ChildProcessStatus process_table[MAX_PROCESS_NUMBRER];
};

struct Terminal_ID
{
	char terminal_id[TERMINAL_ID_LENGTH+1];
};

#define SEM_PROTECTION_FTOK_ID 202
#define SHM_PROTECTION_FTOK_ID 203
struct UpdateProtection 
{
	int update_all_latest_reckoning_date_flag;
	pid_t pid;

};
#endif

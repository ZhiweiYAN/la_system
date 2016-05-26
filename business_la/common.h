/*********************************************************
 *project: Line communication charges supermarket
 *filename: common.h
 *version: 0.4
 *purpose: common data structure and operation about them
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-9
 *********************************************************/
#ifndef COMMON_H
#define COMMON_H

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
#include <time.h>
#include "../config_h_c/config.h"
#include "shmsem.h"
#include <glog/logging.h>

#define MAXSLEEP 30  /*the unit is second*/
#define SHORT_SLEEP 100 /*the unit is us, not second*/
#define RECONNECT_INTERVAL 2

#define TEMP_LENGTH 50 
#define TMP_STRING_LENGTH 50 
#define MAX_REPORT_PKT_SIZE 100
#define CONFIG_FILENAME "../cfg_files/global.cfg"
#define MAXDATASIZE 100
#define MAXPACKETSIZE 40000
#define MAXCONNECTIONNUM 400
#define NUM_PROCESS_QUEUE (2*MAXCONNECTIONNUM)

/*Here, we describle the behavor of the check process and the control channel process
1. each time, control channel process reset the common counter to 0,
2. each time, check process add a step to a common counter,
and judge the counter, if the counter >T, it means the control channel not work,
so the check process ReCreate a control channel process */
#define REPORT_STATUS_INTERVAL 5     /*every ? second, control channel process reset the common counter*/

#define CHECK_CONTROL_CHANNEL_INTERVAL 5  /*every ? second check the control channel*/
#define THRESHOLD_BROKEN_CONTROL_CHANNEL 60  /*if the count > this Threshold, control channel is brokon*/
#define CHECK_ADD_STEP 5   /*each time, the check process add this value to count*/

#define THRESHOLD_BLOCK_DATA_CHANNEL 300  /*if the count > this Threshold, data channel is blocked*/

#define SEM_SERIAL_FTOK_ID 1 /*use this value to make a key_t for semphore of serial number*/
#define SHM_SERIAL_FTOK_ID 2 /*use this value to make a key_t for share memory of serial number*/
#define SEM_DATA_CHAN_FTOK_ID 3 /*use this value to make a key_t for semphore of Queue_business_process*/
#define SHM_DATA_CHAN_FTOK_ID 4 /*use this value to make a key_t for share memory of Queue_business_process*/
#define SEM_DB_STATUS_FTOK_ID 5 /*use this value to make a key_t for semphore of DB_status*/
#define SHM_DB_STATUS_FTOK_ID 6 /*use this value to make a key_t for share memory of DB_status*/
#define SEM_CTL_FTOK_ID_BEGIN 10 /*use this value to make a key_t for semphore of proxy machines control*/
#define SHM_CTL_FTOK_ID_BEGIN 11 /*use this value to make a key_t for share memory of proxy machines control*/

#define OUTPUT_OK printf("\t\t[\33[32mOK\033[0m]\n");
#define OUTPUT_ERROR printf("\t\t[\33[31mERROR\033[0m]\n");

#define DEBUG_SEND_PACKET_DATABASE 1   /*use when debug, if define, send pkt to database, else don't send*/

#define DEBUG_INFO_SHOW 1
#ifdef DEBUG_INFO_SHOW

#define DEBUG_TRANSMIT_PROXY_AFFAIR 1    /*if turn on, show each packet send to database*/
#define DEBUG_TRANSMIT_TELECOM_ENTERPRISE_SERVER 1  /*if turn on, show every packet send to Unicom server*/
#define DEBUG_TRANSMIT_DATABASE_AFFAIR 1    /*if turn on, show each packet send to database*/
#define DEBUG_TRANSMIT_DATABASE_QUERY 1    /*if turn on, show each packet send to database*/
#define DEBUG_TRANSMIT_BANK_AFFAIR 1    /*if turn on, show each packet send to database*/
#define DEBUG_TRANSMIT_PROXY_CONTROL_CHANNEL 1    /*if turn on, show each report status packet from proxy*/

#endif
//#define DEBUG_OBSERVE_SEMPHORE 1 /*request printf the debug infomation of Socket_data_queue*/
//#define DEBUG_WATCHDOG_CTL_CHANNEL 1   /*if turn on, show each chang of the watchdog value (control channel)*/

/*the system internal packet header definations (added by PROXY server)*/
#define COMPANY_INDEX_START_POSITION_AT_HEADER 0
#define COMPANY_INDEX_LENGTH_AT_HEADER 2
#define SERVICE_TYPE_INDEX_START_POSITION_AT_HEADER 2
#define SERVICE_TYPE_INDEX_LENGTH_AT_HEADER 2
#define INTERNAL_PACKET_FLAG_START_POSTION_AT_HEADER 4
#define INTERNAL_PACKET_FLAG_LENGTH_AT_HEADER 2
#define CLIENT_ID_INDEX_START_POSITION_AT_HEADER 6
#define CLIENT_ID_INDEX_LENGTH_AT_HEADER 8
#define EMPLOYEE_ID_START_POSTION_AT_HEADER 14
#define EMPLOYEE_ID_LENGTH_AT_HEADER 4
#define BARGAIN_ID_START_POSTION_AT_HEADER 18
#define BARGAIN_ID_LENGTH_AT_HEADER 30
#define PHONE_NUMBER_START_POSTION_AT_HEADER 48
#define PHONE_NUMBER_LENGTH_AT_HEADER 30
#define CHARGE_MONEY_START_POSITION_AT_HEADER 78
#define CHARGE_MONEY_LENGTH_AT_HEADER 10
#define INTERAL_SUCCESS_FLAG_START_POSITION_AT_HEADER 88
#define INTERAL_SUCCESS_FLAG_LENGTH_AT_HEADER 2
#define INTERAL_ERROR_INFO_START_POSITION_AT_HEADER 90
#define INTERAL_ERROR_INFO_LENGTH_AT_HEADER 30


#define PACKET_HEADER_LENGTH (COMPANY_INDEX_LENGTH_AT_HEADER \
							     +SERVICE_TYPE_INDEX_LENGTH_AT_HEADER \
							     +INTERNAL_PACKET_FLAG_LENGTH_AT_HEADER \
							     +CLIENT_ID_INDEX_LENGTH_AT_HEADER \
							     +EMPLOYEE_ID_LENGTH_AT_HEADER \
							     +BARGAIN_ID_LENGTH_AT_HEADER \
							     +PHONE_NUMBER_LENGTH_AT_HEADER \
							     +CHARGE_MONEY_LENGTH_AT_HEADER\
							     +INTERAL_SUCCESS_FLAG_LENGTH_AT_HEADER\
							     +INTERAL_ERROR_INFO_LENGTH_AT_HEADER)

#define SERIAL_NUMBER_LENGTH 16

struct Report_business_status   /*structure for own machine status*/
{
	int link_num_now;
};

struct Watch_dog_ctl_channel
{
	int counter;
	int ctl_pid;  /*the ctl channel process PID*/
	struct Report_business_status status;
};

/*业务进程的注册表项*/
struct Queue_business_process
{
	int pid;
	unsigned int life_time;
	int step_flag;
	int company_index;
	int service_type_index;
	unsigned long int charge_money;
	char terminal_id[CLIENT_ID_INDEX_LENGTH_AT_HEADER+1];
	char la_serial_number[SERIAL_NUMBER_LENGTH + 1];
	char affair_generate_date[9];
	char affair_generate_time[9];
	char phonenumber[PHONE_NUMBER_LENGTH_AT_HEADER+1];
};

enum db_status
{
	PRIMARY,
	SLAVE,
};

struct DB_status
{
	enum db_status status;
};

int SavePidToArray();
int RegisterInfoToArray(int company_index, int service_type_index, unsigned long int charge_money, char *la_serial_number, char *terminal_id);
int RegisterInfoToArrayEx(int company_index, int service_type_index, unsigned long int charge_money, char *la_serial_number, char *terminal_id, char *phonenumber);
int ModifyInfoStateInArray(int step_flag);
int RecordAbnormalSerialToLog(struct Queue_business_process *process_info);
int DeletePidFromArray();

#endif


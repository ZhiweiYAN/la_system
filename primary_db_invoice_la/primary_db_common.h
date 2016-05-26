/* *************************************************
 * File name:
 * 		primary_db_common.h
 * Description:
 * 		The program is run at the primary database server.
 * 		It receives the data request packet from Slave database.
 * Author:
 * 		Zhiwei Yan, jero.yan@gmail.com
 * Date:
 * 		2007-01-14
 * *************************************************/
#ifndef PRIMARY_DB_COMMON_H
#define PRIMARY_DB_COMMON_H

#include "../config_h_c/config.h"
#include "parse_pkt_common.h"
#include "multi_recvsend.h"
#include "primary_db_error.h"
#include <time.h>
#include <stdio.h>
#include <strings.h>
#include <glog/logging.h>

/* For DEBUG INFORMATION */
/* if number > 5, then display detail information */
#define _PRIMARY_DB_MAIN_DEBUG_  0

#define _PRIMARY_DB_RECORD_DEBUG_  0
#define _DEBUG_RECORD_TEMP_DATABASE_ 0
#define _PRIMARY_DB_VIEW_DEBUG_  0
#define _NUMBER_DEBUG_ 10
#define _PRIMARY_DB_HEART_BEAT_DEBUG_ 0


#define CONFIGFILENAME "../cfg_files/global.cfg"
#define MAXPACKETSIZE_DB 40000



#define SRV_MODE_TYPE_SHARE_ID 74
#define DB_SHARE_ID 76
#define PROCESS_SHARE_ID 78
#define DB_VIEW_SHARE_ID 80

#define MAX_TEMP_SIZE   40000
#define TEMP_LENGTH 4096

#define MAX_QUERY_LENGTH 100000

/* for the common packet header */
#define COMPANY_ID_POSITION 0
#define COMPANY_ID_LENGTH 2

#define SERVICE_ID_POSITION 2
#define SERVICE_ID_LENGTH 2

#define INNER_FLAG_POSITION 4
#define INNER_FLAG_LENGTH 2
#define INVOICE_FLAG "01"

#define TERMINAL_ID_POSITION 6
#define TERMINAL_ID_LENGTH 8

#define WORKER_ID_POSITION 14
#define WORKER_ID_LENGTH 4

#define CONTRACT_ID_POSITION 18
#define CONTRACT_ID_LENGTH 30

#define PHONE_NUMBER_POSITION 48
#define PHONE_NUMBER_LENGTH 30

#define MONEY_POSITION 78
#define MONEY_LENGTH 10

#define INNER_SUCCESS_FLAG_POSITION 88
#define INNER_SUCCESS_FLAG_LENGTH 2

#define ERROR_MEMO_POSITION 90
#define ERROR_MEMO_LENGTH 30
#define INFO_NUMBER_LENGTH 10
#define INFO_FORWARD_PKT_LENGTH 10
#define INFO_BACKWARD_PKT_LENGTH 10

#define PACKET_HEADER_LENGTH (COMPANY_ID_LENGTH+SERVICE_ID_LENGTH+INNER_FLAG_LENGTH+TERMINAL_ID_LENGTH+WORKER_ID_LENGTH+CONTRACT_ID_LENGTH+PHONE_NUMBER_LENGTH+MONEY_LENGTH+INNER_SUCCESS_FLAG_LENGTH+ERROR_MEMO_LENGTH)

#define MAX_SIZE_BUFFER_RECV 40000
//#define MAX_TEMP_SIZE   1024
#define BACKLOG 1024
#define MAX_PROCESS_NUMBRER 1024
/* ** TIME ** */
#define DELAY_MIN_TIME 5
#define DELAY_MID_TIME 10
#define DELAY_MAX_TIME 30
#define PROCESS_DEADLINE  10
#define RECORD_PROCESS_DEADLINE 240
#define CAS_PROCESS_DEADLINE 240
#define PRISYNC_TO_SYNC_TIME 20

#if _PRIMARY_DB_VIEW_DEBUG_ > 5
#define VIEW_PROCESS_DEADLINE 240
#else
#define VIEW_PROCESS_DEADLINE 240
#endif
#define HEART_BEAT_PROCESS_DEADLINE 20
#define PROCESS_LIEF_TIME_INC_MULTIPLY_FACTOR 2

#define SYNC_DEADLINE 	 600
#define SYNC_SLEEP_TIME   10
#define DELAY_MONITOR_TIME 2

#define WATCH_DOG_BACKLOG 5
#define WATCH_DOG_BUFFER_SIZE 50
#define HEART_BEAT_PACKET_SIZE 20

#define HEART_BEAT_BUFFER_SIZE 256
#define HEART_BEAT_BACKLOG 2
#define MIN_PACKET_LENGTH 20

/* Error Information */
#define DB_ERROR 0
#define DB_ERROR_RECORDSQL  20
#define DB_ERROR_POSTGRESQL 30
#define DB_ERROR_NO_RECORDS 31
#define DB_ERROR_NUMBER_THAN_THREE 32
#define DB_ERROR_TERIMAL_THAN_THIRTY 33
#define DB_ERROR_FIXING 34
#define DB_ERROR_MAINTAIN 35
#define DB_ERROR_TWO_RECORDS 36
#define DB_ERROR_ONLY_SELECT 50

#define ERROR_FORK_1 1001
#define ERROR_FORK_2 1002

/* Inner packet flag */
#define OUTER_PACKET_FLAG 00
#define INNER_PACKET_REPEAT_INVOICE_FLAG 01
#define INNER_PACKET_TERMIAL_QUERY_FLAG 02
#define INNER_PACKET_REVERSAL_ASK_FLAG 03


/* Success flag */
#define SUCCESS_FLAG "00"
#define FAILURE_FLAG "01"

/* Reversal limits */
#define TERMIAL_UP_LIMIT 30
#if _NUMBER_DEBUG_ > 5
#define NUMBER_UP_LIMIT	 80000
#else
#define NUMBER_UP_LIMIT 3
#endif

/* Important level */

//enum CompanyName {MOBILE,UNICOM,NETCOM,TELECOM,TIETONG};
enum ServerType {UNKOWN_DB=0,MAIN_DB,SLAVE_DB, INVOICE_DB};
enum ServerMode {ERROR=0,READY,CHECK,MAINTAIN,NORMAL,TWINS,ALONE,SYNC,PRISYNC};
enum ProcessType {NORMAL_PROCESS=0,SYNC_PROCESS,RECORD_PROCESS,VIEW_PROCESS,
                  CAS_PROCESS,HEART_BEAT_SEND_PROCESS,HEART_BEAT_RECV_PROCESS
                 };

struct ShareMemDB {
    void *primary_db_conn;
    void *slave_db_conn;
};

struct ShareMemDBView {
    void *primary_db_conn_view;
};

struct ChildProcessStatus {
    pid_t pid;
    int life_time;
    int deadline;
    enum ProcessType type;
    int process_step;
};

struct ShareMemProcess {
    struct ChildProcessStatus process_table[MAX_PROCESS_NUMBRER];
};

struct ShareMemSrvMode {
    enum ServerMode ownself_mode;
    enum ServerMode brother_mode;
    enum ServerType ownself_type;
};



struct CommonPacketHeader {
    char company_id[COMPANY_ID_LENGTH+1];
    char service_id[SERVICE_ID_LENGTH+1];
    char inner_flag[INNER_FLAG_LENGTH+1];
    char terminal_id[TERMINAL_ID_LENGTH+1];
    char worker_id[WORKER_ID_LENGTH+1];
    char contract_id[CONTRACT_ID_LENGTH+1];
    char phone_number[PHONE_NUMBER_LENGTH+1];
    char money[MONEY_LENGTH+1];
    char inner_success_flag[INNER_SUCCESS_FLAG_LENGTH+1];
    char error_memo[ERROR_MEMO_LENGTH+1];
};

struct CompoundPacketInfo {
    unsigned long int pkt_number ;
    unsigned long int forward_pkt_len ;
    unsigned long int backward_pkt_len ;
};

#endif

/* *************************************************
 * File name:
 * 		primary_db_cas.cc
 * Description:
 * 		The program is run at the database server.
 * 		It do CHECK, ADD and SUB the vitural money of each client.
 * Author:
 * 		Rui Su, ssurui@gmai.com
 *			Xiaodan Ge gexiaodan@mail.xjtu.edu.cn
 *			Zhiwei Yan, jerod.yan@gmail.com
 * Date:
 * 		2007-07-26
 * *************************************************/
#ifndef PRIMARY_DB_CAS
#define PRIMARY_DB_CAS
#include <strings.h>
#include "primary_db_monitor_process.h"
#include "primary_db_sync.h"
#include "primary_db_common.h"
#include <sys/time.h>
#include <time.h>


#define SQL_STRING_LENGTH 500

#define MAX_PACKET_SIZE 1000

//#define _DEBUG_SHOW 1
#ifdef _DEBUG_SHOW
#define _DEBUG_BALANCE_CHECK_ADD_SUBSTRACT_INFO_ 1
#define _DEBUG_FINANCIAL_INFO_ 1
#endif

/*define the structure of packet from business*/
#define CHECK_MONEY_TYPE "00"
#define SUBSTRCT_MONEY_TYPE "01"
#define ADD_MONEY_TYPE "02"
#define VALID_RESPONSE "00"
#define INVALID_RESPONSE "01"
#define TYPE_IN_FORWARD_PACKET_START_POSTION 0
#define TYPE_IN_FORWARD_PACKET_LENGTH 2
#define TERMINAL_ID_IN_FORWARD_PACKET_START_POSTION 2
#define TERMINAL_ID_IN_FORWARD_PACKET_LENGTH 8
#define MONEY_IN_FORWARD_PACKET_START_POSTION 10
#define MONEY_IN_FORWARD_PACKET_LENGTH 10
#define FORWARD_PACKET_LENGTH (TYPE_IN_FORWARD_PACKET_LENGTH + \
								  TERMINAL_ID_IN_FORWARD_PACKET_LENGTH +\
								  MONEY_IN_FORWARD_PACKET_LENGTH)
#define TYPE_IN_BACKWARD_PACKET_START_POSTION 0
#define TYPE_IN_BACKWARD_PACKET_LENGTH 2
#define TERMINAL_ID_IN_BACKWARD_PACKET_START_POSTION 2
#define TERMINAL_ID_IN_BACKWARD_PACKET_LENGTH 8
#define RESPONSE_CODE_IN_BACKWARD_PACKET_START_POSTION 10
#define RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH 2
#define BACKWARD_PACKET_LENGTH (TYPE_IN_BACKWARD_PACKET_LENGTH + \
								   TERMINAL_ID_IN_BACKWARD_PACKET_LENGTH +\
								   RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH)

int Daemon_balance_check_server(int welcome_sd,struct sockaddr_in *sa_business);
int Do_balance_check_procedures(int connection_sd, char *packet, int pkt_len);
int Response_business_query_virtual_money(char *terminal_id, unsigned long long int charge_money, PGconn *conn_db);
int Response_business_subtract_virtual_money(char *terminal_id,char *charge_money, PGconn *conn_db);
int Response_business_add_virtual_money(char *terminal_id,char *charge_money,PGconn *conn_db);
int Response_business_query_real_money(char *terminal_id, unsigned long long int charge_money, PGconn *conn_db_a);
int Response_business_subtract_real_money(char *terminal_id,char *charge_money, PGconn *conn_db);
int Response_business_add_real_money(char *terminal_id,char *charge_money,PGconn *conn_db);
int Response_business_query_real_money_with_bank_card(char *terminal_id,
        unsigned long long int charge_money,
        PGconn *conn_db_a);

#endif


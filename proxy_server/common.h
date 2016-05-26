/*********************************************************
 *project: Line communication charges supermarket
 *filename: common.h
 *version: 1.0
 *purpose: common data structure and operation about them
 *developer: gexiaodan, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-13
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
#include <stdint.h>

/***********************************
 !!!GENERAL INFORMATION!!!
***********************************/
/*define the config file name*/
#define CONFIG_FILENAME "../cfg_files/global.cfg"
#define OUTPUT_OK printf("\t\t[\033[32mOK\033[0m]\n");
#define OUTPUT_ERROR  printf("\t\t[\033[31mERROR\033[0m]\n");

/***********************************
 !!!CONTROL CHANNEL INFORMATION!!!
***********************************/
/*define backlog for control channel*/
#define BACKLOG_CONTROL_CHANNEL 20
/*define database number*/
#define DATABASE_NUMBER 2
/*define maximum packet size that transported in control port between proxy and data base, business servers*/
#define MAX_CONTROL_PACK_SIZE 50
/*define business route flux threshold*/
#define BUSI_FLUX_LOW 1.0
#define BUSI_FLUX_HIGH 1.5
/*define sleep time according to business route flux*/
#define UNITARY_BUSI_SLEEP_TIME 100 /*usleep!! unitary time(us) when one business is down*/
#define ALL_BUSI_DOWN_SLEEP_TIME 5 /*sleep!! time(s) in condition of both business are down*/
/*define life threshold for business control channel process*/
#define CHECK_CTL_INTERVAL 1  /*every ? second check the control channel*/
#define THRESHOLD_CTL_CHAN_DOWN 15  /*if > this threshold, judge control channel is broken*/
/*define ftok id*/
#define SEM_CTL_FTOK_ID 1 /*use this value to make a key_t for semphore of proxy control channel*/
#define SHM_CTL_FTOK_ID 2 /*use this value to make a key_t for share memory of proxy control channel*/

/*enumerate the proxy data channel link status got through control channel with database*/
enum data_channel_link_status
{
	HOLDON=0, /*the data channel link status is hold on*/
	CUTOFF, /*the data channel link status is cut off*/
	ACCOFF, /*the data channel not accept new connection any more*/
	UNKNOWN_STATUS,
};

/*business information*/
typedef struct  
{
	int link_status;/*contain status of business, 1:OK,-1:down, flux control for further use*/
	unsigned int check_cnt;/*counters, in order to check business control channel status*/
	pid_t ctl_pid;/*contain business control pid*/
}business_information;

/*database information*/
typedef struct  
{
	unsigned int check_cnt;/*counters, in order to check database control channel status*/
	pid_t ctl_pid;/*contain database control pid*/
}database_information;

/*proxy control channel information*/
typedef struct  
{
	business_information business_info_array[MAX_BUSINESS_NUMBER];
	database_information database_info_array[DATABASE_NUMBER];
	enum data_channel_link_status data_chan_link_status;
	char busi_to_db_link_ip_address[16]; /*the primary database ip that instructs business to connect*/
}proxy_control_channel;

/***********************************
  !!!DATA CHANNEL INFORMATION!!!
***********************************/
#define MAXPACKETSIZE 40000
#define MAXSLEEP 30

/*define backlog for data channel*/
#define BACKLOG_DATA_CHANNEL 1000
/*define share memory size(sizeof(proxy_data_channel) * NUM_SHARE_DATA_CHAN) for business data channel process*/
#define NUM_SHARE_DATA_CHAN 1000
#define CHECK_BUSI_DATA_INTERVAL 6 /*every ? second check the data channel for proxy server*/
#define THRESHOLD_DATA_CHAN_DOWN 60  /*if > this threshold, judge the data channel is broken*/
/*define ftok id*/
#define SEM_DATA_FTOK_ID 3 /*use this value to make a key_t for semphore of proxy data channel*/
#define SHM_DATA_FTOK_ID 4 /*use this value to make a key_t for share memory of proxy data channel*/

//Added March, 21, 2009
/*use this value to make a key_t for semphore and share memory of proxy terminal control table function*/
#define SEM_TERMINAL_CTRL_TABLE_FTOK_ID 55 
#define SHM_TERMINAL_CTRL_TABLE_FTOK_ID 56
//The maximum ID number of terminal 
#define MAX_TERMINAL_NUM 100000

#define COMPANY_ID_POSITION 0
#define COMPANY_ID_LENGTH 2
#define TERMINAL_ID_POSITION 6
#define TERMINAL_ID_LENGTH 8

#define CTL_PKT_TERMINAL_POSITION 0
#define CTL_PKT_TERMINAL_LENGTH 8
#define CTL_PKT_BUSINESS_POSITION 8
#define CTL_PKT_BUSINESS_LENGTH 3
#define CTL_PKT_COMMAND_POSITION 11
#define CTL_PKT_COMMAND_LENGTH 3

/*information about terminal control table*/
typedef struct
{
	uint64_t enable[MAX_TERMINAL_NUM];
}terminal_control_table;

/*enum packet type in data channel*/
enum data_channel_packet_type
{
	SQL_PACKET=0,
	AFFAIR_PACKET,
	TEMP_PACKET,
	UNKNOWN_PACKET,
};

/*information about data channel, in order to check business data channel status*/
typedef struct
{
	pid_t data_pid;
	unsigned int check_cnt;
	enum data_channel_packet_type data_chan_pack_type;
	unsigned int company_id;
}proxy_data_channel;

/*define packet header information*/
#define INTERNAL_PACKET_FLAG_START_POSTION_AT_HEADER 4
#define INTERNAL_PACKET_FLAG_LENGTH_AT_HEADER 2
#define INTERAL_SUCCESS_FLAG_START_POSITION_AT_HEADER 88
#define INTERAL_SUCCESS_FLAG_LENGTH_AT_HEADER 2
#define INTERAL_ERROR_INFO_START_POSITION_AT_HEADER 90
#define INTERAL_ERROR_INFO_LENGTH_AT_HEADER 30

/*define internal flag*/
#define INTERNAL_FLAG_AFFAIR "00"
#define INTERNAL_FLAG_TEMP "01"
#define INTERNAL_FLAG_SQL "02"
#define INTERNAL_FLAG_LOOP "04"


/***********************************
  !!!DEBUG INFORMATION!!!
***********************************/
/*data channel debug info show*/
#define DEBUG_DATA_CHANNEL_INFO_SHOW 1

#ifdef DEBUG_DATA_CHANNEL_INFO_SHOW
#define DEBUG_TRANSMIT_FROM_CLIENT_DATA 1
#define DEBUG_TRANSMIT_TO_BUSINESS_DATA 1
#define DEBUG_TRANSMIT_FROM_BUSINESS_DATA 1
#define DEBUG_TRANSMIT_TO_CLIENT_DATA 1
#endif

/*control channel debug info show*/
#define DEBUG_CONTROL_CHANNEL_INFO_SHOW 1

#ifdef DEBUG_CONTROL_CHANNEL_INFO_SHOW
#define DEBUG_TRANSMIT_FROM_BUSINESS_CONTROL 1
#define DEBUG_TRANSMIT_TO_BUSINESS_CONTROL 1
#define DEBUG_TRANSMIT_FROM_DATABASE_CONTROL 1
#define DEBUG_TRANSMIT_TO_DATABASE_CONTROL 1
#endif
 
#endif

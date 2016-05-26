/* *************************************************
 * File Name:
 * 		primary_db_heart_beat.cc
 * Description:
 * 		Receiving the couterpart db' heart beat packet,
 * 		and sending itself heart beat packet to the other.
 * Author:
 * 		Yan Zhiwei, jerod.yan@gmail.com (Drum Team)
 * Date:
 * 		2007-01-24
 * *************************************************/
#ifndef PRIMARY_DB_HEART_BEAT_H
#define PRIMARY_DB_HEART_BEAT_H
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
#include <time.h>
#include "primary_db_common.h"
#include "primary_db_monitor_process.h"

int Send_heart_beat_packet_defuncted(char *msg, int msg_length,char *database_ip, int database_port );
int Send_heart_beat_packet(void);
int Receive_heart_beat_packet(void);
int Read_brother_parameters_network(char *ip_addr, int *port);
long int Produce_rand_number(void);

#endif

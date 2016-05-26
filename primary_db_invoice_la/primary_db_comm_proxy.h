/* *************************************************
 * File name:
 * 		primary_db_comm_proxy.h
 * Description:
 * 		The program is run at the primary database server.
 * 		It receives the data request packet from Slave database.
 * Author:
 * 		Zhiwei Yan, jero.yan@gmail.com
 * Date:
 * 		2007-01-14
 * *************************************************/
#ifndef PRIMARY_DB_COMM_PROXY_H
#define PRIMARY_DB_COMM_PROXY_H
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
#include "primary_db_common.h"

int Send_message_to_proxy(char *msg, int msg_length);
int Read_proxy_parameters(char *proxy_addr_array,int *proxy_control_port);
#endif

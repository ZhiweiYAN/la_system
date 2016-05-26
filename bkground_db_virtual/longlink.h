/*********************************************************
 *project: Line communication charges supermarket
 *filename: longlink.h
 *version: 0.4
 *purpose: some function use for LC deamon process 
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-9
 *********************************************************/
#ifndef LONGLINK_H
#define LONGLINK_H

#include "bk_common.h"

PGconn *Connect_db_server(char *user_name, char *password,char *db_name,char *ip_addr);
int connect_server_retry(int sockfd, const struct sockaddr *addr, socklen_t alen);
int Init_comm_server(char *ip_address,int data_port, struct sockaddr_in *sa);

#endif

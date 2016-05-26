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

#include "common.h"

int ConnectServer(char *server_ip_address, int server_port, int sockid);
int CreateSocket(void);
int BindServerPort(int server_port, int sockid);
int Connect_Server_Retry(int sockfd, const struct sockaddr *addr, socklen_t alen);

#endif

#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>

#define MAXPACKETSIZE 400000
#define MAXSLEEP 30
#define BACKLOG_DATA_CHANNEL 400

//int connect_server_retry(int sockfd, const struct sockaddr *addr, socklen_t alen);
int Init_data_comm_server(int client_data_port,struct sockaddr_in *sa);
//int Init_data_comm_client(char *ip_address,int data_port, struct sockaddr_in *sa);

#endif


/*********************************************************
 *project: Line communication charges supermarket
 *filename: recvsend.c
 *version: 1.0
 *purpose: receive and send data
 *developer: gexiaodan, Xi'an Jiaotong University(Drum Team)
 *data: 2007-4-23
 *********************************************************/
#ifndef MULTI_RECVSEND_H
#define MULTI_RECVSEND_H

#include "common.h"

int multi_recv(int socket_fd, char *buf_recv, size_t recv_len, int flags);
int multi_send(int socket_fd, const char *buf_send, size_t send_len, int flags);

#endif

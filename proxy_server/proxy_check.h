/*********************************************************
 *project: Line communication charges supermarket
 *filename: proxy_check.c
 *version: 1.0
 *purpose: prototype of proxy server checker
 *developer: gexiaodan, Xi'an Jiaotong University(Drum Team)
 *data: 2007-1-16
 *********************************************************/
#ifndef PROXY_CHECK_H
#define PROXY_CHECK_H

#include "common.h"
#include "shmsem.h"

int Check_ctl_chan_status(void);
int Check_data_chan_status(void);
int Init_monitor_channel(void);
int Save_data_chan_info(int sem_id, proxy_data_channel *proxy_data_info,enum data_channel_packet_type data_chan_type, char *packet);
int Delete_data_chan_info(int sem_id, proxy_data_channel *proxy_data_info);
int Judge_connection_limit(int sem_id, proxy_data_channel *proxy_data_info, char *packet, enum data_channel_packet_type *data_chan_type);

#endif

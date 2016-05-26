/*********************************************************
 *project: Line communication charges supermarket
 *filename: ctl_channel.h
 *version: 0.4
 *purpose: some function use for control channel process 
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-9
 *********************************************************/
 
#ifndef CTL_CHANNEL_H
#define CTL_CHANNEL_H

#include "common.h"
#include "longlink.h"

int HandleInitByProxy(int sockid, int proxy_id);
int CreateControlChannelProcess(int proxy_id);

#endif

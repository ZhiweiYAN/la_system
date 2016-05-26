/*********************************************************
 *project: Line communication charges supermarket
 *filename: check.h
 *version: 0.4
 *purpose: some function use for check process 
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-12
 *********************************************************/
 
#ifndef CHECK_H
#define CHECK_H

#include "common.h"
#include "ctl_channel.h"

int HandleCheckProcedure();
int CreateCheckProcess();
int CheckControlChannelStateAndReport(struct Report_business_status *status, int proxy_id);
int CheckDataChannelStateAndReport(int semid, struct Queue_business_process *shm_data_chan);
int WriteProcessInfoToFile(const char *processinfo);

#endif

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
int CheckControlChannelStateAndReport(int proxy_id);
int WriteProcessInfoToFile(char *processinfo);

#endif

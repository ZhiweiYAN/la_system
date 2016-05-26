/* *************************************************
 * File name:
 * 		bk_monitor.h
 * Description:
 * 		To avoid the process being blocked, there is a monitor process
 * 		to kill those ones whose life time is too long.
 * Author:
 * 		Yan Zhiwei, jerod.yan@gmail.com  (Drum Team)
 * Date:
 *		2006-01-13
 * *************************************************/
#ifndef BK_MONITOR_H
#define BK_MONITOR_H

#include "bk_common.h"
#include "bk_init.h"

int Insert_pid_process_table(pid_t pid,int deadline,enum ProcessType type);
int Remove_pid_process_table(pid_t pid);
int Register_process_into_process_table(struct ChildProcessStatus *ptr, int prcs_num,pid_t pid,int deadline,enum ProcessType type);
int Unregister_process_from_process_table(struct ChildProcessStatus *ptr, int prcs_num,pid_t pid);
int Increase_process_life_time(struct ChildProcessStatus *ptr, int prcs_num);
int Kill_invalid_process(struct ChildProcessStatus *ptr, int prcs_num);
int Set_process_life_time(pid_t pid, int life_time);

#endif


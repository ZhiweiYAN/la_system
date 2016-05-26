/* *************************************************
 * File name:
 * 		monitor_process.h
 * Description:
 * 		To avoid the process being blocked, there is a monitor process
 * 		to kill those ones whose life time is too long.
 * Author:
 * 		Yan Zhiwei, jerod.yan@gmail.com  (Drum Team)
 * Date:
 *		2006-01-13
 * *************************************************/
#ifndef MONITOR_PROCESS_H
#define MONITOR_PROCESS_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "shmsem.h"
#include "primary_db_common.h"
#include "primary_db_comm_database.h"
#include "primary_db_heart_beat.h"
#include "check.h"

int Insert_pid_process_table(pid_t pid,int deadline,enum ProcessType type);
int Remove_pid_process_table(pid_t pid);
int Register_process_into_process_table(struct ChildProcessStatus *ptr, int prcs_num,pid_t pid,int deadline,enum ProcessType type);
int Unregister_process_from_process_table(struct ChildProcessStatus *ptr, int prcs_num,pid_t pid);
int Increase_process_life_time(struct ChildProcessStatus *ptr, int prcs_num);
int Kill_invalid_process(struct ChildProcessStatus *ptr, int prcs_num);

int Set_process_life_time(pid_t pid, int life_time);

int ReStart_recv_heart_beat_pkt_process(void);
int Restart_send_heart_beat_pkt_process(void);
int Count_record_process_sum(struct ChildProcessStatus *ptr, int prcs_num);
int Increase_half_lifetime_record_process(struct ChildProcessStatus *ptr, int prcs_num);


enum ServerMode Get_ownself_server_mode(void);
int Set_ownself_server_mode(enum ServerMode server_mode);

enum ServerMode Get_brother_server_mode(void);
int Set_brother_server_mode(enum ServerMode server_mode);

enum ServerType Get_ownself_server_type(void);
void Set_ownself_server_type(enum ServerType srv_type);

void Print_ownself_server_type(void);
void Print_ownself_server_mode(void);
void Print_brother_server_mode(void);
void Print_current_date_time(void);

#endif

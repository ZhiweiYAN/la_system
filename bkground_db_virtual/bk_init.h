/* *************************************************
 * File name:
 * 		bk_init.h
 * Description:
 * 		The program is run at the database server.
 * 		It initilizes all child process.
 * 		After that, the bk_server begins to work.
 * Author:
 * 		Zhiwei Yan, jerod.yan@gmail.com
 * Date:
 * 		2007-07-07
 * *************************************************/
#ifndef BK_INIT_H
#define BK_INIT_H

#include "bk_common.h"
#include "update_terminal.h"
#include "shmsem.h"
#include "bk_monitor.h"

int Init_parameters(void);
int Init_finance_control_process(int port,int *welcome_sd, struct sockaddr_in *sa);
int Init_monitor_process(void);
int Init_counter_process(void);
int Init_auto_renew_deposit_process(void);
int Init_auto_renew_balance_process(void);
int Daemon_finance_control_server(int welcome_sd,struct sockaddr_in *sa_business);
int Generate_terminal_manage_view(PGconn *conn_db);
int Genarate_day_report_view(char *day_report_date, PGconn *conn_db);
int Genarate_month_report_view(char *month_report_start_date, char *month_report_end_date, char *table_name, PGconn *conn_db);
int Generate_month_terminal_statistics(char *month_statistics_start_date, char *month_statistics_end_date, PGconn *conn_db);
int Generate_month_worker_statistics(char *month_statistics_start_date, char *month_statistics_end_date, PGconn *conn_db);
int Do_fininace_control_for_updating_all_reckoning_date(PGconn *conn_db_a, char *packet);
int Update_appointed_terminal_reckoning_date(PGconn *conn_db_a, char *terminal_id, char* reckoning_date);
int Do_fininace_control_for_updating_appointed_terminal_reckoning_date(PGconn *conn_db_a, char *packet);
int Do_fininace_control_for_updating_appointed_terminal_deposit(PGconn *conn_db_a, char *packet);
int Do_fininace_control_for_requesting_update_appointed_terminal_deposit(PGconn *conn_db_a, char *packet);
int Do_fininace_control_for_generating_day_report(PGconn *conn_db_a, char *packet);
int Do_fininace_control_for_generating_month_report(PGconn *conn_db_a, char *packet);
int Do_fininace_control_for_generating_month_terminal_statistics(PGconn *conn_db_a, char *packet);
int Do_fininace_control_for_generating_month_worker_statistics(PGconn *conn_db_a, char *packet);
int Do_database_fininace_control_procedures(int connection_sd, char *packet, int pkt_len);

#endif


/*********************************************************
 *project: Line communication charges supermarket
 *filename: update_terminal.h
 *version: 0.1
 *purpose: some function have relationship with update terminal information
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-7-5
 *********************************************************/
 
#ifndef UPDATE_TERMINAL_H
#define UPDATE_TERMINAL_H

#include "bk_common.h"
#include "longlink.h"

int Update_appointed_terminal_total_constant_money(char *terminal_id, 
												   char *start_date,
												   char *end_date, 
												   PGconn *conn_db_a);

int Do_update_total_constant_money(char *terminal_id,
								   unsigned long long int total_constant_money,
								   PGconn *conn_db);

int Query_bank_appointed_terminal_deposit(char *terminal_id, 
										  PGconn *conn_db_a);

int Update_appointed_terminal_bank_deposit(char *terminal_id, PGconn *conn_db, char *deposit);

int Update_manual_appointed_terminal_deposit(char *terminal_id,
											 char *money,
											 PGconn *conn_db);

int Update_appointed_terminal_latest_reckoning_date(char *terminal_id,
													char *latest_reckoning_date,
													PGconn *conn_db);

int Update_all_terminal_latest_reckoning_date(char *latest_reckoning_date, PGconn *conn_db);

int Update_appointed_terminal_apply_refresh_flag(char *terminal_id, int refresh_flag, PGconn *conn_db);


int Update_appointed_terminal_virtual_available_money(char *terminal_id,
													  PGconn *conn_db);

int Update_all_terminal_virtual_available_money(PGconn *conn_db);


#endif



/* *************************************************
 * File Name:
 * 		primary_db_sync.h
 * Brief:
 * 		The two database on two different machine will be synchronized.
 *		The synchronization rule is:
 *			The database that contains less records will be added records,
 *			which are from the database that contains more records.
 * Author:
 * 		Yan Zhiwei, jerod.yan@gmail.com (Drum Team)
 * Date:
 * 		2007-01-08
 * *************************************************/
#ifndef DB_SYNC_H
#define DB_SYNC_H

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "primary_db_common.h"
#include "libpq-fe.h"

#define COMM_LENGTH 256
#define SQL_LENGTH 63
struct db_structure {
    /* PostgreSQL define maximum length as 63 */
    char field_name[SQL_LENGTH];
    char field_type[SQL_LENGTH];
};

int Sync_database_table(PGconn* conn_db_a, PGconn *conn_db_b, char *tb_name);
char * Translate_ftype(Oid ftype_id);
int Sync_databases(PGconn *conn1, PGconn *conn2);
PGconn *Connect_db_server(char *user_name, char *password,char *db_name,char *ip_addr);
int Test_connection_db_server(char *user_name, char *password,char *db_name,char *ip_addr);

int Delete_db_records(PGconn *conn_db,char *tb_name, long long int min_db_id);
int Calibrate_sync_point(PGconn* conn_db_a, PGconn *conn_db_b, char *tb_name);
int Get_record_sum(PGconn *conn_db,char *tb_name, long long int *record_sum);
int Get_min_id(PGconn *conn_db,char *tb_name, long long int *min_id);
int Get_max_id(PGconn *conn_db,char *tb_name, long long int *max_id);
int Set_valid_id(PGconn *conn_db,char *tb_name, long long int cur_id);
#endif

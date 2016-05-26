#ifndef _PRIMSRY_DB_RECORD_H_
#define _PRIMSRY_DB_RECORD_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>

#include  "primary_db_common.h"
#include "parse_pkt_common_header.h"
#include "primary_db_monitor_process.h"
#include "libpq-fe.h"
#include "business_data_analysis.h"
#include "multi_recvsend.h"
#include "primary_db_view.h"

int Do_database_record_procedures(int connection_sd,char *packet,int packet_size);
int Record_pkt_regular_table( char *pkt, int pkt_size,PGconn *conn_db, char* backward_pkt);
int Get_back_pkt_for_business_srv(char *pkt,int pkt_size, char* backward_pkt);


#endif

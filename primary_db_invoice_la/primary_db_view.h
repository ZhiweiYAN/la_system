#ifndef _PRIMSRY_DB_VIEW_H_
#define _PRIMSRY_DB_VIEW_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <assert.h>

#include  "primary_db_common.h"
#include "parse_pkt_common_header.h"
#include "primary_db_monitor_process.h"
#include "libpq-fe.h"
#include "business_data_analysis.h"
#include "multi_recvsend.h"
#include "primary_db_sync.h"
#include "primary_db_record.h"
int Do_db_viewer_procedures(int connection_sd,char *packet,int packet_size);
int Search_pkt_invoice_table(int connection_sd,char *pkt,size_t pkt_size,PGconn* conn_db);
int Query_db_from_client(int connection_sd,char *pkt, size_t pkt_size,PGconn* conn_db);
int Check_forward_packet(int connection_sd,char *pkt, size_t pkt_size,PGconn* conn_db);

#endif

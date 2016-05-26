/*********************************************************
 *project: Line communication charges supermarket
 *filename: database.h
 *version: 0.5
 *purpose: functions about send the pkt to database
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-13
 *********************************************************/
#ifndef DATABASE_H
#define DATABASE_H
 
#include "common.h"
#include "longlink.h"
#include "multi_recvsend.h"

int JudgePrimaryDatabaseServer();
int SendPacketDatabase(char *buf_send, int send_len, char *buf_recv_db, int *recv_db_len);
int SendNonsignificantPacketDatabase(char *buf_send, int send_len, char *buf_recv_db, int *recv_db_len);
int SendPacketInvoiceDatabase(char *buf_send, int send_len, char *buf_recv_db, int *recv_db_len, int use_query_port);

#endif

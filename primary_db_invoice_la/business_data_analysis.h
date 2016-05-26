/*
 * =====================================================================================
 *
 *       Filename:  business_data_analysis.h
 *
 *    Description:  Analyze the packets from business machines
 *    				(分析来自业务机的数据包)
 *
 *        Version:  1.0
 *        Created:  6/14/2010 5:04:36 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhi-wei YAN (Jerod YAN), jerod.yan@gmail.com
 *        Company:  DrumTm
 *
 * =====================================================================================
 */

#ifndef BUSINESS_DATA_ANALYSIS_H
#define BUSINESS_DATA_ANALYSIS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <iconv.h>

#include "primary_db_common.h"
#include "parse_pkt_common_header.h"
#include "primary_db_monitor_process.h"
#include "libpq-fe.h"

int Generate_company_record_from_two_packet(char *fwd_pkt, int fwd_pkt_size, char* back_pkt, int back_pkt_size, char* query_string, int query_string_size);
int Generate_company_record_with_invoice_from_two_packet(char *fwd_pkt, int fwd_pkt_size, char* back_pkt, int back_pkt_size, char* query_string, int query_string_size);

int transcode_from_gb2312_to_utf8(char *bufin, char *bufout);
int Get_company_id(char *pkt, int pkt_size);


int Change_packet_response_code(char *common_packet_header, int packet_size,int respone_code);

int Check_forward_reversal_packet_valid(PGconn* conn_db,char *packet, int packet_size);
int Check_backward_reversal_packet_valid(PGconn* conn_db,char *packet, int packet_size);

int Get_import_level(char *packet);
int Get_inner_pkt_flag(char *packet);
int Set_inner_pkt_flag(char *packet,char *flag);

int Get_compound_pkt_info(char *pkt,int pkt_size, struct CompoundPacketInfo *info);
int Find_clientid_according_serialnumber(PGconn* conn_db, char* packet, int packet_size);
int Get_curr_time(char *curr_time);

int  Fix_invalid_string(unsigned char *str, int str_len, unsigned char *fixed_str);
int  Fix_invalid_string_with_space(unsigned char *str, int str_len);



#endif

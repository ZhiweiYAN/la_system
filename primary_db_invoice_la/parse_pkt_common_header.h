/* *************************************************
 * filename:
 *		parse_pkt_common_header.h
 * author:
 * 		Zhiwei Yan, jerod.yan@gmail.com
 * date:
 * 		2007-04-03
 * *************************************************/
#ifndef 	PARSE_PKT_COMMON_HEADER_H_
#define 	PARSE_PKT_COMMON_HEADER_H_

#include "primary_db_common.h"
int Get_common_header(char *packet, struct CommonPacketHeader *common_pkt_header);
int Get_common_header_company_id(char *packet, char *company_id);
int Get_common_header_service_id(char *packet, char *service_id);
int Get_common_header_inner_flag(char *packet,char *inner_flag);
int Get_common_header_terminal_id(char *packet,char *terminal_id);
int Get_common_header_worker_id(char *packet, char* worker_id);
int Get_common_header_contract_id(char *packet,char* contract_id);
int Get_common_header_phone_number(char *packet,char *phone_number);
int Get_common_header_money(char *packet,char *money);
int Set_common_header_inner_success_id(char *packet,const char *inner_success_id);
int Set_common_header_error_memo(char *packet, const char *error_memo);
#endif

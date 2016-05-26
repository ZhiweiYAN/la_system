/*********************************************************
 *project: Line communication charges supermarket
 *filename: proxy_control.h
 *version: 1.0
 *purpose: prototype of proxy server controller
 *developer: gexiaodan, Xi'an Jiaotong University
 *data: 2007-1-7
 *********************************************************/
#ifndef PROXY_CONTROL_H
#define PROXY_CONTROL_H

#include "common.h"
#include "shmsem.h"
#include "proxy_check.h"

int Do_control_with_business(int sem_id, proxy_control_channel *proxy_control_info, int sock_proxy_control, int position);
int Do_control_with_database(int sem_id, proxy_control_channel *proxy_control_info, int sock_proxy_control, int position);
int Handle_proxy_control(int sock_proxy_control, char business_ip_address_array[][16], char *database_self_ip_address, char *database_brother_ip_address);
int Init_ctl_comm_busi_database_server(int proxy_control_port, char business_ip_address_array[][16], char *database_primary_ip_address, char *database_secondary_ip_address);
int Judge_business_route(int sem_id, proxy_control_channel *proxy_control_info, int *nsleep_time);
enum data_channel_link_status Get_data_channel_link_status(int sem_id, proxy_control_channel *proxy_control_info);
int Query_trans_enable(char *packet);
int Set_terminal_control_table(char *packet);

#endif

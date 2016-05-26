/* ************************************************** 
 * File name:
 * 		proxy_init.h
 * Desciption:
 * 		The file contains the initilizing codes for proxy
 * 		server. It is the first step of all services
 * Coder:
 * 		YanZhiwei, jerod.yan@gmail.com
 * 		Drum Team
 * Date:
 * 		2006-12-26
 * **************************************************/
#ifndef PROXY_INIT_H
#define PROXY_INIT_H

#include "common.h"
#include "proxy_control.h"
#include "proxy_check.h"
#include "multi_recvsend.h"
#include <glog/logging.h>

/*less than INTERAL_ERROR_INFO_LENGTH_AT_HEADER -2(28)*/
#define BUSINESS_ERROR_INFO "所有业务机均不能正常工作"
#define UP_CONNECTION_LIMIT_INFO "系统繁忙，请稍后再试"
#define DATABASE_CHECK_INFO "系统维护中，请稍后再试"
#define ADMISSION_CONTROL_INFO "你的业务受限，请与管理员联系"

int Init_proxy_server(void);
int Init_data_comm_busi_server(const char *business_ip_address,int business_data_port, struct sockaddr_in *sa);
int Daemon_foreground(int sd_to_clients,struct sockaddr_in *sa_clients, struct sockaddr_in *sa_server_array);
int Init_data_comm_client(int client_data_port,struct sockaddr_in *sa);
int connect_server_retry(int sockfd, const struct sockaddr *addr, socklen_t alen);

#endif

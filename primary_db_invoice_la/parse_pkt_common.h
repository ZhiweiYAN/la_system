/*********************************************************
 *project: Line communication charges supermarket
 *filename: parse_pkt_common.h
 *version: 0.1
 *purpose: some common function have relationship with parse packet
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-22
 *********************************************************/
#ifndef PARSE_PKT_COMMON_H
#define PARSE_PKT_COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../config_h_c/config.h"

int FindCompanyIndexFromPacket(char *pkt, int pkt_len, int pkt_postion, int *company_index);
int FindServiceTypeIndexFromPacket(char *pkt, int pkt_len, int pkt_postion, int company_index, int *service_type_index);
int Fill_serial_number_to_packet(char *pkt, int pkt_postion, int company_index, int service_type_index, char *serial_number);
int	Get_money_from_packet(char *pkt, int pkt_postion, int company_index, int service_type_index, char *money);

#endif


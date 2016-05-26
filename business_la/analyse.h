/*********************************************************
 *project: Line communication charges supermarket
 *filename: analyse.h
 *version: 0.4
 *purpose: functions about parse the affair packets
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-13
 *********************************************************/
#ifndef ANALYSE_H
#define ANALYSE_H

#include "common.h"

int GetEnterpriseCodeFromHeader(char *buf_pkt, int len_pkt);
int GetServiceTypeCodeFromHeader(char *buf_pkt, int len_pkt);
unsigned long int GetChargeMoneyFromHeader(char *buf_recv, int recv_len);
int GetClientSerialFromHeader(char *buf_pkt, int len_pkt, char *client_id, int *len_client_id);
int GetInternalPacketFlagFromHeader(char *buf_pkt, int len_pkt);
int GetPhoneNumberFromHeader(char *buf_pkt, int len_pkt, char *phonenumber);
int SetInternalPacketFlagInHeader(char *buf_pkt, int len_pkt, const char *internal_flag);
int ValidRespondCodeInBackwardPacket(char *buf_pkt, int len_pkt, int company_code, int service_type_code);
int ValidInternalSuccessFlagInHeader(char *buf_pkt, int len_pkt);
int CompactOnePacketInDatabasePacket(char *forward_pkt, unsigned int forward_pkt_len);
int CompactTwoPacketInDatabasePacket(char *backward_pkt, unsigned int backward_pkt_len, char *forward_pkt, unsigned int forward_pkt_len);
int WriteInternalSuccessFlag(char *packet, const char* error_code, const char *error_info);
int GetRespondCodeFromPacket(char *pkt, int pkt_postion, int company_index, int service_type_index, char *respond_code, int *respond_code_len);

#endif

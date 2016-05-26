/*********************************************************
 *project: Line communication charges supermarket
 *filename: affair.h
 *version: 0.4
 *purpose: some function have relationship with affair process
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-9
 *********************************************************/
#ifndef AFFAIR_H
#define AFFAIR_H

#include "common.h"
#include "bank.h"
#include "analyse.h"
#include "database.h"
#include "middleware.h"
#include "longlink.h"
#include "serial_number.h"
#include "multi_recvsend.h"

int CreateProcessAcceptClient(int listen_sock, int listen_num);
int TransmitPacketWithDirectLink(char *buf_retransmit, int retran_len, char *buf_feedback, int *feedback, int enterprise_code, int service_type);
int HandleBusinessFromProxy(int sock_proxy);
int HandleChargeBusinessPacket(int sock_proxy, char *pkt_from_proxy);
int HandleReveralBusinessPacket(int sock_proxy, char *pkt_from_proxy);
int HandlePreviousFeeBusinessPacket(int sock_proxy, char *pkt_from_proxy);
int HandleQueryFeeBusinessPacket(int sock_proxy, char *pkt_from_proxy);
int HandleInternalBusinessPacket(int sock_proxy, char *pkt_from_proxy);
int HandleInvoiceQueryBusinessPacket(int sock_proxy, char *pkt_from_proxy);
int HandleLoopChargeBusinessPacket(int sock_proxy, char *pkt_from_proxy);
int HandleLoopReveralBusinessPacket(int sock_proxy, char *pkt_from_proxy);
int HandleTelecomReveralBusinessPacket(int sock_proxy, char *pkt_from_proxy);
int HandleTelecomReveralBusinessPacketNew(int sock_proxy, char *pkt_from_proxy);
int HandleReveralBusinessPacketNew(int sock_proxy, char *pkt_from_proxy);

#endif

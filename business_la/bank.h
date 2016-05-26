/*********************************************************
 *project: Line communication charges supermarket
 *filename: bank.h
 *version: 0.4
 *purpose: functions about bank query and withdraw the money
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-13
 *********************************************************/
#ifndef BANK_H
#define BANK_H

#include "common.h"
#include "longlink.h"
#include "database.h"

int WarrantExcuteAffair(char *client_serial, int client_len, unsigned long int charge_money, int company_index);
int SubtractMoneyFromClientAccount(char *client_serial, int client_len, unsigned long int charge_money, int company_index);
int AddMoneyToClientAccount(char *client_serial, int client_len, unsigned long int reversal_money, int company_index);

int QueryWarrantExcuteAffair(char *client_serial, int client_len, unsigned long int charge_money);
int RequireSubtractMoneyFromClientAccount(char *client_serial, int client_len, unsigned long int charge_money);
int RequireAddMoneyToClientAccount(char *client_serial, int client_len, unsigned long int charge_money);

int SendPacketToAddValueServer(char *pkt, int pkt_len);

#endif

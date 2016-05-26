/*********************************************************
 *project: Line communication charges supermarket
 *filename: serial_number.c
 *version: 0.1
 *purpose: Reconstruct packet
 *developer: gexiaodan, Xi'an Jiaotong University(Drum Team)
 *data: 2007-1-27
 *********************************************************/
#ifndef SERIAL_NUMBER_H
#define SERIAL_NUMBER_H
#include "common.h"

int InitSemShmForSerialNumber(void);
int GenerateSerialNumber(int company_index, char *serial_number);
int FillSerialNumberWithinForwardPacket(char *packet, int company_index, int service_type_index, char *la_serial_number);
int FillSerialNumberToPacket(char *pkt, int pkt_postion, int company_index, int service_type_index, char *serial_number, char *la_serial_number);
int FillRecordDateWithinForwardPacket(char *packet, int company_index, int service_type_index);

#endif


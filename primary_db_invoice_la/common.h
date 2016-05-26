/*********************************************************
 *project: Line communication charges supermarket
 *filename: common.h
 *version: 0.4
 *purpose: common data structure and operation about them
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-9
 *********************************************************/
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <signal.h>
#include "../config_h_c/config.h"
#include "shmsem.h"

#define MAXSLEEP 30  /*the unit is second*/
#define SHORT_SLEEP 100 /*the unit is us, not second*/
#define MAXPACKETSIZE 40000
#define MAX_REPORT_PKT_SIZE 100

/*Here, we describle the behavor of the check process and the control channel process
1. each time, control channel process reset the common counter to 0,
2. each time, check process add a step to a common counter,
and judge the counter, if the counter >T, it means the control channel not work,
so the check process ReCreate a control channel process */
#define REPORT_STATUS_INTERVAL 5     /*every ? second, control channel process reset the common counter*/

#define CHECK_CONTROL_CHANNEL_INTERVAL 5  /*every ? second check the control channel*/
#define THRESHOLD_BROKEN_CONTROL_CHANNEL 60  /*if the count > this Threshold, control channel is brokon*/
#define CHECK_ADD_STEP 5   /*each time, the check process add this value to count*/

#define DELAY_BEFORE_SYNC     30
#define DELAY_AFTER_SYNC      30

#define SEM_CTL_FTOK_ID_BEGIN 10 /*use this value to make a key_t for semphore of proxy machines control*/
#define SHM_CTL_FTOK_ID_BEGIN 11 /*use this value to make a key_t for share memory of proxy machines control*/


struct Watch_dog_ctl_channel {
    int counter;
    int ctl_pid;  /*the ctl channel process PID*/
};

#endif


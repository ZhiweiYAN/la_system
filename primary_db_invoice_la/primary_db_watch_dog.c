/*
 * =====================================================================================
 *
 *       Filename:  primary_db_watch_dog.c
 *
 *    Description:  Watch dog for check-sum program
 *
 *        Version:  1.0
 *        Created:  2012-12-14
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhi-wei YAN (Jerod YAN), jerod.yan@gmail.com
 *        Company:  DrumTm
 *
 * =====================================================================================
 */

#include "primary_db_watch_dog.h"
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Init_db_check_watch_dog
 *  Description:
 * =====================================================================================
 */
int Init_db_check_watch_dog(void)
{
    pid_t pid = 0;
    int success = 0;

//	create a seperate process to monitor the process table.
    if ((pid = fork()) < 0) {
        OUTPUT_ERROR;
        return -1;
    } else if (0 == pid) {

        // enter into the child process, child name=alice, mother=obama
        if ((pid = fork()) < 0) {
            OUTPUT_ERROR;
            return -1;
        } else if (pid > 0) {
            exit(0); /* the child "alice" return to her mother "obama" */
        } else {
            LOG(INFO)<<"WATCHDOG_PROCESS";

            /* Dead Loop which MUST NOT be returned */
            success = Wake_up_check_watch_dog();

            success = Set_ownself_server_mode(ERROR);
            exit(0);
        }
    }

//	obama waits for the return of alice
    if (waitpid(pid,NULL,0)!=pid) {
        OUTPUT_ERROR;
    }
    return 1;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Wake_up_check_watch_dog
 *  Description:
 * =====================================================================================
 */
int Wake_up_check_watch_dog(void)
{

    int success = 0;

    int welcome_sd = 0;
    int connection_sd = 0;
    struct sockaddr_in sa;
    int port = 0;

    socklen_t len = 0;
    char buf_recv[WATCH_DOG_BUFFER_SIZE];
    int count = 0;

    enum ServerType db_type = UNKOWN_DB;

    port = global_par.system_par.database_watchdog_port;
    len = sizeof (sa);

    success = Start_network_service(port,&welcome_sd, &sa," WATCH DOG SERVER " );
    if (-1 == success) {
        return -1;
    }

    while (1) {
        printf("\r\033[34mTHE WATCH DOG HAVE BEEN WAKEN UP NOW.\033[0m\n");
        if (( connection_sd = accept(welcome_sd,(struct sockaddr*)&sa,&len))<0) {
            OUTPUT_ERROR;
            close(connection_sd);
            continue;
        }

        bzero(buf_recv,WATCH_DOG_BUFFER_SIZE);
        count = recv(connection_sd,buf_recv,WATCH_DOG_BUFFER_SIZE,0);

        /* The machine is working so that the string is checked */
        if (count>0) {

            printf("RECV the STRING : %s", buf_recv);
            if (0==strncmp(buf_recv,"CHECK_BEGIN",11)) {
                if (CHECK==Get_ownself_server_mode()) {
                    count = send(connection_sd,"ALREADY_CHECK",13,0);
                    close(connection_sd);
                    continue;
                }

                db_type = Get_ownself_server_type();
                switch (db_type) {
                case MAIN_DB:
                    printf("\r\033[33mTHE WATCH DOG GETS THE MESSAGE CHECKSUM_START!\033[0m\n");

                    while ( READY!=Get_ownself_server_mode() ) {
                        sleep(DELAY_MIN_TIME);
                    }

                    success = Set_ownself_server_mode(CHECK);
                    count = send(connection_sd,"CHECK",5,0);
                    break;
                case SLAVE_DB:
                    count = send(connection_sd,"SLAVE",5,0);
                    break;
                case INVOICE_DB:
                    count = send(connection_sd,"INVCE",5,0);
                    break;
                default:
                    count = send(connection_sd,"UNKWN",5,0);
                    break;
                }
            }

            if (0==strncmp(buf_recv,"CHECK_END",9)) {
                printf("\r\033[33mTHE WATCH DOG GETS THE MESSAGE CHECKSUM_END!\033[0m\n");
                success = Set_ownself_server_mode(READY);
                count = send(connection_sd,"READY",5,0);
            }

            if (0==strncmp(buf_recv,"MAINTAIN_BEGIN",14)) {
                printf("\r\033[33mTHE WATCH DOG GETS THE MESSAGE MAINTAIN_BEGIN!\033[0m\n");
                success = Set_ownself_server_mode(MAINTAIN);
                count = send(connection_sd,"MAINTAIN_BEGIN",14,0);
            }

            if (0==strncmp(buf_recv,"MAINTAIN_END",12)) {
                printf("\r\033[33mTHE WATCH DOG GETS THE MESSAGE MAINTAIN_END!\033[0m\n");
                success = Set_ownself_server_mode(READY);
                count = send(connection_sd,"MAINTAIN_END",12,0);
            }

        }
        close(connection_sd);
    }
    return 1;
}


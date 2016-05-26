/*
 * =====================================================================================
 *
 *       Filename:  primary_db_start.c
 *
 *    Description:  The program will be run on the database servers. It receives the data
 *    				packet from business-servers. Then, it analyzes the accepted data packets
 *                  and records the specific fields of the packets or carries out the SQL quries.
 *        Version:  1.0
 *        Created:  2012-12-28
 *       Revision:  1.0
 *       Compiler:  g++
 *
 *         Author:  Zhi-wei YAN (Jerod YAN), jerod.yan@gmail.com
 *        Company:  DrumTm
 *
 * =====================================================================================
 */

#include "primary_db_start.h"


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Init_db_server(void)
 *  Description:  Initializes the sockets, processes, memory and so on.
 * =====================================================================================
 */
int Init_db_server(void)
{
    int ret  = 0;

    /* for socket connections */
    int welcome_sd_record = 0;                  /* socket for payment packets */
    struct sockaddr_in sa_record;

    int welcome_sd_view = 0;                    /* socket for query packets */
    struct sockaddr_in sa_view;

    int welcome_sd_balance_check = 0;           /* socket for accounting packets */
    struct sockaddr_in sa_balance_check;

    /* pid for the daemon process management */
    pid_t pid_daemon_record = 0;
    pid_t pid_daemon_view = 0;
    pid_t pid_daemon_balance_check = 0;


    LOG(INFO)<<"================ Initialization Step I =====================";
    printf("\n================ Initialization Step I =====================\n");
    /* Initialize SERVER MODE and TYPE semaphore and One block memory*/
    ret  = Init_server_mode_type_share_memory(SRV_MODE_TYPE_SHARE_ID);
    LOG(INFO)<<"Initialize the SERVER MODE TYPE share memory and semaphore:";
    printf("Initialize the SERVER MODE TYPE share memory and semaphore:");
    if (-1 == ret ) {
        LOG(ERROR)<<"[!Failed]";
        OUTPUT_ERROR;
        return -1;
    } else {
        LOG(INFO)<<"[Success!]";
        OUTPUT_OK;
    }

    /* Initialize PROCESS MANGER semaphore and One block memory for process*/
    ret  = Init_process_manager_share_memory(PROCESS_SHARE_ID);
    LOG(INFO)<<"Initialize the PROCESS MANAGMENT share memory and semaphore:";
    printf("Initialize the PROCESS MANAGMENT share memory and semaphore:");
    if (-1 == ret ) {
        LOG(ERROR)<<"[!Failed]";
        OUTPUT_ERROR;
        return -1;
    } else {
        LOG(INFO)<<"[Success!]";
        OUTPUT_OK;
    }

    LOG(INFO)<<"================ Initialization Step II =====================";
    printf("\n================ Initialization Step II =====================\n");
    /* Read parameters from the configuration files. */
    ret  = Setup_config_parameters();
    LOG(INFO)<<"Reading parameters from the config file:";
    printf("Reading parameters from the configfile : ");
    if (-1 == ret ) {
        LOG(ERROR)<<"[!Failed]";
        OUTPUT_ERROR;
        return -1;
    } else {
        LOG(INFO)<<"[Success!]";
        OUTPUT_OK;
    }

    LOG(INFO)<<"================ Initialization Step III =====================";
    printf("\n================ Initialization Step III =====================\n");
    /* Tesing the OWNSELF DB connections */
    ret  = Test_connection_db_server(global_par.system_par.database_user[0],
                                     global_par.system_par.database_password[0],
                                     global_par.system_par.database_name,
                                     global_par.system_par.localhost_ip_address);
    LOG(INFO)<<"Testing OWNSELF DB connections:";
    printf("Testing OWNSELF DB connections:");
    if (-1 == ret ) {
        LOG(ERROR)<<"[!Failed]";
        LOG(ERROR)<<"Detail: "
                  <<global_par.system_par.database_user[0]
                  <<global_par.system_par.database_password[0]
                  <<global_par.system_par.database_name
                  <<global_par.system_par.localhost_ip_address;
        OUTPUT_ERROR;
        return -1;
    } else {
        ret  = Set_ownself_server_mode(READY);
        LOG(INFO)<<"[Success!]";
        OUTPUT_OK;
    }

    LOG(INFO)<<"================ Initialization Step IV =====================";
    printf("\n================ Initialization Step IV =====================\n");
    /* Initialize the three communication socket and port with business servers*/
    /* server socket 1 */
    bzero(&sa_balance_check,sizeof(struct sockaddr_in));
    ret  = Init_balance_check_socket(
               global_par.system_par.business_virtual_money_port,
               &welcome_sd_balance_check,
               &sa_balance_check);
    printf("Initialize the balance communication with business servers :");
    LOG(INFO)<<"Initialize the balance communication with business servers :";
    if (-1 == ret ) {
        LOG(ERROR)<<"[!Failed]";
        OUTPUT_ERROR;
        return -1;
    } else {
        LOG(INFO)<<"[Success!]";
        OUTPUT_OK;
    }
    /* server socket 2 */
    bzero(&sa_record,sizeof(struct sockaddr_in));
    ret  =Init_db_record_socket(
              global_par.system_par.database_data_port,
              &welcome_sd_record,
              &sa_record);
    LOG(INFO)<<"Initialize the payment/record communication with business servers :";
    printf("Initialize the payment/record communication with business servers :");
    if (-1 == ret ) {
        LOG(ERROR)<<"[!Failed]";
        OUTPUT_ERROR;
        return -1;
    } else {
        LOG(INFO)<<"[Success!]";
        OUTPUT_OK;
    }
    /* server socket 3 */
    bzero(&sa_view,sizeof(struct sockaddr_in));
    ret  =Init_db_view_socket(
              global_par.system_par.database_query_port,
              &welcome_sd_view,
              &sa_view);
    LOG(INFO)<<"Initialize the query/view communication with business servers :";
    printf("Initialize the query/view communication with business servers :");
    if (-1 == ret ) {
        LOG(ERROR)<<"[!Failed]";
        OUTPUT_ERROR;
        return -1;
    } else {
        LOG(INFO)<<"[Success!]";
        OUTPUT_OK;
    }

    printf("\n================ Initialization Step V =====================\n");
    LOG(INFO)<<"================ Initialization Step V =====================";
    /* Initialize the wactch dog process for check account*/
    if (MAIN_DB==Get_ownself_server_type()) {
        ret  = Init_db_check_watch_dog();
        printf("Initialize the CheckSum watchdog process:");
        LOG(INFO)<<"Initialize the CheckSum watchdog process:";
        if (-1 == ret ) {
            OUTPUT_ERROR;
            LOG(ERROR)<<"[!Failed]";
            return -1;
        } else {
            LOG(INFO)<<"[Success!]";
            OUTPUT_OK;
        }
    }
    /* Initialize the monitor process*/
    ret  = Start_monitor_process();
    printf("Start the Server State monitor process:");
    LOG(INFO)<<"Start the Server State monitor process:";
    if (-1 == ret ) {
        OUTPUT_ERROR;
        LOG(ERROR)<<"[!Failed]";
        return -1;
    } else {
        LOG(INFO)<<"[Success!]";
        OUTPUT_OK;
    }

    /* Initialize the time counter process - clock*/
    ret  = Start_life_time_counter_process();
    printf("Start the server time counter:");
    LOG(INFO)<<"Start the server time counter:";
    if (-1 == ret ) {
        OUTPUT_ERROR;
        LOG(ERROR)<<"[!Failed]";
        return -1;
    } else {
        LOG(INFO)<<"[Success!]";
        OUTPUT_OK;
    }

    /* Initialize heart beat receiving process between databases */
    if (MAIN_DB==Get_ownself_server_type()) {

        ret  = Start_recv_heart_beat_pkt_process();
        printf("Start heart-beat receiving process between databases:");
        LOG(INFO)<<"Start heart-beat receiving process between databases:";
        if (-1 == ret ) {
            OUTPUT_ERROR;
            LOG(ERROR)<<"[!Failed]";
            return -1;
        } else {
            LOG(INFO)<<"[Success!]";
            OUTPUT_OK;
        }
    }
    if (SLAVE_DB==Get_ownself_server_type()) {
        /* Initialize heart beat sending process between databases */
        ret  = Start_send_heart_beat_pkt_process();
        printf("Start heart-beat sending process between databases:");
        LOG(INFO)<<"Start heart-beat sending process between databases:";
        if (-1 == ret ) {
            OUTPUT_ERROR;
            LOG(ERROR)<<"[!Failed]";
            return -1;
        } else {
            OUTPUT_OK;
            LOG(INFO)<<"[Success!]";
        }
    }
    /*init check process which is in charge of longlink with proxy*/
    if (MAIN_DB==Get_ownself_server_type()) {
        ret  = CreateCheckProcess();
        printf("Start reporting process with proxy machines:");
        LOG(INFO)<<"Start reporting process with proxy machines:";
        if (-1 == ret ) {
            OUTPUT_ERROR;
            LOG(ERROR)<<"[!Failed]";
            return -1;
        } else {
            OUTPUT_OK;
            LOG(INFO)<<"[Success!]";
        }
    }
    printf("\n================ Initialization Step Final =====================\n");
    LOG(INFO)<<"================ Initialization Step Final =====================";

    /* add the CAS(Check_Add_Substract) process */
    if ((pid_daemon_balance_check = fork()) < 0) {
        OUTPUT_ERROR;
        LOG(ERROR)<<"[pid_daemon_balance_check, fork(), !Failed]";
        return -1;
    } else if (0 == pid_daemon_balance_check) {
        ret  = Daemon_balance_check_server(welcome_sd_balance_check,&sa_balance_check);
        if (-1 == ret) {
            LOG(ERROR)<<"[Daemon_balance_check_server, !Failed]";
            exit(0);
        }
    }
    /* Server starts two damon services: payment and query. */
    if ((pid_daemon_view = fork()) < 0) {
        LOG(ERROR)<<"[pid_daemon_view, fork(), !Failed]";
        OUTPUT_ERROR;
        return -1;
    } else if (0 == pid_daemon_view) {
        ret  = Daemon_db_view_server(welcome_sd_view,&sa_view);
        if (-1==ret) {
            LOG(ERROR)<<"[Daemon_balance_check_server, !Failed]";
            exit(0);
        }
    }

    if ((pid_daemon_record = fork()) < 0) {
        LOG(ERROR)<<"[pid_daemon_record, fork(), !Failed]";
        OUTPUT_ERROR;
        return -1;
    } else if (0 == pid_daemon_record) {
        ret  = Daemon_db_record_server(welcome_sd_record,&sa_record);
        if (-1== ret) {
            LOG(ERROR)<<"[Daemon_db_record_server, !Failed]";
            exit(0);
        };
    }

    /* wait for daemon process exit-pid, if one of three exits, ERROR happens. */
    if (waitpid(pid_daemon_balance_check,NULL,0)!=pid_daemon_balance_check) {
        LOG(ERROR)<<"Unexpected function return,pid_daemon_balance_check.";
        OUTPUT_ERROR;
    } else {
        LOG(ERROR)<<"THE DAEMON BALANCE_CHECK PROCESS HAS EXITED NOW";
        printf("\r\033[32mTHE DAEMON BALANCE_CHECK PROCESS HAS EXITED NOW.\033[0m\n");
    }

    if (waitpid(pid_daemon_record,NULL,0)!=pid_daemon_record) {
        LOG(ERROR)<<"Unexpected function return,pid_daemon_record.";
        OUTPUT_ERROR;
    } else {
        LOG(ERROR)<<"THE DAEMON RECORD PROCESS HAS EXITED NOW.";
        printf("\r\033[32mTHE DAEMON RECORD PROCESS HAS EXITED NOW.\033[0m\n");
    }

    if (waitpid(pid_daemon_view,NULL,0)!=pid_daemon_view) {
        LOG(ERROR)<<"Unexpected function return,pid_daemon_view.";
        OUTPUT_ERROR;
    } else {
        LOG(ERROR)<<"THE DAEMON VIEW PROCESS HAS EXITED NOW.";
        printf("\r\033[32mTHE DAEMON VIEW PROCESS HAS EXITED NOW.\033[0m\n");
    }
    return 1;
}


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Daemon_db_record_server
 *  Description:  record the payment packets  into the database.
 *  			  (记录交易数据到数据库中，这是一个守护进程)
 * =====================================================================================
 */
int Daemon_db_record_server(int welcome_sd,struct sockaddr_in *sa_business)
{
    int ret  = 0;

    pid_t pid = 0;

    int connection_sd = 0;
    socklen_t len = 0;
    int count = 0;

    char *buf_recv = NULL;
    char *packet = NULL;

    len = sizeof (struct sockaddr);

    assert(0<welcome_sd);
    assert(NULL!=sa_business);

    /* Enter the Daemon */
    while (1) {
        if (ERROR==Get_ownself_server_mode()) {
            LOG(ERROR)<<"DB Server mode = ERROR, the daemon process is exiting!";
            OUTPUT_ERROR;
            return -1;
        }
        printf("\r\033[32mThe Record Daemon Process is waiting for connections .... \033[0m\n");
        LOG(INFO)<<"The Record Daemon Process is waiting for connections .... ";

        if (( connection_sd = accept(welcome_sd,(struct sockaddr*)sa_business,&len))<0) {
            LOG(ERROR)<< "Error happens when socket function <accept> is running. We will close the socket at once and delay 2 ms and restart.";
            OUTPUT_ERROR;
            close(connection_sd);
            sleep(2);
            continue;
        }

        if (-1==Verify_peer_ip_valid(connection_sd)) {
            LOG(ERROR)<<"Invalid connection from unknown IP addresses. We close the invalid connection.";
            close(connection_sd);
            continue;
        }
//	create a seperate process to monitor the process table.
        if ((pid = fork()) < 0) {
            LOG(ERROR)<<"1st fork() db record, failed";
            OUTPUT_ERROR;
            close(connection_sd);
            return -1;
        } else if (0 == pid) {

//		enter into the child process, child name=alice, mother=obama
            /* Close the listening socket description */
            close(welcome_sd);

            if ((pid = fork()) < 0) {
                LOG(ERROR)<<"2nd fork() db record, failed";
                OUTPUT_ERROR;
                return -1;
            } else if (pid > 0)
                exit(0); /* the child "alice" return to her mother "obama" */


//			enter into the grandchild, child name=jose, mother=alice

            /* In the grandchild process */
            /* Allocate memory for receiving data */
            ret  = Insert_pid_process_table(getpid(),RECORD_PROCESS_DEADLINE,RECORD_PROCESS);
            buf_recv = (char*)malloc(sizeof(char)*MAX_SIZE_BUFFER_RECV);
            if (NULL==buf_recv) {
                LOG(ERROR)<<"malloc buf_recv, failed.";
                OUTPUT_ERROR;
                goto END;
            } else {
                bzero(buf_recv,MAX_SIZE_BUFFER_RECV);
            }

            /* BUSINESS:Receiving data from business servers*/
            count = multi_recv(connection_sd,buf_recv,MAX_SIZE_BUFFER_RECV,0);
            LOG(INFO)<<"Record: Recv data from Business Server";
            LOG(INFO)<<"Data Len:"<<count<<"\nData String:|"<<buf_recv<<"|";
            DBG("\n%s:|%s|\n","Record: Recv data from BusiSrv",buf_recv);


            /* Prepare the actual memory for the packet */
            packet = (char *)malloc(sizeof(char)*(count+1));
            if (NULL == packet) {
                OUTPUT_ERROR;
                LOG(ERROR)<<"malloc packet, failed.";
                goto END;
            } else {
                bzero(packet,count+1);
                memcpy(packet,buf_recv,count);
            }

            /* Deal with the packet in the following function, there is a function
             * to send the result to the business machines. */
            /* (在下面函数中，有一个函数是将处理后的包回给业务机) */
            ret  = Do_database_record_procedures(connection_sd,packet,count);

END:
            close(connection_sd);
            ret  = Remove_pid_process_table(getpid());

            free(buf_recv);
            buf_recv = NULL;
            free(packet);
            packet = NULL;

            exit(0);
        }

        /* Close the connection socket description in the parent obama  process*/
        close(connection_sd);

        /* In the parent process */
        if (waitpid(pid,NULL,0)!=pid) {
            OUTPUT_ERROR;
        }
        continue;
    }
    return 1;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Daemon_db_view_server
 *  Description:  deal with the request packet
 *  			  (处理查询包)
 * =====================================================================================
 */
int Daemon_db_view_server(int welcome_sd,struct sockaddr_in *sa_business)
{
    int ret  = 0;

    pid_t pid = 0;

    int connection_sd = 0;
    socklen_t len = 0;
    int count = 0;

    char *buf_recv = NULL;
    char *packet = NULL;

    len = sizeof (struct sockaddr);

    assert(0<welcome_sd);
    assert(NULL!=sa_business);

    /* Enter the Daemon */
    while (1) {
        if (ERROR==Get_ownself_server_mode()) {
            LOG(ERROR)<<"Server Mode = Error, Daemon_db_view_server is exiting.";
            OUTPUT_ERROR;
            return -1;
        }
        fflush(NULL);
        printf("\r\033[34mThe View Daemon Process is waiting for connections .... \033[0m\n");
        if (( connection_sd = accept(welcome_sd,(struct sockaddr*)sa_business,&len))<0) {
            LOG(ERROR)<<"Daemon_db_view_server, accept failed. We close the connection and delay 2ms";
            OUTPUT_ERROR;
            close(connection_sd);
            sleep(2);
            continue;
        }

        fflush(NULL);
        if ((pid = fork()) < 0) {
            LOG(ERROR)<<"1st fork(), failed.";
            OUTPUT_ERROR;
            return -1;
        } else if (0 == pid) {

            /* In the first child process*/
            /* Close the listening socket description */
            close(welcome_sd);

            if ((pid = fork()) < 0) {
                LOG(ERROR)<<"2nd fork(), failed.";
                OUTPUT_ERROR;
            } else if (pid > 0)
                exit(0); 		/* Return original parent */

            /* In the grandchild process */
            /* Allocate memory for receiving data */
            ret  = Insert_pid_process_table(getpid(),VIEW_PROCESS_DEADLINE,VIEW_PROCESS);
            buf_recv = (char*)malloc(sizeof(char)*MAX_SIZE_BUFFER_RECV);
            if (NULL==buf_recv) {
                LOG(ERROR)<<"malloc, buf_recv, failed.";
                OUTPUT_ERROR;
                goto END;
            } else {
                bzero(buf_recv,MAX_SIZE_BUFFER_RECV);
            }

            /* BUSINESS:Receiving data from business servers*/
            count = multi_recv(connection_sd,buf_recv,MAX_SIZE_BUFFER_RECV,0);
            DBG("\n%s |%s|", "View: Recv data from BusiSrv", buf_recv);

            LOG(INFO)<<"Record: Recv data from Business Server";
            LOG(INFO)<<"Data Len:"<<count<<"\nData String:|"<<buf_recv<<"|";

            /* Prepare the actual memory for the packet */
            packet = (char *)malloc(sizeof(char)*(count+1));
            if (NULL == packet) {
                LOG(ERROR)<<"malloc, packet, failed.";
                OUTPUT_ERROR;
                goto END;
            } else {
                bzero(packet,count+1);
                memcpy(packet,buf_recv,count);
            }

            /* Database Procedures */
            ret  = Do_db_viewer_procedures(connection_sd,packet,count);

            /* Free all resources */
END:
            close(connection_sd);
            ret  = Remove_pid_process_table(getpid());

            free(buf_recv);
            buf_recv = NULL;
            free(packet);
            packet = NULL;

            exit(0);
        }

        /* Close the connection socket description in the parent process*/
        close(connection_sd);
        /* In the parent process */
        if (waitpid(pid,NULL,0)!=pid) {
            OUTPUT_ERROR;
        }
    }

}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Verify_peer_ip_valid
 *  Description:  Verify the connections from invalid addresses.
 * =====================================================================================
 */
int Verify_peer_ip_valid(int sockfd)
{
    int ret = 0;
    int i = 0;
    char peeraddrstr[MAX_TEMP_SIZE];

    struct sockaddr_in peer;
    socklen_t len;
    bzero(peeraddrstr,MAX_TEMP_SIZE);
    ret = getpeername(sockfd, (struct sockaddr *)&peer, &len);
    if (ret < 0)
        return -1;

    sprintf(peeraddrstr, "%s", inet_ntoa(peer.sin_addr));

    //Verify the ip of peer with Global parameter, business server IPs
    for (i=0; i<global_par.system_par.business_number; i++) {
        if (0==strcmp(global_par.system_par.business_ip_addr_array[i],
                      peeraddrstr)) {
            ret = 0;
            return ret;
        } else {
            LOG(WARNING)<<"Invalid Connection from the illegal IP address:" <<peeraddrstr;
            OUTPUT_ERROR;
            printf("Invalid IP address: %s.\n", peeraddrstr);
            //LOG_PACKET_STRING("Illegal client hacking, IP address: ");
            //LOG_PACKET_STRING(peeraddrstr);
            ret = -1;
        }

    }
    return ret;
}



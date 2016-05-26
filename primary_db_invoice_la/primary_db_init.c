/*
 * =====================================================================================
 *
 *       Filename:  primary_db_init.c
 *
 *    Description:  The program is running at the database server. It initilizes all
 *    				child-processes.
 *        Version:  1.0
 *        Created:  6/7/2010 12:26:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhi-wei YAN (Jerod YAN), jerod.yan@gmail.com
 *        Company:  DrumTm
 *
 * =====================================================================================
 */
#include "primary_db_init.h"

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Setup_config_parameters
 *  Description:  read setup-parameters from the specific configuration file and save
 *  			  them into the global variables.
 * =====================================================================================
 */
int Setup_config_parameters(void)
{
    char config_fname[FILE_NAME_LENGTH] = CONFIGFILENAME;
    int success = 0;

    success = ReadConfigAll(config_fname);

    if (-1 == success) {
        OUTPUT_ERROR;
        LOG(ERROR)<<"Reading config file"<<config_fname<<",ERROR.";
        exit(0);
    }

    /*  judge which database type is set according to the computer IP address. */
    if (0==strcmp(global_par.system_par.database_self_ip_address,
                  global_par.system_par.localhost_ip_address)) {
        Set_ownself_server_type(MAIN_DB);
    }
    if (0==strcmp(global_par.system_par.database_brother_ip_address,
                  global_par.system_par.localhost_ip_address)) {
        Set_ownself_server_type(SLAVE_DB);
    }
    if (0==strcmp(global_par.system_par.database_invoice_ip_address,
                  global_par.system_par.localhost_ip_address)) {
        Set_ownself_server_type(INVOICE_DB);
    }


    return success;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Start_network_service
 *  Description:  prepare the network connections as a server such as binding port and
 *  			  listening port.
 * =====================================================================================
 */
int Start_network_service(int port,int *welcome_sd, struct sockaddr_in *sa, const char* s)
{
    int reuse =1;
    /* Check input parameters */
    if (1024>port||NULL==welcome_sd||NULL==sa) {
        OUTPUT_ERROR;
        LOG(ERROR)<<"input parameters.";
        return -1;
    }

    /* Create a socket */
    if ((*welcome_sd=socket(AF_INET,SOCK_STREAM,0))<0) {
        LOG(ERROR)<<"socket error.";
        OUTPUT_ERROR;
        return -1;
    }

    /* Bind the socket to the port, and allow all IP connect with it */
    bzero(sa,sizeof(struct sockaddr_in));
    sa->sin_family = AF_INET;
    sa->sin_port = htons(port);
    sa->sin_addr.s_addr = htonl(INADDR_ANY);

    /* Elimitate the message "Address already in use" */

    if (setsockopt(*welcome_sd,SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof(int))<0) {
        LOG(ERROR)<<"setsockopt,  error, IP address already in use.";
        OUTPUT_ERROR;
    }
    do {
        if (0==bind(*welcome_sd,(struct sockaddr *)sa,sizeof(struct sockaddr_in))) {
            /* success for bind operation */
            break;
        } else {
            printf("\nDatabase program attempts to bind %s port for business machines after 5 seconds.\n",s);
            sleep(5);
            continue;
        }
    } while (1);

    LOG(INFO)<<" Listen port: [" << port<< "] has been opened, "<<s;
    listen(*welcome_sd,BACKLOG);
    return 1;

}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Init_balance_check_socket
 *  Description:  Initialize the check socket for check service.
 * =====================================================================================
 */
int Init_balance_check_socket(int port,int *welcome_sd, struct sockaddr_in *sa)
{
    int res = -1;
    res = Start_network_service(port, welcome_sd, sa, " BALANCE CHECK PROCESS ");
    return res;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Init_db_record_socket
 *  Description:  Initialize the payment socket as record service.
 * =====================================================================================
 */
int Init_db_record_socket(int port,int *welcome_sd, struct sockaddr_in *sa)
{
    int res = -1;
    res = Start_network_service(port, welcome_sd, sa, " RECORD PROCESS ");
    return res;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Init_db_view_socket
 *  Description:  Initialize the inquire socket as view service
 * =====================================================================================
 */
int Init_db_view_socket(int port,int *welcome_sd, struct sockaddr_in *sa)
{
    int res = -1;
    res = Start_network_service(port, welcome_sd, sa, " VIEW PROCESS ");
    return res;
}


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Init_server_mode_share_memory
 *  Description:  Initialize a share memory for saving server mode(TWIN,ALONE,READY,etc)
 * =====================================================================================
 */
int Init_server_mode_type_share_memory(int share_id)
{
    int res = -1;
    int semid = 0;
    struct ShareMemSrvMode *mem_ptr = NULL;

    res = InitialShmSem(sizeof(struct ShareMemSrvMode),share_id);
    if (-1 == res) {
        return -1;
    }

    /* Set default values for the ShareMemSRV structure */
    semid = GetExistedSemphore(share_id);
    res = AcquireAccessRight(semid);
    mem_ptr = (struct ShareMemSrvMode *) MappingShareMemOwnSpace(share_id);

    bzero(mem_ptr,sizeof(struct ShareMemSrvMode));
    mem_ptr->ownself_mode = ERROR;
    mem_ptr->brother_mode = ERROR;
    mem_ptr->ownself_type = UNKOWN_DB;

    res = UnmappingShareMem((void*)mem_ptr);
    res = ReleaseAccessRight(semid);

    return res;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Init_process_manager_share_memory
 *  Description:  Initialize a share memory for saving registed processes info
 *  			  into the table.
 * =====================================================================================
 */
int Init_process_manager_share_memory(int share_id)
{
    int res = -1;
    int semid = 0;
    struct ShareMemProcess *mem_ptr = NULL;

    res = InitialShmSem(sizeof(struct ShareMemProcess),share_id);
    if (-1 == res) {
        return -1;
    }
    /* Set default values for the ShareMemProcess structure */
    semid = GetExistedSemphore(share_id);
    res = AcquireAccessRight(semid);
    mem_ptr = (struct ShareMemProcess *) MappingShareMemOwnSpace(share_id);

    bzero(mem_ptr->process_table,sizeof(struct ChildProcessStatus)*MAX_PROCESS_NUMBRER);

    res = UnmappingShareMem((void*)mem_ptr);
    res = ReleaseAccessRight(semid);

    return res;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Start_monitor_process
 *  Description:  Start the seperate monitor process, which take care of the zombie
 *  			  process in the process table.
 * =====================================================================================
 */
int Start_monitor_process(void)
{
    pid_t pid = 0;
    void * mem_process_ptr = NULL;

    int success = 0;
    int semid_process = 0;
    int kill_flag = 0;

//	create a seperate process to monitor the process table.
    if ((pid = fork()) < 0) {
        OUTPUT_ERROR;
        return -1;
    } else if (0 == pid) {

//		enter into the child process, child name=alice, mother=obama
        if ((pid = fork()) < 0) {
            OUTPUT_ERROR;
            return -1;
        } else if (pid > 0) {
            exit(0); /* the child "alice" return to her mother "obama" */
        } else {

//			enter into the grandchild, child name=jose, mother=alice
            LOG(INFO) <<"MONITOR_PROCESS";

            while (1) {

                /* Kill or clear all invalid processes */
                semid_process = GetExistedSemphore(PROCESS_SHARE_ID);
                success = AcquireAccessRight(semid_process);
                mem_process_ptr = MappingShareMemOwnSpace(PROCESS_SHARE_ID);

                kill_flag = Kill_invalid_process(
                                ((struct ShareMemProcess *)mem_process_ptr)->process_table,
                                MAX_PROCESS_NUMBRER);

                success = UnmappingShareMem((void*)mem_process_ptr);
                success = ReleaseAccessRight(semid_process);
                Print_ownself_server_type();
                Print_ownself_server_mode();
                if (MAIN_DB==Get_ownself_server_type() ||SLAVE_DB==Get_ownself_server_type()) {
                    Print_brother_server_mode();
                }
                Print_current_date_time();


                sleep(DELAY_MONITOR_TIME);
            }
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
 *         Name:  Start_life_time_counter_process
 *  Description:  Add the life time by one second, which is the item of every process
 *  			  in the process table.
 * =====================================================================================
 */
int Start_life_time_counter_process(void)
{
    pid_t pid = 0;
    void * mem_ptr = NULL;
    int success = 0;
    int semid = 0;

//	create a seperate process to monitor the process table.
    if ((pid = fork()) < 0) {
        OUTPUT_ERROR;
        return -1;
    } else if (0 == pid) {

//		enter into the child process, child name=alice, mother=obama
        if ((pid = fork()) < 0) {
            OUTPUT_ERROR;
            return -1;
        } else if (pid > 0) {
            exit(0); /* the child "alice" return to her mother "obama" */
        } else {
//			enter into the grandchild, child name=jose, mother=alice
            while (1) {
                semid = GetExistedSemphore(PROCESS_SHARE_ID);
                success = AcquireAccessRight(semid);
                mem_ptr = MappingShareMemOwnSpace(PROCESS_SHARE_ID);

                /* increasing the lifetime of the processes. */
                success = Increase_process_life_time(((struct ShareMemProcess *)mem_ptr)->process_table,MAX_PROCESS_NUMBRER);

                success = UnmappingShareMem((void*)mem_ptr);
                success = ReleaseAccessRight(semid);

                sleep(1);
            }
        }
    }

//	obama waits for the return of alice
    if (waitpid(pid,NULL,0)!=pid) {

        OUTPUT_ERROR;
    }

    return 1;

}
/* *************************************************
 * Function Name:
 * 		int Init_comm_db_process(void)
 * Input:
 * 		NONE;
 * Ouput:
 * 		1 ---> success
 * 		-1 ---> failure
 * *************************************************/
int Start_recv_heart_beat_pkt_process(void)
{
    pid_t pid = 0;
    int success = 0;
//	create a seperate process to monitor the process table.
    if ((pid = fork()) < 0) {
        OUTPUT_ERROR;
        return -1;
    } else if (0 == pid) {

//		enter into the child process, child name=alice, mother=obama
        if ((pid = fork()) < 0) {
            OUTPUT_ERROR;
            return -1;
        } else if (pid > 0) {
            exit(0); /* the child "alice" return to her mother "obama" */
        } else {
//			enter into the grandchild, child name=jose, mother=alice
            LOG(INFO) <<"RECVBH_PROCESS";
            /* Register process into the process table */
            success = Insert_pid_process_table(getpid(),HEART_BEAT_PROCESS_DEADLINE,HEART_BEAT_RECV_PROCESS);
            success = Receive_heart_beat_packet();
        }
    }

    if (waitpid(pid,NULL,0)!=pid)
        perror("primary_db_la.cc:Init_comm_db_process:waitpid");
    return 1;
}
/* *************************************************
 * Function Name:
 * 		int Start_send_heart_beat_pkt_process(void)
 * Input:
 * 		NONE;
 * Ouput:
 * 		1 ---> success
 * 		-1 ---> failure
 * *************************************************/
int Start_send_heart_beat_pkt_process(void)
{
    pid_t pid = 0;
    int success = 0;
    if ((pid = fork()) < 0) {
        perror("error:primary_db_la.cc:Start_send_heart_beat_pkt_process():fork");
        return -1;
    } else if (0 == pid) {
        /* In the first child process*/
        if ((pid = fork()) < 0)
            perror("error:primary_db_la.cc:Start_send_heart_beat_pkt_process():fork2");
        else if (pid > 0)
            exit(0); 		/* Return original parent */
        else {
            LOG(INFO) <<"SENDBH_PROCESS";
            success = Insert_pid_process_table(getpid(),HEART_BEAT_PROCESS_DEADLINE,HEART_BEAT_SEND_PROCESS);
            success = Send_heart_beat_packet();
        }
    }

    if (waitpid(pid,NULL,0)!=pid)
        perror("primary_db_la.cc:Start_send_heart_beat_pkt_process:waitpid");
    return 1;
}


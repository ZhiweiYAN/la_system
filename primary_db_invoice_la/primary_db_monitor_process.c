/*
 * =====================================================================================
 *
 *       Filename:  primary_db_monitor_process.c
 *
 *    Description: 	To avoid the process being blocked, there is a monitor process
 * 					to kill those ones whose life time is too long.
 *        Version:  1.0
 *        Created:  6/12/2010 9:18:31 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhi-wei YAN (Jerod YAN), jerod.yan@gmail.com
 *        Company:  DrumTm
 *
 * =====================================================================================
 */

#include "primary_db_monitor_process.h"
/* *************************************************
 * Function Name:
 * 		int Insert_pid_process_table(pid_t pid,int deadline,enum ProcessType type)
 * Input:
 * 		pid_t pid;
 * 		int deadline;
 * 		processType type;
 * Ouput:
 * 		1 ---> success
 * 		-1 ---> failure
 * *************************************************/
int Insert_pid_process_table(pid_t pid,int deadline,enum ProcessType type)
{
    void * mem_ptr = NULL;
    int success = 0;
    int semid = 0;
    semid = GetExistedSemphore(PROCESS_SHARE_ID);
    success = AcquireAccessRight(semid);
    mem_ptr = MappingShareMemOwnSpace(PROCESS_SHARE_ID);

    /*  insert the pid of the process into the table */
    success = Register_process_into_process_table(((struct ShareMemProcess *)mem_ptr)->process_table,MAX_PROCESS_NUMBRER,pid,deadline,type);

    /* Free memory control handler */
    success = UnmappingShareMem((void*)mem_ptr);
    success = ReleaseAccessRight(semid);

    return success;
}


/* *************************************************
 * Function Name:
 * 		int int Remove_pid_process_table(pid_t pid)
 * Input:
 * 		pid_t pid;
 * Ouput:
 * 		1 ---> success
 * 		-1 ---> failure
 * *************************************************/
int Remove_pid_process_table(pid_t pid)
{
    void * mem_ptr = NULL;
    int success = 0;
    int semid = 0;
    semid = GetExistedSemphore(PROCESS_SHARE_ID);
    success = AcquireAccessRight(semid);
    mem_ptr = MappingShareMemOwnSpace(PROCESS_SHARE_ID);

    /*  remove the pid of the process into the table */
    success = Unregister_process_from_process_table(((struct ShareMemProcess *)mem_ptr)->process_table,MAX_PROCESS_NUMBRER,pid);

    /* Free memory control handler */
    success = UnmappingShareMem((void*)mem_ptr);
    success = ReleaseAccessRight(semid);

    return success;
}

/* *************************************************
 * Function Name:
 * 		int Set_process_life_time(pid_t pid,int life_time)
 * Input:
 * 		pid_t pid;
 * 		int life_time;
 * Ouput:
 * 		1 ---> success
 * 		-1 ---> failure
 * *************************************************/
int Set_process_life_time(pid_t pid, int life_time)
{
    struct ShareMemProcess * mem_ptr = NULL;
    struct ChildProcessStatus *process_ptr = NULL;
    int success = 0;
    int i = 0;
    int semid = 0;
    semid = GetExistedSemphore(PROCESS_SHARE_ID);
    success = AcquireAccessRight(semid);
    mem_ptr = (struct ShareMemProcess *)MappingShareMemOwnSpace(PROCESS_SHARE_ID);
    process_ptr = (struct ChildProcessStatus * )(mem_ptr->process_table);

    /*  set the life time of the process table */
    /* Place the pid into the table */
    for (i=0; i<MAX_PROCESS_NUMBRER; i++) {
        if (pid==(process_ptr+i)->pid) {
            (process_ptr+i)->life_time = life_time;
            break;
        }
    }
    /* Free memory control handler */
    success = UnmappingShareMem((void*)mem_ptr);
    success = ReleaseAccessRight(semid);

    return success;
}

/* *************************************************
 * Function Name:
 * 		int Register_process_into_process_table(struct ChildProcessStatus *ptr, int prcs_num,pid_t pid,int deadline, enum ProcessType type)
 * Input:
 * 		struct ChildProcessStatus *ptr ---> table of life time of processes
 * 		int prcs_num ---> the sum of all child processes
 * Output:
 * 		1 ---> success;
 * 		-1 ---> failure
 * *************************************************/
int Register_process_into_process_table(struct ChildProcessStatus *ptr, int prcs_num,pid_t pid,int deadline,enum ProcessType type)
{
    int i = 0;
    int j = 0;

    int available_slot_sum = 0;
    /* Check the input parameters */
    if (NULL==ptr||0>prcs_num||0>deadline) {
        perror("error@monitor_process.cc:Add_process_into_time_table():NULL==ptr");
        return -1;
    }

    /* Place the pid into the table */
    for (i=0; i<prcs_num; i++) {
        if (0==(ptr+i)->pid) {
            (ptr+i)->pid = pid;
            (ptr+i)->life_time = 0;
            (ptr+i)->deadline = deadline;
            (ptr+i)->type = type;
            (ptr+i)->process_step = 0;
            //if(RECORD_PROCESS==type||SYNC_PROCESS==type)
            //printf("\r\033[33mRegister PID %d is OK in slot %d\033[0m. \n",pid,i);
            break;
            fflush(NULL);
        }
    }
    for (j=0; j<prcs_num; j++){
        if(0==(ptr+j)->pid){
            available_slot_sum ++;
        }
    }
    printf("\r\033[33mRegister PID %d into slot %d, ava_slot_ratio = %d/%d \033[0m. \n",pid,i, available_slot_sum, prcs_num);
    return 1;
}

int Unregister_process_from_process_table(struct ChildProcessStatus *ptr, int prcs_num,pid_t pid)
{
    int i = 0;
    int j = 0;

    int available_slot_sum = 0;
    /* Check the input parameters */
    if (NULL==ptr||0>prcs_num) {
        perror("error@monitor_process.cc:Add_process_into_time_table():NULL==ptr");
        return -1;
    }

    /* Remove the pid into the table */
    for (i=0; i<prcs_num; i++) {
        if (pid==(ptr+i)->pid) {
            (ptr+i)->pid = 0;
            (ptr+i)->life_time = 0;
            (ptr+i)->deadline = 0;
            (ptr+i)->type = NORMAL_PROCESS;
            (ptr+i)->process_step = 0;
            //printf("\r\033[36mUnRegister PID %d is OK from slot %d.\033[0m. \n",pid,i);
            break;
            fflush(NULL);
        }
    }

    for (j=0; j<prcs_num; j++){
        if(0==(ptr+j)->pid){
            available_slot_sum ++;
        }
    }
    printf("\r\033[36mUnRegister PID %d from slot %d, ava_slot_ratio = %d/%d \033[0m. \n",pid,i, available_slot_sum, prcs_num);

    return 1;
}

/* *************************************************
 * Function Name:
 * 		int Increase_process_life_time(struct ChildProcessStatus *ptr, int prcs_num)
 * Input:
 * 		struct ChildProcessStatus *ptr ---> the pointer of the process table
 * 		int prcs_num ---> the sum of the process
 * Ouput:
 * 		1 ---> success
 * 		-1 ---> failure
 * *************************************************/
int Increase_process_life_time(struct ChildProcessStatus *ptr, int prcs_num)
{
    int i = 0;
    /* Check the input parameters */
    if (NULL==ptr||0>=prcs_num) {
        perror("error@mointor_process.cc:Increase_process_life_time():NULL==ptr");
        return -1;
    }

    for (i=0; i<prcs_num; i++) {
        if (0!= (ptr+i)->pid)
            ((ptr+i)->life_time)++;
    }
    return 1;
}

/* *************************************************
 * Function Name:
 * 		int Kill_invalid_process(struct ChildProcessStatus *ptr, int prcs_num, int deadline)
 * Input:
 * 		struct ChildProcessStatus *ptr ---> all the child process in the table
 * 		int prcs_num   --->  the sum of all child processes
 * Output:
 * 		1 ---> success;
 * 		-1 ---> failure
 * *************************************************/
int Kill_invalid_process(struct ChildProcessStatus *ptr, int prcs_num)
{
    int i = 0;
    int success = 0;
    int flag_sync = 0;
    int flag_record = 0;
    int flag_send_hb = 0;
    int flag_recv_hb = 0;

    /* Check the input parameters */
    if (NULL==ptr||0>=prcs_num) {
        perror("error@mointor_process.cc:Kill_invalid_process():NULL==ptr");
        return -1;
    }

    /* Kill all process that exceed their deadlines. */
    for (i=0; i<prcs_num; i++) {
        /* if the process belongs to the BEART HEART process */
        if (1<(ptr+i)->pid&&(ptr+i)->deadline<(ptr+i)->life_time) {
            switch ((ptr+i)->type) {
            case HEART_BEAT_SEND_PROCESS:
                flag_send_hb = 1;
                break;
            case HEART_BEAT_RECV_PROCESS:
                flag_recv_hb = 1;
                break;
            case RECORD_PROCESS:
                success = kill((ptr+i)->pid,SIGKILL);
                waitpid(-1,NULL,WNOHANG);
                if (0==success) {
                    printf("\n\033[32mRECORD process %d was killed due to its deadline (%d).\033[0m\n",(ptr+i)->pid,(ptr+i)->deadline);
                    LOG(ERROR)<<"Record process "<<(ptr+i)->pid <<", was killed due to lifetime: "<<(ptr+i)->deadline;
                    Increase_half_lifetime_record_process(ptr, prcs_num);
                } else {
                    LOG(ERROR)<<"Record process "<<(ptr+i)->pid <<", was not killed successfully although its lifetime expires: "<<(ptr+i)->deadline;
                    perror("Record processs was killed, but kill operation faild");
                    printf("\n\033[35mThe RECORD %d process should be killed. But killing operation failed.\033[0m\n",(ptr+i)->pid);
                }
                if (1 == (ptr+i)->process_step) {
                    success = Set_ownself_server_mode(PRISYNC);
                }
                flag_record = 1;
                (ptr+i)->pid = 0;
                (ptr+i)->life_time = 0;
                (ptr+i)->deadline = 0;
                (ptr+i)->type = NORMAL_PROCESS;
                (ptr+i)->process_step = 0;
                success = Unregister_process_from_process_table(ptr, prcs_num, (ptr+i)->pid);
                break;
            case VIEW_PROCESS:
                success = kill((ptr+i)->pid,SIGKILL);
                waitpid(-1,NULL,WNOHANG);
                if (0==success) {
                    LOG(ERROR)<<"View process "<<(ptr+i)->pid <<", was killed due to lifetime: "<<(ptr+i)->deadline;
                    printf("\n\033[32mVIEW process %d was killed due to its deadline (%d).\033[0m\n",(ptr+i)->pid,(ptr+i)->deadline);
                } else {
                    LOG(ERROR)<<"View process "<<(ptr+i)->pid <<", was not killed successfully although its lifetime expires: "<<(ptr+i)->deadline;
                    perror("View processs was killed, but kill operation faild");
                    printf("\n\033[35mThe VIEW %d process should be killed. But killing operation failed.\033[0m\n",(ptr+i)->pid);
                }
                (ptr+i)->pid = 0;
                (ptr+i)->life_time = 0;
                (ptr+i)->deadline = 0;
                (ptr+i)->type = NORMAL_PROCESS;
                (ptr+i)->process_step = 0;  
                success = Unregister_process_from_process_table(ptr, prcs_num, (ptr+i)->pid);
                break;
            case CAS_PROCESS:
                success = kill((ptr+i)->pid,SIGKILL);
                waitpid(-1,NULL,WNOHANG);
                if (0==success) {
                    LOG(ERROR)<<"CAS balance process "<<(ptr+i)->pid <<", was killed due to lifetime: "<<(ptr+i)->deadline;
                    printf("\n\033[32mCAS balance process %d was killed due to its deadline (%d).\033[0m\n",(ptr+i)->pid,(ptr+i)->deadline);
                    success = Increase_half_lifetime_record_process(ptr, prcs_num);
                } else {
                    LOG(ERROR)<<"CAS balance process "<<(ptr+i)->pid <<", was not killed successfully although its lifetime expires: "<<(ptr+i)->deadline;
                    perror("Cas balance processs was killed, but kill operation faild");
                    printf("\n\033[35mThe CAS balance %d process should be killed. But killing operation failed.\033[0m\n",(ptr+i)->pid);
                }
                (ptr+i)->pid = 0;
                (ptr+i)->life_time = 0;
                (ptr+i)->deadline = 0;
                (ptr+i)->type = NORMAL_PROCESS;
                (ptr+i)->process_step = 0;
                success = Unregister_process_from_process_table(ptr, prcs_num, (ptr+i)->pid);
                break;
            default:
                break;
            }
            fflush(NULL);
            google::FlushLogFiles(google::ERROR);
            google::FlushLogFiles(google::INFO);
            usleep(100);
        }
    }
    if (1==flag_recv_hb||1==flag_send_hb) {
        Set_brother_server_mode(ERROR);
    } else {
        Set_brother_server_mode(READY);
    }
    if (1==flag_record||1==flag_sync)
        return 1;
    else
        return -1;
}





enum ServerType Get_ownself_server_type(void)
{
    enum ServerType t = UNKOWN_DB;
    void * mem_ptr = NULL;
    int success = 0;
    int semid_srv = 0;
    semid_srv = GetExistedSemphore(SRV_MODE_TYPE_SHARE_ID);
    success = AcquireAccessRight(semid_srv);
    mem_ptr = MappingShareMemOwnSpace(SRV_MODE_TYPE_SHARE_ID);

    t = ((struct ShareMemSrvMode *)mem_ptr)->ownself_type;

    /* Free memory control handler */
    success = UnmappingShareMem((void*)mem_ptr);
    success = ReleaseAccessRight(semid_srv);

    return t;
}

void Set_ownself_server_type(enum ServerType srv_type)
{
    struct ShareMemSrvMode * mem_ptr = NULL;
    int success = 0;
    int semid = 0;
    semid = GetExistedSemphore(SRV_MODE_TYPE_SHARE_ID);
    success = AcquireAccessRight(semid);
    mem_ptr = (struct ShareMemSrvMode *) MappingShareMemOwnSpace(SRV_MODE_TYPE_SHARE_ID);

    mem_ptr->ownself_type = srv_type;

    /* Free memory control handler */
    success = UnmappingShareMem((void*)mem_ptr);
    success = ReleaseAccessRight(semid);

    return ;
}

void  Print_ownself_server_type(void)
{

    enum ServerType srv_type;
    srv_type = Get_ownself_server_type();
    printf("\rOwnself TYPE: ");
    switch (srv_type) {
    case INVOICE_DB:
        printf("\033[04;34m%s\033[0m","INVOICE_DB");
        break;
    case MAIN_DB:
        printf("\033[04;33m%s\033[0m","MAIN_DB");
        break;
    case SLAVE_DB:
        printf("\033[04;32m%s\033[0m","SLAVE_DB");
        break;
    default:
        printf("\033[04;31m%s\033[0m","UNKNOWN");
        break;
    }

    return;

}

/* *************************************************
 * Function Name:
 *		enum ServerMode Get_ownself_server_mode(void)
 * Input:
 * 		NONE
 * Ouput:
 * 		the server mode TWINS or ALONE
 * *************************************************/
enum ServerMode Get_ownself_server_mode(void)
{
    enum ServerMode mode = ERROR;
    void * mem_ptr = NULL;
    int success = 0;
    int semid_srv = 0;
    semid_srv = GetExistedSemphore(SRV_MODE_TYPE_SHARE_ID);
    success = AcquireAccessRight(semid_srv);
    mem_ptr = MappingShareMemOwnSpace(SRV_MODE_TYPE_SHARE_ID);

    mode = ((struct ShareMemSrvMode *)mem_ptr)->ownself_mode;

    /* Free memory control handler */
    success = UnmappingShareMem((void*)mem_ptr);
    success = ReleaseAccessRight(semid_srv);

    return mode;
}


/* *************************************************
 * Function Name:
 *		enum ServerMode Print_ownself_server_mode(void)
 * Input:
 * 		NONE
 * Ouput:
 * 		the server mode TWINS or ALONE
 * *************************************************/
void Print_ownself_server_mode(void)
{
    /* Display the two servers status */
    enum ServerMode srv_mode;
    srv_mode = Get_ownself_server_mode();
    printf(" Ownself: ");
    switch (srv_mode) {
    case READY:
        printf("\033[36mREADY\033[0m");
        break;
    case TWINS:
        printf("\033[32mTWINS\033[0m");
        break;
    case MAINTAIN:
        printf("\033[33mMAINTAIN\033[0m");
        break;
    case PRISYNC:
    case SYNC:
        printf("\033[34mSYNC \033[0m");
        break;
    case ERROR:
        printf("\033[35mERROR\033[0m");
        break;
    case CHECK:
        printf("\033[0mCHECK\033[0m");
        break;
    default:
        printf("\033[31mNO SUCH STATUS!\033[0m\t");
        break;
    }
    return;
}


/* *************************************************
 * Function Name:
 * 		int Set_ownself_server_mode(enum ServerMode server_mode)
 * Input:
 * 		server_mode ---> TWINS or ALONE
 * Output:
 * 		1 ---> success;
 * 		-1 ---> failure;
 * *************************************************/
int Set_ownself_server_mode(enum ServerMode server_mode)
{
    void * mem_ptr = NULL;
    int success = 0;
    int semid_srv = 0;

    /*chang the share state*/
    semid_srv = GetExistedSemphore(SRV_MODE_TYPE_SHARE_ID);
    success = AcquireAccessRight(semid_srv);
    mem_ptr = MappingShareMemOwnSpace(SRV_MODE_TYPE_SHARE_ID);

    ((struct ShareMemSrvMode *)mem_ptr)->ownself_mode = server_mode;

    /* Free memory control handler */
    success = UnmappingShareMem((void*)mem_ptr);
    success = ReleaseAccessRight(semid_srv);

    usleep(200);
    //DBG("\n%s: |%d|\n", "Server Mode(ERROR=0,READY,CHECK,MAINTAIN) is changed into ", server_mode);

    return success;
}

/* *************************************************
 * Function Name:
 *		enum ServerMode Get_brother_server_mode(void)
 * Input:
 * 		NONE
 * Ouput:
 * 		the server mode TWINS or ALONE
 * *************************************************/
enum ServerMode Get_brother_server_mode(void)
{
    enum ServerMode mode = ERROR;
    void * mem_ptr = NULL;
    int success = 0;
    int semid_srv = 0;
    semid_srv = GetExistedSemphore(SRV_MODE_TYPE_SHARE_ID);
    success = AcquireAccessRight(semid_srv);
    mem_ptr = MappingShareMemOwnSpace(SRV_MODE_TYPE_SHARE_ID);

    mode = ((struct ShareMemSrvMode *)mem_ptr)->brother_mode;

    /* Free memory control handler */
    success = UnmappingShareMem((void*)mem_ptr);
    success = ReleaseAccessRight(semid_srv);

    return mode;
}

void Print_brother_server_mode(void)
{
    /* Display the two servers status */
    enum ServerMode srv_mode;
    srv_mode = Get_brother_server_mode();
    printf(" Brother: ");
    switch (srv_mode) {
    case READY:
        printf("\033[36mREADY\033[0m");
        break;
    case TWINS:
        printf("\033[32mTWINS\033[0m");
        break;
    case MAINTAIN:
        printf("\033[33mMAINTAIN\033[0m");
        break;
    case PRISYNC:
    case SYNC:
        printf("\033[34mSYNC \033[0m");
        break;
    case ERROR:
        printf("\033[35mERROR\033[0m");
        break;
    case CHECK:
        printf("\033[0mCHECK\033[0m");
        break;
    default:
        printf("\033[31mUNKOWN\033[0m");
        break;
    }
    return;
}

/* *************************************************
 * Function Name:
 * 		int Set_brother_server_mode(enum ServerMode server_mode)
 * Input:
 * 		server_mode ---> TWINS or ALONE
 * Output:
 * 		1 ---> success;
 * 		-1 ---> failure;
 * *************************************************/
int Set_brother_server_mode(enum ServerMode server_mode)
{
    void * mem_ptr = NULL;
    int success = 0;
    int semid_srv = 0;
    semid_srv = GetExistedSemphore(SRV_MODE_TYPE_SHARE_ID);
    success = AcquireAccessRight(semid_srv);
    mem_ptr = MappingShareMemOwnSpace(SRV_MODE_TYPE_SHARE_ID);

    ((struct ShareMemSrvMode *)mem_ptr)->brother_mode = server_mode;

    /* Free memory control handler */
    success = UnmappingShareMem((void*)mem_ptr);
    success = ReleaseAccessRight(semid_srv);

    usleep(200);
    return success;
}

/* *************************************************
 * Function Name:
 * 		int ReStart_recv_heart_beat_pkt_process(void)
 * Input:
 * 		NONE;
 * Ouput:
 * 		1 ---> success
 * 		-1 ---> failure
 * *************************************************/
int ReStart_recv_heart_beat_pkt_process(void)
{
    pid_t pid = 0;
    int success = 0;
RE_FORK:
    if ((pid = fork()) < 0) {
        perror("error:primary_db_la.cc:Init_comm_db_process():fork");
        sleep(DELAY_MIN_TIME);
        goto RE_FORK;
    } else if (0 == pid) {
        /* In the first child process*/
        if ((pid = fork()) < 0)
            perror("error:primary_db_la.cc:Init_comm_db_process():fork2");
        else if (pid > 0)
            exit(0); 		/* Return original parent */
        else {
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
 * 		int Restart_send_heart_beat_pkt_process(void)
 * Input:
 * 		NONE;
 * Ouput:
 * 		1 ---> success
 * 		-1 ---> failure
 * *************************************************/
int Restart_send_heart_beat_pkt_process(void)
{
    pid_t pid = 0;
    int success = 0;
RE_FORK:
    if ((pid = fork()) < 0) {
        perror("error:primary_db_la.cc:Init_comm_db_process():fork");
        sleep(DELAY_MIN_TIME);
        goto RE_FORK;
    } else if (0 == pid) {
        /* In the first child process*/
        if ((pid = fork()) < 0)
            perror("error:primary_db_la.cc:Start_heart_beat_process():fork2");
        else if (pid > 0)
            exit(0); 		/* Return original parent */
        else {
            LOG(INFO) <<"SENDBH_PROCESS";
            success = Insert_pid_process_table(getpid(),HEART_BEAT_PROCESS_DEADLINE,HEART_BEAT_SEND_PROCESS);
            success = Send_heart_beat_packet();
        }
    }

    if (waitpid(pid,NULL,0)!=pid)
        perror("primary_db_la.cc:Start_heart_beat_process:waitpid");
    return 1;
}

/* *************************************************
 * Function Name:
 * 		int Count_record_process_sum(struct ChildProcessStatus *ptr, int prcs_num)
 *      find the number of Record_process in queue
 * Input:
 * 		struct ChildProcessStatus *ptr ---> all the child process in the table
 * 		int prcs_num   --->  the sum of all child processes
 * Output:
 * 		1 ---> success;
 * 		-1 ---> failure
 * *************************************************/
int Count_record_process_sum(struct ChildProcessStatus *ptr, int prcs_num)
{
    int i = 0;
    int record_process_count = 0;
    /* Check the input parameters */
    if (NULL==ptr||0>=prcs_num) {
        perror("error@mointor_process.cc:Count_record_process_sum():NULL==ptr");
        return -1;
    }

    /* Count all RECORD or CAS process to be handled in the queue */
    record_process_count = 0;
    for (i=0; i<prcs_num; i++) {
        if ((RECORD_PROCESS == (ptr+i)->type) ||(CAS_PROCESS == (ptr+i)->type)) {
            record_process_count++;
        }
    }
    if (0 != record_process_count ) {
        printf("\n\033[32mThere are %d RECORD or CAS process to be handled in the queue, please wait!\033[0m\n",record_process_count);
    } else {
        printf("\n\033[32mThere are NO RECORD or CAS process to be handled in the queue, the Next Step will be excuted right now!\033[0m\n");
    }
    return record_process_count;
}

/* *************************************************
 * Function Name:
 * 		int Increase_half_lifetime_record_process(struct ChildProcessStatus *ptr, int prcs_num, int deadline)
 *      set the lifetime of Record_process in queue to 0
 * Input:
 * 		struct ChildProcessStatus *ptr ---> all the child process in the table
 * 		int prcs_num   --->  the sum of all child processes
 * 		int deadline ---> the max life time
 * Output:
 * 		1 ---> success;
 * 		-1 ---> failure
 * *************************************************/
int Increase_half_lifetime_record_process(struct ChildProcessStatus *ptr, int prcs_num)
{
    int i = 0;
    /* Check the input parameters */
    if (NULL==ptr||0>=prcs_num) {
        perror("error@mointor_process.cc:Zero_Lifetime_Record_Process():NULL==ptr");
        return -1;
    }

    /* ZERO all RECORD or CAS process to be handled in the queue */
    for (i=0; i<prcs_num; i++) {
        if ((RECORD_PROCESS == (ptr+i)->type) ||(CAS_PROCESS == (ptr+i)->type)) {
            (ptr+i)->life_time = (int)((ptr+i)->life_time / PROCESS_LIEF_TIME_INC_MULTIPLY_FACTOR);
        }
    }
    return 1;
}

void Print_current_date_time(void)
{
    time_t t;
    t = time(NULL);
    printf(" %24.24s\r",ctime(&t));

}


/* *************************************************
 * File name:
 * 		bk_monitor.cc
 * Description:
 * 		To avoid the process being blocked, there is a monitor process
 * 		to kill those ones whose life time is too long.
 * Author:
 * 		Yan Zhiwei, jerod.yan@gmail.com  (Drum Team)
 * Date:
 *		2006-07-07
 * *************************************************/
#include "bk_monitor.h"
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
	semid = GetExistedSemphoreExt(SEM_PROCESS_FTOK_ID);
	mem_ptr = MappingShareMemOwnSpaceExt(SHM_PROCESS_FTOK_ID);

	success = AcquireAccessRight(semid);
	/*  insert the pid of the process into the table */
	success = Register_process_into_process_table(((struct ShareMemProcess *)mem_ptr)->process_table,MAX_PROCESS_NUMBRER,pid,deadline,type);
	success = ReleaseAccessRight(semid);

	/* Free memory control handler */
	success = UnmappingShareMem((void*)mem_ptr);

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
	semid = GetExistedSemphoreExt(SEM_PROCESS_FTOK_ID);
	mem_ptr = MappingShareMemOwnSpaceExt(SHM_PROCESS_FTOK_ID);

	success = AcquireAccessRight(semid);
	/*  insert the pid of the process into the table */
	success = Unregister_process_from_process_table(((struct ShareMemProcess *)mem_ptr)->process_table,MAX_PROCESS_NUMBRER,pid);
	success = ReleaseAccessRight(semid);

	/* Free memory control handler */
	success = UnmappingShareMem((void*)mem_ptr);

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
	semid = GetExistedSemphoreExt(SEM_PROCESS_FTOK_ID);
	mem_ptr = (struct ShareMemProcess *)MappingShareMemOwnSpaceExt(SHM_PROCESS_FTOK_ID);
	process_ptr = (struct ChildProcessStatus * )(mem_ptr->process_table);

	success = AcquireAccessRight(semid);
	/*  insert the pid of the process into the table */
	/* Place the pid into the table */
	for(i=0;i<MAX_PROCESS_NUMBRER;i++) {
		if(pid==(process_ptr+i)->pid) {
			(process_ptr+i)->life_time = life_time;
			break;
		}
	}
	success = ReleaseAccessRight(semid);

	/* Free memory control handler */
	success = UnmappingShareMem((void*)mem_ptr);

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
	/* Check the input parameters */
	if(NULL==ptr||0>prcs_num||0>deadline)
	{
		perror("error@monitor_process.cc:Add_process_into_time_table():NULL==ptr");
		return -1;
	}

	/* Place the pid into the table */
	for(i=0;i<prcs_num;i++)
	{
		if(0==(ptr+i)->pid) {
			(ptr+i)->pid = pid;
			(ptr+i)->life_time = 0;
			(ptr+i)->deadline = deadline;
			(ptr+i)->type = type;

			printf("\r\033[33mRegister PID %d is OK in slot %d\033[0m\n",pid,i);
			break;
			fflush(NULL);
		}
	}

	return 1;
}

int Unregister_process_from_process_table(struct ChildProcessStatus *ptr, int prcs_num,pid_t pid)
{
	int i = 0;
	/* Check the input parameters */
	if(NULL==ptr||0>prcs_num)
	{
		perror("error@monitor_process.cc:Add_process_into_time_table():NULL==ptr");
		return -1;
	}

	/* Remove the pid into the table */
	for(i=0;i<prcs_num;i++)
	{
		if(pid==(ptr+i)->pid) {
			(ptr+i)->pid = 0;
			(ptr+i)->life_time = 0;
			(ptr+i)->deadline = 0;
			(ptr+i)->type = NORMAL_PROCESS;
			printf("\r\033[36mUnRegister PID %d is OK from slot %d.\033[0m\n",pid,i);
			break;
			fflush(NULL);
		}
	}

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
	if(NULL==ptr||0>=prcs_num)
	{
		perror("error@bk_monitor.cc:Increase_process_life_time():NULL==ptr");
		return -1;
	}

	for(i=0;i<prcs_num;i++)
	{
		if(0!= (ptr+i)->pid)
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
 * 		int deadline ---> the max life time
 * Output:
 * 		1 ---> success;
 * 		-1 ---> failure
 * *************************************************/
int Kill_invalid_process(struct ChildProcessStatus *ptr, int prcs_num)
{
	int i = 0;
	int success = 0;

	int sem_id = 0;
	struct UpdateProtection *mem_protection_ptr = NULL;

	/* Check the input parameters */
	if(NULL==ptr||0>=prcs_num)
	{
		perror("error@bk_monitor.cc:Add_process_life_time():NULL==ptr");
		return -1;
	}

	/* Kill all process that exceed their deadlines. */
	for(i=0;i<prcs_num;i++)
	{
		/* if the process belongs to the process */
		if(1<(ptr+i)->pid&&(ptr+i)->deadline<(ptr+i)->life_time) {
			switch((ptr+i)->type) {
			case ACCOUNT_SERVER_PROCESS:
				fflush(NULL);
				printf("\033[01;31mACCOUNT_SERVER_PROCESS %d is in invalid status, it will be killed now\033[0m\n", (ptr+i)->pid);
				fflush(NULL);
				if(0!=(success = kill((ptr+i)->pid,SIGKILL)))
				{
					perror("error@bk_monitor.c:Kill_invalid_process:kill(ACCOUNT_SERVER_PROCESS)");
				}
				waitpid(-1,NULL,WNOHANG);	

				sem_id = GetExistedSemphoreExt(SEM_PROTECTION_FTOK_ID);
				mem_protection_ptr = (struct UpdateProtection *)MappingShareMemOwnSpaceExt(SHM_PROTECTION_FTOK_ID);
				if (mem_protection_ptr->pid == (ptr+i)->pid)
				{
					/*clear the flag of update all terminal latest reckoning date*/
					AcquireAccessRight(sem_id);
					mem_protection_ptr->update_all_latest_reckoning_date_flag = 0;
					mem_protection_ptr->pid = 0;
					ReleaseAccessRight(sem_id);
				}
				success = UnmappingShareMem((void *)mem_protection_ptr);

				(ptr+i)->pid = 0;
				(ptr+i)->life_time = 0;
				(ptr+i)->deadline = 0;
				(ptr+i)->type = NORMAL_PROCESS;
				break;
			case RENEW_DEPOSIT_PROCESS:
				fflush(NULL);
				printf("\033[01;31mRENEW_DEPOSIT_PROCESS %d is in invalid status, it will be killed now\033[0m\n", (ptr+i)->pid);
				fflush(NULL);
				if(0!=(success = kill((ptr+i)->pid,SIGKILL)))
				{
					perror("error@bk_monitor.c:Kill_invalid_process:kill(RENEW_DEPOSIT_PROCESS)");
				}
				waitpid(-1,NULL,WNOHANG);	
				
				(ptr+i)->pid = 0;
				(ptr+i)->life_time = 0;
				(ptr+i)->deadline = 0;
				(ptr+i)->type = NORMAL_PROCESS;
				/* Restart */
				success = Init_auto_renew_deposit_process();
				fflush(NULL);
				printf("Initiating auto refresh bank deposit:");
				if(-1 == success) {
					OUTPUT_ERROR;
					perror("error@bk_monitor.c:Kill_invalid_process:Init_auto_renew_deposit_process()");
				} else {
					OUTPUT_OK;
				}
				fflush(NULL);
				break;
			case RENEW_BALANCE_PROCESS:
				fflush(NULL);
				printf("\033[01;31mRENEW_BALANCE_PROCESS %d is in invalid status, it will be killed now\033[0m\n", (ptr+i)->pid);
				fflush(NULL);
				if(0!=(success = kill((ptr+i)->pid,SIGKILL)))
				{
					perror("error@bk_monitor.c:Kill_invalid_process:kill(RENEW_BALANCE_PROCESS)");
				}
				waitpid(-1,NULL,WNOHANG);	
				
				(ptr+i)->pid = 0;
				(ptr+i)->life_time = 0;
				(ptr+i)->deadline = 0;
				(ptr+i)->type = NORMAL_PROCESS;
				/* Restart */				
				success = Init_auto_renew_balance_process();
				fflush(NULL);
				printf("Initiating auto refresh virtual balance:");
				if(-1 == success) {
					OUTPUT_ERROR;
					perror("error@bk_monitor.c:Kill_invalid_process:Init_auto_renew_balance_process()");
				} else {
					OUTPUT_OK;
				}
				fflush(NULL);
				break;

			default:
				break;
			}
			usleep(100);
		}
	}
	return 1;
}

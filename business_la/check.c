/*********************************************************
 *project: Line communication charges supermarket
 *filename: check.c
 *version: 0.4
 *purpose: some function use for check process
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-12
 *********************************************************/
#include "check.h"


/*************************************************************************
 *  \brief
 *    create a process for check the control channel process and (longlink process) 
 *    the process check the control channel process is alive (Is Blocked 
 *    because of proxy dump) periodically.
 *
 *  \par Input:
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/
int CreateCheckProcess()
{
    int pid, success = 0;
    
	if ((pid = fork())<0)
    {
    	perror("error@check.c:CreateCheckProcess:fork_1\n");
    	exit(1);
    }
    else if (0==pid) /* first child */ 
    {
    	/*first child process */
   		if((pid=fork())<0)
   		{
			perror("error@check.c:CreateCheckProcess:fork_2\n");
   			exit(1);
   		}
   		else if (pid>0)
   		{
   			/*parent for second fork == fist child */
   			/*Make it exit normally*/
   			exit(0);
   			/*We're the second child; our parent becomes init as soon
   			as our real parent calls exit() in the statement above.
   			Here's where we'd continue executing, knowing that when
   			we're done, init will reap our status. */
   		}
   			
   		/* this is the second child process! */
		
		/*This process is the Check Process and get the pid of it*/
	    
   		success = 1;
		printf("The process check is running ");
	   	if(success)
   		{
   			OUTPUT_OK;
   		}
   		else
   		{
   			OUTPUT_ERROR;
   		}				
        
   		//WriteProcessInfoToFile("CHECK_PROCESS");
     	HandleCheckProcedure();
   
        exit(0);
  	}
	if(waitpid(pid, NULL, 0)!=pid)
		perror("error@check.c:CreateCheckProcess:waitpid\n");

	return 1;    
}

/*************************************************************************
 *  \brief
 *    Handle the check procedure periodicaly
 *
 *  \par Input:
 *    semid: the semphore of access watch dog.
 *    watchdog: the pointer of share counter between check process 
 *              and control channel process.
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/
int HandleCheckProcedure()
{
	int success = 0, i = 0;
	struct Report_business_status status;
	
	int semid;
	struct Watch_dog_ctl_channel * watchdog; 
	
	/*varibles for check data channels*/
	struct Queue_business_process *shm_data_chan;
	int sem_data_chan;

	/*malloc the share memory for check data channels at the first time*/
	sem_data_chan = GetExistedSemphoreExt(SEM_DATA_CHAN_FTOK_ID);
	shm_data_chan = (struct Queue_business_process *)MappingShareMemOwnSpaceExt(SHM_DATA_CHAN_FTOK_ID);	

	/*create the control channel process at first time*/
	for (i=0;i<global_par.system_par.proxy_number;i++)
	{
   		/*create the control channel process */
		semid = InitialSem(SEM_CTL_FTOK_ID_BEGIN + 2*i);
		watchdog = (struct Watch_dog_ctl_channel *)InitialShm(sizeof(struct Watch_dog_ctl_channel), SHM_CTL_FTOK_ID_BEGIN + 2*i);
		bzero(watchdog, sizeof(struct Watch_dog_ctl_channel));
		success = CreateControlChannelProcess(i);
	}
	
	/*check the status of LC channel and control channel, if it is broken, ReCreate it*/
	while(1)
	{
		/*check the status of data channel, if they are blocked, kill them*/
		success = CheckDataChannelStateAndReport(sem_data_chan, shm_data_chan);

   		/*check the status of control channel, if it is broken, ReCreate it*/
		for (i=0;i<global_par.system_par.proxy_number;i++)
		{
			success = CheckControlChannelStateAndReport(&status, i);
		}
		sleep(CHECK_CONTROL_CHANNEL_INTERVAL);		
	}

	/*Unmapping the share memory of data channels status*/
	success = UnmappingShareMem((void*)shm_data_chan);

	return success;
    
}

/*************************************************************************
 *  \brief
 *    Handle the check control channel status
 *
 *  \par Input:
 *    semid: the semphore of control channel information.
 *    watchdog: the pointer of share counter between check process 
 *              and control channel process.
 *    proxy_id: the id which identify the proxy machine
 *
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/
int CheckControlChannelStateAndReport(struct Report_business_status *status, int proxy_id)
{
	int watchdog_value = -1;
	int success = 0;
	int ctl_pid;

	int semid;
	struct Watch_dog_ctl_channel *watchdog;

	semid = GetExistedSemphoreExt(SEM_CTL_FTOK_ID_BEGIN + 2*proxy_id);
	watchdog = (struct Watch_dog_ctl_channel *)MappingShareMemOwnSpaceExt(SHM_CTL_FTOK_ID_BEGIN + 2*proxy_id);

   	success = AcquireAccessRight(semid);
#ifdef DEBUG_WATCHDOG_CTL_CHANNEL
	printf("Before change, watch dog = %d \n", watchdog->counter);
#endif
   	/*add CHECK_ADD_STEP to the counter*/
   	watchdog->counter += CHECK_ADD_STEP;
   	watchdog_value = watchdog->counter;
   	ctl_pid = watchdog->ctl_pid;
	/*write current LC channel status to watchdog*/
	memcpy(&(watchdog->status), status, sizeof(struct Report_business_status));
#ifdef DEBUG_WATCHDOG_CTL_CHANNEL
	printf("After change, watch dog = %d \n", watchdog->counter);
#endif  
 	success = ReleaseAccessRight(semid);
   		
   	/*judge the watchdog > T ? */
   		
   	if(watchdog_value > THRESHOLD_BROKEN_CONTROL_CHANNEL)
   	{
   		/*The control channel is brocken, Kill this control channel
   		process, and ReCreate a new process*/
   		printf("The status of control channel is");
   		OUTPUT_ERROR;
   			
		//printf("I will kill control process PID is %d\n", ctl_pid);
   		if(-1 != ctl_pid)
		{
			if(0 != kill(ctl_pid, SIGKILL))
			{
				perror("error:check.c:HandleCheckProcedure:kill");
			}
			waitpid(-1,NULL,WNOHANG);
		}		
#ifdef DEBUG_WATCHDOG_CTL_CHANNEL
		printf("Before change, watch dog = %d \n", watchdog->counter);
#endif
		/*clear the watchdog*/
		success = AcquireAccessRight(semid);
		watchdog->counter = 0;
		success = ReleaseAccessRight(semid);
#ifdef DEBUG_WATCHDOG_CTL_CHANNEL
		printf("After change, watch dog = %d \n", watchdog->counter);
#endif  

   		success = CreateControlChannelProcess(proxy_id);
   		/*wait some time till the control channel is OK!!!*/
   		usleep(SHORT_SLEEP);
   	}
	success = UnmappingShareMem((void *)watchdog);
	return 1;
}

/*************************************************************************
 *  \brief
 *    check data channel (short link) status
 *
 *  \par Input:
 *    semid: the semphore of data channel information.
 *    shm_data_chan: the pointer of share memory of data channels.
 *
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/
int CheckDataChannelStateAndReport(int semid, struct Queue_business_process *shm_data_chan)
{
	int success = 0;	
	int i;
	//int process_num = 0;

	success = AcquireAccessRight(semid);/*acquire the right to access the share memory*/
	for (i=0; i<NUM_PROCESS_QUEUE; i++)
	{
		if (shm_data_chan[i].pid > 0)
		{
			//process_num++;
			shm_data_chan[i].life_time += CHECK_ADD_STEP;
			//printf("The PID : %d 's left time is %d !!!\n", shm_data_chan[i].pid, shm_data_chan[i].life_time);
			if (shm_data_chan[i].life_time > THRESHOLD_BLOCK_DATA_CHANNEL)/*this data channel process is down*/
			{
				RecordAbnormalSerialToLog((struct Queue_business_process *)(&(shm_data_chan[i])));
				if (0 != kill(shm_data_chan[i].pid, SIGKILL))
				{
					perror("error@proxy_check.c:Check_business_ctl_status:kill()");
				}
				waitpid(-1,NULL,WNOHANG);
				printf("\033[01;31mProcess PID : %d of data channel is killed because it was blocked!\033[0m\n", shm_data_chan[i].pid);
				shm_data_chan[i].pid = 0;
				shm_data_chan[i].life_time = 0;
			}
		}
	}				
	success = ReleaseAccessRight(semid);/*release the right to access the share memory*/
	//printf("\n\n\n\n\t\t\t\t proxy_check=|%d|\n\n\n\n\n\n\n ", process_num);

	return success;
}

/*************************************************************************
 *  \brief
 *    Write myself porcess pid to files
 *
 *  \par Input:
 *    semid: the semphore of control channel information.
 *    watchdog: the pointer of share counter between check process 
 *              and control channel process.
 *    proxy_id: the id which identify the proxy machine
 *
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/
int WriteProcessInfoToFile(const char *processinfo)
{
	time_t time1 = 0;
	struct tm *time2 = NULL;
	char current_time_str[TEMP_LENGTH];
	char fname[TEMP_LENGTH];
	FILE *fp;
	
	/*get the now time*/
	time(&time1);
	time2=localtime(&time1);/*converts the calendar time to broken-time representation*/
	
	sprintf(fname, "%s.log", processinfo);
	if((fp = fopen(fname, "w")) == NULL)
	{
		printf("Fail to open %s\n", fname);
 		return -1;
	}	
	sprintf(current_time_str, "(%s) PID : %d created at %04d%02d%02d   %02d:%02d:%2d!", processinfo, getpid(), 1900+time2->tm_year, 1+time2->tm_mon, time2->tm_mday, time2->tm_hour, time2->tm_min, time2->tm_sec);
	fprintf(fp, "%s", current_time_str);

	fclose(fp);
	return 1;

}

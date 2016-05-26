/*********************************************************
 *project: Line communication charges supermarket
 *filename: ctl_channel.c
 *version: 0.4
 *purpose: some function use for control channel process 
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-9
 *********************************************************/
#include "ctl_channel.h"


/*************************************************************************
 *  \brief
 *    Handle the init procedure by order from proxy
 *    Setup the long link with enterprise according proxy
 *
 *  \par Input:
 *    sockid: the array of connected socket
 *    proxy_id: the id which identify the proxy machine
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/
int HandleInitByProxy(int sockid, int proxy_id)
{

	/*varibles for config unicom parameters*/
	char buf_recv[MAX_REPORT_PKT_SIZE];
	char buf_send[MAX_REPORT_PKT_SIZE];
	char tmp_string[MAX_REPORT_PKT_SIZE];
	int recv_len;

	/*varibles for share memory and semphore*/
	struct Watch_dog_ctl_channel *watchdog;
   int semid;

	/*varibles for check data channels*/
	struct DB_status *shm_db_status;
	int sem_db_status;
	
	/*malloc the share memory for check data channels at the first time*/
	sem_db_status = GetExistedSemphoreExt(SEM_DB_STATUS_FTOK_ID);
	shm_db_status = (struct DB_status *)MappingShareMemOwnSpaceExt(SHM_DB_STATUS_FTOK_ID);

	int success = 0;
	
	/*Get the existed semphore and mapping the share memory at first*/
    semid = GetExistedSemphoreExt(SEM_CTL_FTOK_ID_BEGIN+2*proxy_id);
    watchdog = (struct Watch_dog_ctl_channel *)MappingShareMemOwnSpaceExt(SHM_CTL_FTOK_ID_BEGIN+2*proxy_id);

	/*communicate to proxy for report own status, need be implement*/
	while(1)
	{
		/*reset the watch dog to zero!!!*/
   		success = AcquireAccessRight(semid);
		
		/*before use the buffer, fill it with all zeros*/
		bzero(buf_recv,MAX_REPORT_PKT_SIZE);
		bzero(buf_send,MAX_REPORT_PKT_SIZE);
		//strcpy(buf_send, configinput.localhost_id);
		strcpy(buf_send, global_par.system_par.localhost_id);
		strcat(buf_send,":");

		/*Get all LC channel status*/
		bzero(tmp_string,MAX_REPORT_PKT_SIZE);
		sprintf(tmp_string, "%04d", watchdog->status.link_num_now);
		strcat(buf_send,tmp_string);
		strcat(buf_send,"ALL");

#ifdef DEBUG_WATCHDOG_CTL_CHANNEL
		printf("Before reset, watch dog = %d \n", watchdog->counter);
#endif
   		/*add CHECK_ADD_STEP to the counter*/
   		watchdog->counter = 0;

#ifdef DEBUG_WATCHDOG_CTL_CHANNEL
		printf("After reset, watch dog = %d \n", watchdog->counter);
#endif  
 		success = ReleaseAccessRight(semid);
		
		//printf("Begin report own status!\n");
		/*send report packet to proxy*/
		if(send(sockid,buf_send,strlen(buf_send),0)==-1) 
		{
			success = 0;
			perror("error@init.c:HandleInitByProxy:send\n");
			exit(1);
    	}
#ifdef DEBUG_TRANSMIT_PROXY_CONTROL_CHANNEL
		printf("send to proxy = %d \n", strlen(buf_send));
		printf("send pkt to proxy is %s \n", buf_send);
#endif
		/*recieve the feedback packet from proxy*/
		recv_len = recv(sockid, buf_recv, MAX_REPORT_PKT_SIZE, 0);
		if(recv_len<0)
		{
			success = 0;
			perror("error@init.c:HandleInitByProxy:recv\n");
			exit(1);
		}
   	
		success = AcquireAccessRight(sem_db_status);
   		shm_db_status->status = (0 == strcmp(buf_recv, global_par.system_par.database_self_ip_address))? PRIMARY : SLAVE;
		success = ReleaseAccessRight(sem_db_status);
		
		
#ifdef DEBUG_TRANSMIT_PROXY_CONTROL_CHANNEL
		printf("recv from proxy length = %d \n", recv_len);
		printf("recieved pkt from proxy is %s \n", buf_recv);
#endif
		
		sleep(REPORT_STATUS_INTERVAL);		
	}	
    
    /*release the share memory*/
    success = UnmappingShareMem((void *)watchdog);
	/*Unmapping the share memory of data channels status*/
	success = UnmappingShareMem((void*)shm_db_status);
    
	return 1;
    
}

/*************************************************************************
 *  \brief
 *    create a process for charging the control channel 
 *    the process report the business server's status to proxy.
 *
 *  \par Input:
 *    proxy_id: the id which identify the proxy machine
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/
int CreateControlChannelProcess(int proxy_id)
{
    int pid, sock_init, success = 0;
	/*varibles for share memory and semphore*/
	struct Watch_dog_ctl_channel *watchdog;
    int semid;
    
	if ((pid = fork())<0)
    {
    	perror("error@init.c:CreateControlChannelProcess:fork_1\n");
    	exit(1);
    }
    else if (0==pid) /* first child */ 
    {
    	/*first child process */
   		if((pid=fork())<0)
   		{
			perror("error@init.c:CreateControlChannelProcess:fork_2\n");
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
		
		/*This process is the Control Channel Process and get the pid of it*/
	   	//printf("I'm control process PID is %d\n", getpid());

   		/*Create the semphore and share memory at first*/
		semid = GetExistedSemphoreExt(SEM_CTL_FTOK_ID_BEGIN + 2*proxy_id);
		watchdog = (struct Watch_dog_ctl_channel *)MappingShareMemOwnSpaceExt(SHM_CTL_FTOK_ID_BEGIN + 2*proxy_id);
   		
   		/*Initial the watchdog to 0*/

   		success = AcquireAccessRight(semid);

#ifdef DEBUG_WATCHDOG_CTL_CHANNEL
		printf("Before reset, watch dog PID = %d \n", watchdog->ctl_pid);
#endif
   		/*add CHECK_ADD_STEP to the counter*/
   		watchdog->counter = 0;
		watchdog->ctl_pid = getpid();
#ifdef DEBUG_WATCHDOG_CTL_CHANNEL
		printf("After reset, watch dog PID = %d \n", watchdog->ctl_pid);
#endif  
 		success = ReleaseAccessRight(semid); 			


		/*Connection to proxy control server*/
   		sock_init = CreateSocket();
   		success = ConnectServer(global_par.system_par.proxy_ip_addr_array[proxy_id], global_par.system_par.proxy_control_port, sock_init);
   		
		printf("The procedure of connect proxy (%s) control channel is ", global_par.system_par.proxy_ip_addr_array[proxy_id]);
	   	if(success)
   		{
   			OUTPUT_OK;
   		}
   		else
   		{
   			OUTPUT_ERROR;
   		}				
        
        HandleInitByProxy(sock_init, proxy_id);
        
    	/*release the share memory*/
     	success = UnmappingShareMem((void *)watchdog);

        exit(0);
  	}
	if(waitpid(pid, NULL, 0)!=pid)
		perror("error@ctl_channel.c:CreateControlChannelProcess:waitpid\n");	
	return 1;    
}



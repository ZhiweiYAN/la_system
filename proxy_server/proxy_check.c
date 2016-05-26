/*********************************************************
 *project: Line communication charges supermarket
 *filename: proxy_check.c
 *version: 1.0
 *purpose: prototype of proxy server checker
 *developer: gexiaodan, Xi'an Jiaotong University(Drum Team)
 *data: 2007-1-16
 *********************************************************/
#include "proxy_check.h"

/* *************************************************
 *  \brief
 *    check control channel status, if some control channel process is down, kill it
 * 
 * Function Name: 
 * 		int Check_ctl_chan_status(void)
 *
 * Input: 
 *
 * Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Check_ctl_chan_status(void)
{
	/*variables for semaphore and share memory*/
	int sem_id;
	proxy_control_channel *proxy_control_info = NULL;

	int success = 0;
	int i = 0;
	pid_t pid;
	
	/*Create a share memory and a semaphore*/
  	sem_id = InitialSem(SEM_CTL_FTOK_ID);
   	proxy_control_info = (proxy_control_channel *)InitialShm(sizeof(proxy_control_channel), SHM_CTL_FTOK_ID);
	
	/*Initialize the share memory*/
	bzero(proxy_control_info, sizeof(proxy_control_channel));
	/*init that all of the business server are down*/
	for (i=0; i<global_par.system_par.business_number; i++)
	{
		proxy_control_info->business_info_array[i].link_status = -1;
	}
	strcpy(proxy_control_info->busi_to_db_link_ip_address, global_par.system_par.database_self_ip_address);
	proxy_control_info->data_chan_link_status = CUTOFF;/*init that database is cut off*/
	success = UnmappingShareMem((void *)proxy_control_info);/*unmap the share memory*/

	if ((pid = fork()) < 0)
    {
		perror("error@proxy_check.c:Check_business_ctl_status:fork1()");
		exit(1);
	}
    else if (0 == pid)
    {
		/*first child*/
   		if((pid=fork()) < 0)
		{
			perror("error@proxy_check.c:Check_business_ctl_status:fork2()");
   			exit(1);
   		}
   		else if (pid > 0)
   		{
   			exit(0);/*first child exit normally*/
   		}
  		/* this is the second child process*/
		sem_id = GetExistedSemphoreExt(SEM_CTL_FTOK_ID);
   		proxy_control_info = (proxy_control_channel *)MappingShareMemOwnSpaceExt(SHM_CTL_FTOK_ID);
		while(1)
		{
			success = AcquireAccessRight(sem_id);/*acquire the right to access the share memory*/
			/*check business*/
			for (i=0; i<global_par.system_par.business_number; i++)
			{
				if (proxy_control_info->business_info_array[i].ctl_pid > 0)/*business[i] is connected*/
				{
					if (proxy_control_info->business_info_array[i].check_cnt < THRESHOLD_CTL_CHAN_DOWN)/*business[i] is OK*/
					{
						proxy_control_info->business_info_array[i].check_cnt++;
					}
					else/* business[i] is down*/
					{
						if (0 != kill(proxy_control_info->business_info_array[i].ctl_pid, SIGKILL))
						{
							perror("error@proxy_check.c:Check_ctl_chan_status:kill()");
						}
						waitpid(-1,NULL,WNOHANG);
						printf("\033[01;31mProcess PID_:%d of control channel for business[%d] is killed because it was down!\033[0m\n", proxy_control_info->business_info_array[i].ctl_pid, i);
						proxy_control_info->business_info_array[i].check_cnt = 0;
						proxy_control_info->business_info_array[i].link_status = -1;
						proxy_control_info->business_info_array[i].ctl_pid = 0;
					}
				}
			}
			/*check database*/
			for (i=0; i<DATABASE_NUMBER; i++)
			{
				if (proxy_control_info->database_info_array[i].ctl_pid > 0)
				{
					if (proxy_control_info->database_info_array[i].check_cnt < THRESHOLD_CTL_CHAN_DOWN)/*database is ok*/
					{
						proxy_control_info->database_info_array[i].check_cnt++;
					}
					else/*database is down*/
					{
						if (0 != kill(proxy_control_info->database_info_array[i].ctl_pid, SIGKILL))
						{
							perror("error@proxy_check.c:Check_ctl_chan_status:kill()");
						}
						waitpid(-1,NULL,WNOHANG);
						printf("\033[01;31mProcess PID_:%d of control channel for database is killed because it was down!\033[0m\n", proxy_control_info->database_info_array[i].ctl_pid);
						proxy_control_info->database_info_array[i].check_cnt = 0;
						proxy_control_info->database_info_array[i].ctl_pid = 0;
					}
				}
			}
			success = ReleaseAccessRight(sem_id);
			/*check control channel every ? seconds*/
			sleep(CHECK_CTL_INTERVAL);
		}
		/*unmap the existed share memory*/
		success = UnmappingShareMem((void *)proxy_control_info);
		exit(0);
	} 

	/*parent process*/
	if(waitpid(pid,NULL,0) != pid)
	{
		perror("error@proxy_control.c:Check_business_status:waitpid()");
	}
	
	return 1;
	
}

/* *************************************************
 *  \brief
 *    check data channel status, if some data channel process is down, kill it
 *
 * Function Name: 
 * 		int Check_data_chan_status(void)
 *
 * Input: 
 *
 * Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Check_data_chan_status(void)
{
	/*variables for semaphore and share memory*/
	int sem_id;
	proxy_data_channel *proxy_data_info = NULL;

	int success = 0;
	pid_t pid;
	int i = 0;
	
	/*Create a share memory and a semaphore*/
   	sem_id = InitialSem(SEM_DATA_FTOK_ID);
   	proxy_data_info = (proxy_data_channel *)InitialShm(NUM_SHARE_DATA_CHAN * sizeof(proxy_data_channel), SHM_DATA_FTOK_ID);
	
	/*Initialize the share memory*/
	bzero(proxy_data_info, NUM_SHARE_DATA_CHAN * sizeof(proxy_data_channel));
	for (i=0; i<NUM_SHARE_DATA_CHAN; i++)
	{
		proxy_data_info[i].data_pid = 0;
		proxy_data_info[i].check_cnt = 0;
		proxy_data_info[i].data_chan_pack_type = UNKNOWN_PACKET;
		proxy_data_info[i].company_id = 20000;
	}
	success = UnmappingShareMem((void *)proxy_data_info);

	if ((pid = fork()) < 0)
    {
		perror("error@proxy_check.c:Check_business_data_status:fork1()");
		exit(1);
	}
    else if (0 == pid)
    {
		/*first child*/
   		if((pid=fork()) < 0)
		{
			perror("error@proxy_check.c:Check_business_data_status:fork2()");
   			exit(1);
   		}
   		else if (pid > 0)
   		{
   			exit(0);/*first child exit normally*/
   		}
  		/* this is the second child process*/
		sem_id = GetExistedSemphoreExt(SEM_DATA_FTOK_ID);
   		proxy_data_info = (proxy_data_channel *)MappingShareMemOwnSpaceExt(SHM_DATA_FTOK_ID);
		while(1)
		{
			success = AcquireAccessRight(sem_id);
			for (i=0; i<NUM_SHARE_DATA_CHAN; i++)
			{
				if (proxy_data_info[i].data_pid > 0)
				{
					proxy_data_info[i].check_cnt++;

					if (proxy_data_info[i].check_cnt > THRESHOLD_DATA_CHAN_DOWN)/*this data channel process is down*/
					{
						if (0 != kill(proxy_data_info[i].data_pid, SIGKILL))
						{
							perror("error@proxy_check.c:Check_business_ctl_status:kill()");
						}
						waitpid(-1,NULL,WNOHANG);
						printf("\033[01;31mProcess PID_:%d of data channel is killed because it was down!\033[0m\n", proxy_data_info[i].data_pid);
						proxy_data_info[i].data_pid = 0;
						proxy_data_info[i].check_cnt = 0;
						proxy_data_info[i].company_id = 20000;
					}
				}
			}				
			success = ReleaseAccessRight(sem_id);/*release the right to access the share memory*/
			/*check business server every ? seconds*/
			sleep(CHECK_BUSI_DATA_INTERVAL);
		}
		/*unmap the existed share memory*/
		success = UnmappingShareMem((void *)proxy_data_info);
		exit(0);
	} 

	/*parent process*/
	if(waitpid(pid,NULL,0) != pid)
	{
		perror("error@proxy_check.c:Check_data_chan_status:waitpid()");
	}
	return 1;
	
}

/* ************************************************* 
 *  \brief
 *    Initialize monitor channel
 *
 * Function Name: 
 * 		int Init_monitor_channel(void)
 *
 * Input: 
 *
 * Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Init_monitor_channel(void)
{
	int success = 0;
	
	/*initialize control channel check process*/
	success = Check_ctl_chan_status();
	/*initialize data channel check process*/
	success = Check_data_chan_status();
	return success;
}

/* ************************************************* 
 *  \brief
 *    save the process pid of data channel to the share memory
 *
 * Function Name: 
 * 		int Save_data_chan_info(void)
 *
 * Input: 
 *		int sem_id ---> the id of semaphore
 *		proxy_control_channel *proxy_control_info --- > the point of the share memory
 * Output:
 * 		1 ---> success
 * 		-1 ---> reach the connection limits
 * *************************************************/
int Save_data_chan_info(int sem_id, proxy_data_channel *proxy_data_info, 
enum data_channel_packet_type data_chan_type, char *packet)
{
	int success = 0;
	int i = 0;
	pid_t pid = getpid();

	
	int company_id = 0;
	char *e = NULL;
	char company_id_char[10];
	
	//Get the company id and connection number limit; added 2009-11-07
	bzero(company_id_char, 10);
	memcpy(company_id_char,packet+COMPANY_ID_POSITION,COMPANY_ID_LENGTH);
	company_id = strtoll(company_id_char,&e ,10);

	success = AcquireAccessRight(sem_id);

	/*insert the pid and clear the corresponding count*/
	for (i=0; i<NUM_SHARE_DATA_CHAN; i++)
	{
		if (0 == proxy_data_info[i].data_pid)
		{
			proxy_data_info[i].data_pid = pid;
			proxy_data_info[i].check_cnt = 0;
			proxy_data_info[i].data_chan_pack_type = data_chan_type;
			proxy_data_info[i].company_id = company_id;
			
			break;
		}
	}			
	success = ReleaseAccessRight(sem_id);	

	if (NUM_SHARE_DATA_CHAN == i)
	{
   		printf("\033[01;31mproxy_check.c:Save_data_chan_info:space not enough\033[0m\n");
	}

	return 1;
}

/* ************************************************* 
 *  \brief
 *    delete the process pid of data channel from the share memory
 *
 * Function Name: 
 * 		int Delete_data_chan_info(void)
 *
 * Input: 
 *		int sem_id ---> the id of semaphore
 *		proxy_control_channel *proxy_control_info --- > the point of the share memory
 * Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Delete_data_chan_info(int sem_id, proxy_data_channel *proxy_data_info)
{
	int success = 0;
	int i = 0;
	pid_t pid = getpid();
		
	success = AcquireAccessRight(sem_id);

	/*find the pid and clear the corresponding information*/
	for (i=0; i<NUM_SHARE_DATA_CHAN; i++)
	{
		if (pid == proxy_data_info[i].data_pid)
		{
			proxy_data_info[i].data_pid = 0;
			proxy_data_info[i].check_cnt = 0;
			proxy_data_info[i].data_chan_pack_type = UNKNOWN_PACKET;
			proxy_data_info[i].company_id = 20000;
			break;
		}
	}			
	success = ReleaseAccessRight(sem_id);	

	if (NUM_SHARE_DATA_CHAN == i)
	{
   		printf("proxy_check.c:Delete_data_chan_info:pid not found\n");
	}

	return 1;
}

/* ************************************************* 
 *  \brief
 *    judge whether reach the connection upper limit
 *
 * Function Name: 
 * 		int Judge_connection_limit(int sem_id, proxy_data_channel *proxy_data_info, char *packet)
 *
 * Input: 
 *		int sem_id ---> the id of semaphore
 *		proxy_control_channel *proxy_control_info --- > the point of the share memory
 * Output:
 * 		1 ---> not reach the connection limit
 * 		-1 ---> reach the connection limit
 * *************************************************/
int Judge_connection_limit(int sem_id, proxy_data_channel *proxy_data_info, char *packet, enum data_channel_packet_type *data_chan_type)
{
	int connection_num = 0;
	int connection_upper_limit = 0;
	
	int connection_company_num = 0;
	int connection_upper_company_limit = 0;
	
	int company_id = 0;
	char *e = NULL;
	char company_id_char[10];
	
	int success = 0;
	int i = 0;

	//Get the company id and connection number limit; added 2009-11-07
	bzero(company_id_char, 10);
	memcpy(company_id_char,packet+COMPANY_ID_POSITION,COMPANY_ID_LENGTH);
	company_id = strtoll(company_id_char,&e ,10);
	connection_upper_company_limit = global_par.company_par_array[company_id].company_conn;

	/*get the internal flag, and check the packet type*/
	if (0 == memcmp(packet+INTERNAL_PACKET_FLAG_START_POSTION_AT_HEADER, INTERNAL_FLAG_AFFAIR, INTERNAL_PACKET_FLAG_LENGTH_AT_HEADER))
	{
		*data_chan_type = AFFAIR_PACKET;
		connection_upper_limit = global_par.system_par.proxy_max_affair_connection;		
	}
	else if (0 == memcmp(packet+INTERNAL_PACKET_FLAG_START_POSTION_AT_HEADER, INTERNAL_FLAG_SQL, INTERNAL_PACKET_FLAG_LENGTH_AT_HEADER))
	{
		*data_chan_type = SQL_PACKET;
		connection_upper_limit = global_par.system_par.proxy_max_sql_connection;
	}
	else if (0 == memcmp(packet+INTERNAL_PACKET_FLAG_START_POSTION_AT_HEADER, INTERNAL_FLAG_TEMP, INTERNAL_PACKET_FLAG_LENGTH_AT_HEADER))
	{
		*data_chan_type = TEMP_PACKET;
		connection_upper_limit = global_par.system_par.proxy_max_temp_connection;
	}
	else if (0 == memcmp(packet+INTERNAL_PACKET_FLAG_START_POSTION_AT_HEADER, INTERNAL_FLAG_LOOP, INTERNAL_PACKET_FLAG_LENGTH_AT_HEADER))
    	{
       	 *data_chan_type = AFFAIR_PACKET;
        	connection_upper_limit = global_par.system_par.proxy_max_affair_connection;
    	}
	else
	{
		printf("\033[01;31mproxy_check.c:Judge_connection_limit:invalid internal flag\033[0m\n");
		exit(1);
	}

	/*check whether connection reach the upper limit*/
	success = AcquireAccessRight(sem_id);
	for (i=0; i<NUM_SHARE_DATA_CHAN; i++)
	{
		if ((proxy_data_info[i].data_pid > 0) && (*data_chan_type == proxy_data_info[i].data_chan_pack_type))
		{
			connection_num ++;	
		}
		if(AFFAIR_PACKET ==(*data_chan_type))
		{
			if ((proxy_data_info[i].company_id == (unsigned int)company_id) 
				&&(proxy_data_info[i].data_pid > 0)
				&&(AFFAIR_PACKET == proxy_data_info[i].data_chan_pack_type))
			{
				connection_company_num ++;
			}
		}
	}
	success = ReleaseAccessRight(sem_id);

	printf("\n\n\ncompany_name:|%s|,company_conn_limit: |%d|, current connection num:|%d|, TYPE:|%d|, conn_num:|%d|,conn_upper_limit:|%d|\n\n\n", 
		global_par.company_par_array[company_id].company_name, 
		connection_upper_company_limit, 
		connection_company_num,
		*data_chan_type,
		connection_num,
		connection_upper_limit);
	
	if ((connection_num < connection_upper_limit) && (connection_company_num < connection_upper_company_limit))
	{
		return 1;
	}
	else
	{
		return -1;
	}
	
}

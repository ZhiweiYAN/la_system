/*********************************************************
 *project: Line communication charges supermarket
 *filename: proxy_control.c
 *version: 1.0
 *purpose: prototype of proxy server controller
 *developer: gexiaodan, Xi'an Jiaotong University
 *data: 2007-1-7
 *********************************************************/
#include "proxy_control.h"

/* *************************************************
 *  \brief
 *    do control with business A and B, receive packet from them, which
 *    contains the business link status
 *
 * Function Name:
 * 		int Do_control_with_business(int sem_id, proxy_control_channel *proxy_control_info, int sock_proxy_control, int position)
 *
 * Input:
 *		int sem_id ---> the id of semaphore
 *		proxy_control_channel *proxy_control_info --- > the point of the share memory
 * 		int sock_proxy_control ---> the socket of connected business
 *		int position ---> position saved in business_info_array
 *
 * Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Do_control_with_business(int sem_id, proxy_control_channel *proxy_control_info, int sock_proxy_control, int position)
{
	/*variables for packet to send and receive*/
	char buf_recv[MAX_CONTROL_PACK_SIZE] = "00";
	char buf_send[MAX_CONTROL_PACK_SIZE] = "00";
	int count = 0;
	int busi_break_flag = 0;/*clear break flag for business connection break*/
	int success = 0;

	/*long connection with business server*/
	while (1)
	{
		/*receive packet(contains business's connected sockets) from business, and send database link ip to business*/
		bzero(buf_recv, MAX_CONTROL_PACK_SIZE);
		/*receive control information from business and response*/
		/*check whether send and recv functions are above zero**/
		if ((count = recv(sock_proxy_control, buf_recv, MAX_CONTROL_PACK_SIZE, 0)) > 0)
		{
#ifdef DEBUG_TRANSMIT_FROM_BUSINESS_CONTROL
			printf("Packet recv from business(control channel): length:|%d|, content:|%s|\n", strlen(buf_recv), buf_recv);
#endif

			success = AcquireAccessRight(sem_id);
			strcpy(buf_send, proxy_control_info->busi_to_db_link_ip_address);
			success = ReleaseAccessRight(sem_id);
			if ((count = send(sock_proxy_control, buf_send, strlen(buf_send), 0)) <= 0)
			{
				perror("error@proxy_control.c:Do_control_with_business:send()");
			}

#ifdef DEBUG_TRANSMIT_TO_BUSINESS_CONTROL
			printf("Packet send to business(control channel): length:|%d|, content:|%s|\n", strlen(buf_send), buf_send);
#endif
		}
		else
		{
			perror("error@proxy_control.c:Do_control_with_business:recv()");
		}

		/*store link status from business server*/
		success = AcquireAccessRight(sem_id);
		if (count > 0)   /*business[position] is ok*/
		{
			proxy_control_info->business_info_array[position].link_status = 1;
		}
		else   /*business[position] is down*/
		{
			printf("\033[01;31mControl Process PID_%d of business[%d] will exit because it's recv() or send() returns value <= 0!\033[0m\n", proxy_control_info->business_info_array[position].ctl_pid, position);
			proxy_control_info->business_info_array[position].ctl_pid = 0;
			busi_break_flag = 1;
			proxy_control_info->business_info_array[position].link_status = -1;
		}
		proxy_control_info->business_info_array[position].check_cnt =0;
		success = ReleaseAccessRight(sem_id);

		/*break out of while(1) when recv() or send() <= 0*/
		if (1 == busi_break_flag)
		{
			break;
		}
	}

	return 1;
}

/* *************************************************
 *  \brief
 *    do control with database, receive packet from it, which
 *    contains the data channel link status; and send packet to it, which contains the business status
 *
 * Function Name:
 * 		int Do_control_with_database(int sem_id, proxy_control_channel *proxy_control_info, int sock_proxy_control, int position)
 *
 * Input:
 *		int sem_id ---> the id of semaphore
 *		proxy_control_channel *proxy_control_info --- > the point of the share memory
 * 		int sock_proxy_control ---> the socket of connected data base
 *      int position ---> position saved in database_info_array
 *
 * Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Do_control_with_database(int sem_id, proxy_control_channel *proxy_control_info, int sock_proxy_control, int position)
{
	/*variables for packet to send and receive*/
	char buf_recv[MAX_CONTROL_PACK_SIZE] = "00";
	char buf_send[MAX_CONTROL_PACK_SIZE] = "00";
	int db_break_flag = 0;/*clear break flag for database connection break*/
	int count = 0;
	int i = 0;
	int success = 0;

	/*long connection with database server*/
	while (1)
	{
		/*receive packet(contains link status) from data base, and send a response(contains business link status) to database*/
		/*The format of packet received from database: CUTOFF(HOLDON)"+"ip address*/
		bzero(buf_recv, MAX_CONTROL_PACK_SIZE);
		if ((count = recv(sock_proxy_control, buf_recv, MAX_CONTROL_PACK_SIZE, 0)) > 0)
		{
#ifdef DEBUG_TRANSMIT_FROM_DATABASE_CONTROL
			printf("Packet recv from database(control channel): length:|%d|, content:|%s|\n", strlen(buf_recv), buf_recv);
#endif

			/*construct the packet to send*/
			/*The format of packet sent to database: Bxxx(or NONE)*/
			success = AcquireAccessRight(sem_id);
			bzero(buf_send, MAX_CONTROL_PACK_SIZE);
			for (i=0; i<global_par.system_par.business_number; i++)
			{
				if (proxy_control_info->business_info_array[i].ctl_pid > 0)
				{
					sprintf(buf_send, "B%03d", i);
					goto SEND_TO_DB;
				}
			}
			strcpy(buf_send, "NONE");/*if all of the business servers are down, send NONE*/
SEND_TO_DB:
			success = ReleaseAccessRight(sem_id);

			if ((count = send(sock_proxy_control, buf_send, strlen(buf_send), 0)) <= 0)
			{
				perror("error@proxy_control.c:Do_control_with_database:send()");
			}

#ifdef DEBUG_TRANSMIT_TO_DATABASE_CONTROL
			printf("Packet send to database(control channel): length:|%d|, content:|%s|\n", strlen(buf_send), buf_send);
#endif

		}
		else
		{
			perror("error@proxy_control.c:Do_control_with_database:recv()");
		}

		success = AcquireAccessRight(sem_id);
		if (count > 0)   /*database is ok*/
		{
			/*get the data channel link status from database(HOLDON, CUTOFF, NOWAYS)*/
			if ((0 == strncmp("HOLDON", buf_recv, 6)) && (ACCOFF != proxy_control_info->data_chan_link_status))
			{
				proxy_control_info->data_chan_link_status = HOLDON;
				strcpy(proxy_control_info->busi_to_db_link_ip_address, buf_recv+6);
			}
			else if (0 == strncmp("CUTOFF", buf_recv, 6) && (ACCOFF != proxy_control_info->data_chan_link_status))
			{
				proxy_control_info->data_chan_link_status = CUTOFF;
			}
			//else if (0 == strncmp("NOWAYS", buf_recv, 6))
			//{
			//}
			else if (0 == strncmp("ACCEPTOFF", buf_recv, 9))
			{
				proxy_control_info->data_chan_link_status = ACCOFF;
				db_break_flag = 1;/*short link*/
			}
			else if (0 == strncmp("ACCEPTON", buf_recv, 8))
			{
				proxy_control_info->data_chan_link_status = CUTOFF;
				db_break_flag = 1;/*short link*/
			}
			//else
			//{
			//	printf("\033[01;31mControl Process receives illegal command!\033[0m\n");
			//	db_break_flag = 1;
			//}

			//Added March 21, 2009
			else
			{
				success = Set_terminal_control_table(buf_recv);
				db_break_flag = 1;/*short link*/
			}
		}
		else   /*database is down*/
		{
			printf("\033[01;31mControl Process PID_%d of database will exit because it's recv() or send() returns value <= 0!\033[0m\n", proxy_control_info->database_info_array[position].ctl_pid);
			db_break_flag = 1;
		}
		proxy_control_info->database_info_array[position].check_cnt =0;

		/*break out of while(1) when recv() or send() <= 0*/
		if (1 == db_break_flag)
		{
			proxy_control_info->database_info_array[position].ctl_pid = 0;
			success = ReleaseAccessRight(sem_id);
			break;
		}
		success = ReleaseAccessRight(sem_id);
	}/*while*/

	return 1;

}

/* *************************************************
 *  \brief
 *    handle the proxy control of business A, B and data base
 *
 * Function Name:
 * 		int Handle_proxy_control(int sock_proxy_control, char business_ip_address_array[][16], char *database_self_ip_address, char *database_brother_ip_address)
 *
 * Input:
 * 		int sock_proxy_control ---> the listening socket of proxy control channel
 *		char *business_ip_address_array ---> IP address of business servers
 *		char *database_self_ip_address ---> IP address of self database
 *		char database_brother_ip_address[][16] ---> IP address of brother database
 *
 * Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Handle_proxy_control(int sock_proxy_control, char business_ip_address_array[][16], char *database_self_ip_address, char *database_brother_ip_address)
{
	/*variables for response the query from business or data base*/
	int new_sock_proxy_control;/*connection socket for business or data base*/
	struct sockaddr_in server_addr; /*information for business or data base */
	unsigned int sin_size = 0;
	pid_t pid;
	int success = 0;
	int i = 0;

	/*variable for semaphore and share memory*/
	int sem_id;
	proxy_control_channel *proxy_control_info = NULL;

	while (1)
	{
		sin_size = sizeof(struct sockaddr_in);
		if ((new_sock_proxy_control = accept(sock_proxy_control, (struct sockaddr *)&server_addr, &sin_size)) == -1)
		{
			perror("error@proxy_control.c:Handle_proxy_control:accept()");
			continue;
		}
		if ((pid = fork()) < 0)
		{
			perror("error@proxy_control.c:Handle_proxy_control:fork");
			exit(1);
		}
		else if (0 == pid)
		{
			/*first child, close the listening socket*/
			close(sock_proxy_control);
			if ((pid=fork()) < 0)
			{
				perror("error@proxy_control.c:Handle_proxy_control:fork()");
				exit(1);
			}
			else if (pid > 0)
			{
				exit(0);/*first child exit normally*/
			}

			/* this is the second child process, handle with business or data base*/
			sem_id = GetExistedSemphoreExt(SEM_CTL_FTOK_ID);
			proxy_control_info = (proxy_control_channel *)MappingShareMemOwnSpaceExt(SHM_CTL_FTOK_ID);
			if (server_addr.sin_addr.s_addr == inet_addr(database_self_ip_address))
			{
				printf("Proxy Control Channel: get connection from database(IP:%s)\n",inet_ntoa(server_addr.sin_addr));
				proxy_control_info->database_info_array[0].ctl_pid = getpid();/*save pid*/
				proxy_control_info->database_info_array[0].check_cnt = 0;
				success = ReleaseAccessRight(sem_id);
				Do_control_with_database(sem_id, proxy_control_info, new_sock_proxy_control, 0);
			}
			else if (server_addr.sin_addr.s_addr == inet_addr(database_brother_ip_address))
			{
				printf("Proxy Control Channel: get connection from database(IP:%s)\n",inet_ntoa(server_addr.sin_addr));
				proxy_control_info->database_info_array[1].ctl_pid = getpid();/*save pid*/
				proxy_control_info->database_info_array[1].check_cnt = 0;
				success = ReleaseAccessRight(sem_id);
				Do_control_with_database(sem_id, proxy_control_info, new_sock_proxy_control, 1);
			}
			else
			{
				/*do business control*/
				for (i=0; i< global_par.system_par.business_number; i++)
				{
					if (server_addr.sin_addr.s_addr == inet_addr(business_ip_address_array[i]))
					{
						printf("Proxy Control Channel: get connection from business[%d](IP:%s)\n",i , inet_ntoa(server_addr.sin_addr));
						success = AcquireAccessRight(sem_id);
						proxy_control_info->business_info_array[i].ctl_pid = getpid();/*save pid*/
						success = ReleaseAccessRight(sem_id);
						Do_control_with_business(sem_id, proxy_control_info, new_sock_proxy_control, i);
						break;
					}
				}
			}
			success = UnmappingShareMem((void *)proxy_control_info);
			close(new_sock_proxy_control);
			exit(0);
		}

		/* In the parent process */
		close(new_sock_proxy_control); /*parent process close the new connected socket*/
		/*wait for the first child exit, so that it wouldn't become zombie process*/
		if (waitpid(pid,NULL,0) != pid)
		{
			perror("error@proxy_control.c:Init_control_comm_busi_database_server:waitpid()");
		}
		continue;

	}/*while*/
	return 1;
}

/* *************************************************
 *  \brief
 *    fork a child process to initialize control communication with business
 *	  servers, and databases
 *
 * Function Name:
 * 		int Init_ctl_comm_busi_database_server(int proxy_control_port, char *business_ip_address_array, char *database_primary_ip_address, char *database_secondary_ip_address)
 *
 * Input:
 * 		int proxy_control_port ---> control port of proxy for business and database
 *		char *business_ip_address_array ---> IP address of business server
 *		char *database_self_ip_address ---> IP address of self database
 *		char database_brother_ip_address[][16] ---> IP address of brother database
 *
 * Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Init_ctl_comm_busi_database_server(int proxy_control_port, char business_ip_address_array[][16], char *database_primary_ip_address, char *database_secondary_ip_address)
{
	/*variables for response the query from business or data base*/
	int sock_proxy_control;/*socket for business or data base*/
	struct sockaddr_in local_addr; /*information for local host(proxy server)*/
	pid_t pid;
	int reuse = 1;

	if (-1 == (sock_proxy_control = socket(AF_INET, SOCK_STREAM, 0)))
	{
		perror("error@proxy_control.c:Init_control_comm_busi_database_server:socket()");
		exit(1);
	}
	bzero(&local_addr, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(proxy_control_port);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	if (setsockopt(sock_proxy_control,SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof(int))<0){
		perror("error@proxy_control.c:Init_control_comm_busi_database_server:setsockopt()");
	}

	while (-1 == bind(sock_proxy_control, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)))
	{
		perror("error@proxy_control.c:Init_control_comm_busi_database_server:bind()");
		sleep(1);
	}
	if (-1 == listen(sock_proxy_control, BACKLOG_CONTROL_CHANNEL))
	{
		perror("error@proxy_control.c:Init_control_comm_busi_database_server:listen()");
		exit(1);
	}

	/*fork a process to make up proxy control process*/
	if ((pid = fork()) < 0)
	{
		perror("error@proxy_control.c:Init_control_comm_busi_database_server:fork");
		exit(1);
	}
	else if (0 == pid)
	{
		/*first child*/
		if ((pid=fork()) < 0)
		{
			perror("error@proxy_control.c:Init_control_comm_busi_database_server:fork()");
			exit(1);
		}
		else if (pid > 0)
		{
			exit(0);/*first child exit normally*/
		}
		/*this is second child process*/
		/*begin accept the query from business and data base server*/
		Handle_proxy_control(sock_proxy_control, business_ip_address_array, database_primary_ip_address, database_secondary_ip_address);
	}

	/*in parent process*/
	if (waitpid(pid,NULL,0) != pid)
	{
		perror("error@proxy_control.c:Init_control_comm_busi_database_server:waitpid()");
	}

	/*finish initialization, return normally*/
	return 1;
}

/* *************************************************
 *  \brief
 *    judge the business route status, to A or to B or to none of them
 *
 * Function Name:
 * 		int Judge_business_route(int sem_id, proxy_control_channel *proxy_control_info, int *nsleep_time)
 *
 * Input:
 *		int sem_id ---> the id of semaphore
 *		proxy_control_channel *proxy_control_info --- > the point of the share memory
 *
 * Output:
 *      int *nsleep_time : sleep time used to adjust number of connected clients
 *
 * Return:
 *		-1 ---> none, all of the businesses are down
 *		i>=0 ---> select business[i]
 * *************************************************/
int Judge_business_route(int sem_id, proxy_control_channel *proxy_control_info, int *nsleep_time)
{
	/*variable for business server selection*/
	proxy_control_channel proxy_ctl_info;
	float threshold = 0;
	int number = 0;
	int success = 0;
	int i = 0;
	int busi_ok_num = 0;
	int busi_ok_array[MAX_BUSINESS_NUMBER];

	success = AcquireAccessRight(sem_id);
	proxy_ctl_info = *(proxy_control_info);
	success = ReleaseAccessRight(sem_id);

	for (i=0; i< global_par.system_par.business_number; i++)
	{
		if (proxy_ctl_info.business_info_array[i].link_status > 0)   /*business is OK*/
		{
			busi_ok_array[busi_ok_num] = i;
			busi_ok_num++;
		}
		else
		{
			printf("\033[01;31mBusiness server[%d] is down\033[0m\n", i);
		}
	}

	/*get the threshold and sleep time*/
	if (busi_ok_num > 0)
	{
		threshold = global_par.system_par.business_number*10.0/busi_ok_num;
		*nsleep_time = UNITARY_BUSI_SLEEP_TIME*(global_par.system_par.business_number - busi_ok_num);
	}
	else   /*all of the business servers are down*/
	{
		threshold = -1;
		*nsleep_time = ALL_BUSI_DOWN_SLEEP_TIME;
		printf("\033[01;31mAll of the business servers are down\033[0m\n");
	}

	/*get a random number between 1 and 10*global_par.system_par.business_number*/
	number = 1 + (int)(global_par.system_par.business_number*10.0 * rand() / (RAND_MAX + 1.0));
	/*judge route*/
	if (threshold > 0)
	{
		return busi_ok_array[(int)(number/(threshold+1.0))];
	}
	else
	{
		return -1;
	}
}

/* *************************************************
 *  \brief
 *    get the database link status
 *
 * Function Name:
 * 		 enum data_channel_link_status	Get_data_channel_link_status(int sem_id, proxy_control_channel *proxy_control_info)
 *
 * Input:
 *		int sem_id ---> the id of semaphore
 *		proxy_control_channel *proxy_control_info --- > the point of the share memory
 *
 * Output:
 *
 * Return:
 * 		HOLDON ---> database is ok
 * 		CUTOFF ---> database is down
 * *************************************************/
enum data_channel_link_status Get_data_channel_link_status(int sem_id, proxy_control_channel *proxy_control_info)
{
	int success = 0;
	enum data_channel_link_status data_chan_link_status;

	success = AcquireAccessRight(sem_id);
	/*get the link status*/
	if (HOLDON == proxy_control_info->data_chan_link_status)   /*if hold on*/
	{
		data_chan_link_status = HOLDON;
	}
	else if (CUTOFF == proxy_control_info->data_chan_link_status)   /*if cut off*/
	{
		data_chan_link_status = CUTOFF;
	}
	else   /*if accept off*/
	{
		data_chan_link_status = ACCOFF;
	}
	success = ReleaseAccessRight(sem_id);

	return data_chan_link_status;

}

int Query_trans_enable(char *packet)
{
	int res = 0;
	int sem_id = 0;
	terminal_control_table *terminal_ctl_table = NULL;
	int success = 0;
	char *e = NULL;

	int terminal_id = 0;
	char terminal_id_str[TERMINAL_ID_LENGTH+1];

	int busi_id =0;
	char busi_id_str[COMPANY_ID_LENGTH+1];

	bzero(terminal_id_str,TERMINAL_ID_LENGTH+1);
	bzero(busi_id_str,COMPANY_ID_LENGTH+1);

	//Get the header of packet
	//Get terminal ID
	memcpy(terminal_id_str,packet+TERMINAL_ID_POSITION,TERMINAL_ID_LENGTH);
	//Example: "HS001111" -> 01111
	terminal_id = strtoull(terminal_id_str+3, &e, 10);

	//Get business ID
	memcpy(busi_id_str,packet+COMPANY_ID_POSITION,COMPANY_ID_LENGTH);
	busi_id = strtoull(busi_id_str, &e, 10);;


	/* Initialize terminal control table */
	sem_id = GetExistedSemphoreExt(SEM_TERMINAL_CTRL_TABLE_FTOK_ID);
	terminal_ctl_table = (terminal_control_table *)MappingShareMemOwnSpaceExt(SHM_TERMINAL_CTRL_TABLE_FTOK_ID);
	success = AcquireAccessRight(sem_id);
       	
	if (0!=(((uint64_t)*((uint64_t *)terminal_ctl_table+terminal_id))&(uint64_t)(1<<busi_id)))
	{
		//Allow transaction to be executed
		res = 1;
	}
	else
	{
		//Deny the transaction
		res = 0;
	}


	success = ReleaseAccessRight(sem_id);/*release the right to access the share memory*/
	success = UnmappingShareMem((void *)terminal_ctl_table);

	return res;

}


int Set_terminal_control_table(char *packet)
{
	//return 1;
	int res = 0;
	int i = 0;
	int sem_id = 0;
	terminal_control_table *terminal_ctl_table = NULL;
	int success = 0;
	char *e = NULL;
	uint64_t mask = 0, tmp=0;

	int terminal_id = 0;
	char terminal_id_str[CTL_PKT_TERMINAL_LENGTH+1];

	int busi_id =0;
	char busi_id_str[CTL_PKT_BUSINESS_LENGTH+1];

	char ctl_cmd_str[CTL_PKT_COMMAND_LENGTH+1];

	bzero(terminal_id_str,CTL_PKT_TERMINAL_LENGTH+1);
	bzero(busi_id_str,CTL_PKT_BUSINESS_LENGTH+1);
	bzero(ctl_cmd_str,CTL_PKT_COMMAND_LENGTH+1);
	
	//Get the header of packet
	//Get terminal ID
	memcpy(terminal_id_str,packet+CTL_PKT_TERMINAL_POSITION,CTL_PKT_TERMINAL_LENGTH);
	//Example: "HS000111" -> 00111
	terminal_id = strtoull(terminal_id_str+3, &e, 10);
	//Get business ID
	memcpy(busi_id_str,packet+CTL_PKT_BUSINESS_POSITION,CTL_PKT_BUSINESS_LENGTH);
	busi_id = strtoull(busi_id_str, &e, 10);	
	//Get command
	memcpy(ctl_cmd_str,packet+CTL_PKT_COMMAND_POSITION,CTL_PKT_COMMAND_LENGTH);


	/* Initialize terminal control table */
	sem_id = GetExistedSemphoreExt(SEM_TERMINAL_CTRL_TABLE_FTOK_ID);
	terminal_ctl_table = (terminal_control_table *)MappingShareMemOwnSpaceExt(SHM_TERMINAL_CTRL_TABLE_FTOK_ID);
	success = AcquireAccessRight(sem_id);

	if (0==memcmp(terminal_id_str, "AAAAAAAA", sizeof("AAAAAAAA")))
	{
		//All terminals,
		if (0==memcmp(busi_id_str,"AAA",sizeof("AAA")))
		{
			//all terminals, all busi
			if (0==memcmp(ctl_cmd_str,"ENA", sizeof("ENA")))
			{
				memset(terminal_ctl_table,0xFF,sizeof(terminal_control_table));
			}
			else if (0==memcmp(ctl_cmd_str,"DIS",sizeof("DIS")))
			{
				memset(terminal_ctl_table,0x00,sizeof(terminal_control_table));
			}
			else
			{
				//do nothing
				return res;
			}
		}//end of "if AAA"
		else
		{
			//All terminal, one busi
			if (0==memcmp(ctl_cmd_str,"ENA", sizeof("ENA")))
			{
				mask = (uint64_t)(1<<busi_id);
				for (i=0;i<MAX_TERMINAL_NUM;i++)
				{
					tmp = (uint64_t)*((uint64_t *)terminal_ctl_table+i); 
					tmp = tmp|mask;
					*((uint64_t *)terminal_ctl_table+i) = tmp;
				}
			}
			else if (0==memcmp(ctl_cmd_str,"DIS",sizeof("DIS")))
			{
				mask = (uint64_t)(~(1<<busi_id));
				for (i=0;i<MAX_TERMINAL_NUM;i++)
				{
					tmp = (uint64_t)*((uint64_t *)terminal_ctl_table+i); 
					tmp = tmp&mask;
					*((uint64_t *)terminal_ctl_table+i) = tmp;
				}
			}
			else
			{
				//do nothing
				return res;
			}
		}
	}//end of "if AAAAAAAA"
	else
	{
		//One terminal
		if (0==memcmp(busi_id_str,"AAA",sizeof("AAA")))
		{
			//One terminal, all busi
			if (0==memcmp(ctl_cmd_str,"ENA",sizeof("ENA")))
			{
				memset((char*)terminal_ctl_table+terminal_id*sizeof(uint64_t),0xFF,sizeof(uint64_t));
			}
			else if (0==memcmp(ctl_cmd_str,"DIS",sizeof("DIS")))
			{
				//tmp = (uint64_t)*((uint64_t *)terminal_ctl_table+terminal_id); 
				//printf("Number is:|%lu|\n",tmp);
				memset((char*)terminal_ctl_table+terminal_id*sizeof(uint64_t),0x0,sizeof(uint64_t));
				
			}
			else
			{
				//do nothing
				return res;
			}
		}//end of "if AAA"
		else
		{
			//One terminal, one busi
			if (0==memcmp(ctl_cmd_str,"ENA",sizeof("ENA")))
			{
				mask = (uint64_t)(1<<busi_id);
				tmp = (uint64_t)*((uint64_t *)terminal_ctl_table+terminal_id); 
				//printf("ENA:before:Number is:|%lu|\n",tmp);
				tmp = tmp|mask;
				*((uint64_t *)terminal_ctl_table+terminal_id) = tmp;
				//printf("ENA:after:Number is:|%lu|\n",*((uint64_t *)terminal_ctl_table+terminal_id));
			}
			else if (0==memcmp(ctl_cmd_str,"DIS",sizeof("DIS")))
			{
				mask = (uint64_t)(~(1<<busi_id));
				tmp = (uint64_t)*((uint64_t *)terminal_ctl_table+terminal_id); 
				//printf("DIS:before:Number is:|%lu|\n",tmp);
				tmp = tmp&mask;
				*((uint64_t *)terminal_ctl_table+terminal_id) = tmp;
				//printf("DIS:after:Number is:|%lu|\n",*((uint64_t *)terminal_ctl_table+terminal_id));
			}
			else
			{
				//do nothing
				return res;
			}			
		}
	}
	success = ReleaseAccessRight(sem_id);/*release the right to access the share memory*/
	success = UnmappingShareMem((void *)terminal_ctl_table);

	return res;

}


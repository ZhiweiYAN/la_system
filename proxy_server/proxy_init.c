/* ************************************************** 
 * File name:
 * 		proxy_init.c
 * Desciption:
 * 		The file contains the initilizing codes for proxy
 * 		server. It is the first step of all services
 * Coder:
 * 		YanZhiwei, jerod.yan@gmail.com
 *      gexiaodan, Xi'an Jiaotong University
 * 		Drum Team
 * Date:
 * 		2006-12-26
 * **************************************************/
#include "proxy_init.h"

/* ************************************************** 
 * Function name:
 * 		int Init_proxy_server(void);
 * Input:
 * 		NONE
 * Output:
 * 		1 ---> success;
 * 		-1 ---> failure;
 * **************************************************/
int Init_proxy_server(void)
{
	int sd_to_client = 0;

	struct sockaddr_in sa_client;

	char cfg_fname[FILE_NAME_LENGTH];

	int i = 0;

	int success = 0;
	int sem_id = 0;
	terminal_control_table* terminal_ctl_table = NULL;
	
	/* Open the config file of the proxy server to set value */
	printf("Read configuration file:");
	strcpy(cfg_fname, CONFIG_FILENAME);
	strcpy(global_par.execute_path_name, program_invocation_name);
	strcpy(global_par.execute_path_short_name, program_invocation_short_name);
	if(1 == ReadConfigAll(cfg_fname))
	{
		OUTPUT_OK;
	}
	else
	{
		OUTPUT_ERROR;
		perror("error@proxy_init.c:ReadConfig\n");
		return -1;
	}

	/*Initialize monitor channel for data channel and control channel*/
	printf("Initialize monitor channel:");
	fflush(NULL);
	if(-1 == Init_monitor_channel())	
	{
		perror("error@proxy_init.c:Init_monitor_channel\n");
		OUTPUT_ERROR;
		return -1;
	}
	else
	{
		OUTPUT_OK;
	}

	/* Initialize control communication with database and business server*/
	printf("Initialize the control communication with business server and database:");
	fflush(NULL);
	if(-1 == Init_ctl_comm_busi_database_server(global_par.system_par.proxy_control_port, 
		global_par.system_par.business_ip_addr_array, 
		global_par.system_par.database_self_ip_address, global_par.system_par.database_brother_ip_address))	
	{
		perror("error@proxy_init.c:Init_com_busi_server\n");
		OUTPUT_ERROR;
		return -1;
	}
	else
	{
		OUTPUT_OK;
	}

	/* Initialize data communication with business server*/
	struct sockaddr_in sa_data_business_array[global_par.system_par.business_number];
	for (i=0; i<global_par.system_par.business_number; i++)
	{
		printf("Initialize the data communication with business server[%d]:", i);
		if(-1==Init_data_comm_busi_server(global_par.system_par.business_ip_addr_array[i],global_par.system_par.business_data_port,&sa_data_business_array[i]))
		{
			perror("error@proxy_init.c:Init_com_busi_server\n");
			OUTPUT_ERROR;
			return -1;
		}
		else
		{
			OUTPUT_OK;
		}
	}

	/* Initialize communication with clients */
	printf("Initialize the data communication with clients:");
	if((sd_to_client=Init_data_comm_client(global_par.system_par.proxy_data_port,&sa_client))<0)
	{
		perror("error@proxy_init.c:Init_data_com_client\n");
		OUTPUT_ERROR;
		return -1;
	}
	else
	{
		OUTPUT_OK;
	}

//Added March 21, 2009
	/* Initialize terminal control table */
	printf("Initialize the terminal control table:");
   	sem_id = InitialSem(SEM_TERMINAL_CTRL_TABLE_FTOK_ID);
   	terminal_ctl_table = (terminal_control_table *)InitialShm(sizeof(terminal_control_table), SHM_TERMINAL_CTRL_TABLE_FTOK_ID);
	sem_id = GetExistedSemphoreExt(SEM_TERMINAL_CTRL_TABLE_FTOK_ID);
	terminal_ctl_table = (terminal_control_table *)MappingShareMemOwnSpaceExt(SHM_TERMINAL_CTRL_TABLE_FTOK_ID);
	success = AcquireAccessRight(sem_id);
	memset(terminal_ctl_table, 0xFF, sizeof(terminal_control_table));
	success = ReleaseAccessRight(sem_id);/*release the right to access the share memory*/
	success = UnmappingShareMem((void *)terminal_ctl_table);


	/* Enter the normal data transferring time */
	/* Start the daemon process */
	Daemon_foreground(sd_to_client,&sa_client,sa_data_business_array);

	return 1;
}

 /* ************************************************* 
 * Function Name: 
 * 		int Init_data_comm_busi_server(char *business_ip_address,int business_data_port, struct sockaddr_in *sa)
 * Input: 
 * 		char *business_ip_address ---> IP address of business 
 * 		int business_data_port ---> data port of business
 * Output:
 * 		struct sockaddr_in *sa ---> the initialized data
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Init_data_comm_busi_server(const char *business_ip_address,int business_data_port, struct sockaddr_in *sa)
{
	/* Initialize the structure of socket */
	bzero(sa,sizeof(struct sockaddr_in));
	sa->sin_family = AF_INET;
	sa->sin_port = htons(business_data_port);

	sa->sin_addr.s_addr = inet_addr(business_ip_address);
	return 1;
}

/* ************************************************* 
 * Function Name: 
 * 		int Init_data_comm_client(int client_data_port, struct sockaddr_in &sa)
 * Input: 
 * 		int client_data_port ---> Open the port for receiving client connection
 * Output: 
 * 		socket description ---> success
 * 		-1 ---> error
 * *************************************************/
int Init_data_comm_client(int client_data_port,struct sockaddr_in *sa)
{
	int sd_to_clients = 0;
	int reuse =1;
	
	/* Check input parameters */
	if(client_data_port<1024){
		perror("error:proxy_init.c:Init_data_comm_client()\n");
		return -1;
	}

	/* Prepare the socket */
	if((sd_to_clients = socket(AF_INET,SOCK_STREAM,0))<0){
		perror("error:proxy_init.c:Init_data_com_client():sd_to_clients\n");
		return -1;
	}
	bzero(sa,sizeof(struct sockaddr_in));
	sa->sin_family = AF_INET;
	sa->sin_port = htons(client_data_port);
	if(INADDR_ANY)
		sa->sin_addr.s_addr = htonl(INADDR_ANY);

	if (setsockopt(sd_to_clients,SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof(int))<0){
		perror("error:proxy_int.c:Init_data_com_client():setsockopt()");
	}
	
	/*!	permit any IP address to send request */
	while (bind(sd_to_clients,(struct sockaddr *) sa, sizeof(struct sockaddr))<0){
		perror("error:proxy_int.c:Init_data_com_client():bind");
		sleep(5);
	}

	/* Listen the port for clients */
	listen(sd_to_clients, BACKLOG_DATA_CHANNEL);

	return sd_to_clients;
}

/* ************************************************* 
 * Function Name: 
 * 		int Daemon_foreground(int sd_to_clients, struct sockaddr_in *sa_server_array)
 * Input: 
 * 		int sd_to_clients ---> for clients 
 * Ouput: 
 * 		-1 ---> error
 * 		Not -1 ---> success
 * *************************************************/
int Daemon_foreground(int sd_to_clients,struct sockaddr_in *sa_clients,
						struct sockaddr_in *sa_server_array)
{
	char buf_pack[MAXPACKETSIZE] = "00";
	char error_info[INTERAL_ERROR_INFO_LENGTH_AT_HEADER + 1] = "00";
	int count = 0;
	pid_t pid = 0;

	socklen_t len = 0;
	struct sockaddr_in *sa_server = NULL;
	int nsd_to_clients = 0;
	int sd_to_server = 0;

	int business_route = -1; /*route to none of the business servers*/
	int nsleep_time = 0;
	int j = 0;
	int success = 0;
	/*variable for semaphore and share memory*/
	int sem_ctl_id;
	proxy_control_channel *proxy_control_info = NULL;
	int sem_data_id;
	proxy_data_channel *proxy_data_info = NULL;
	data_channel_packet_type data_chan_type = UNKNOWN_PACKET;
	data_channel_link_status data_chan_link_status = UNKNOWN_STATUS;

	/*form error info*/
	bzero(error_info, INTERAL_ERROR_INFO_LENGTH_AT_HEADER + 1);
	memcpy(error_info, global_par.system_par.localhost_id, LOCALHOST_ID_LENGTH);

	/*get existed semaphore and share memory*/
	sem_ctl_id = GetExistedSemphoreExt(SEM_CTL_FTOK_ID);
   	proxy_control_info = (proxy_control_channel *)MappingShareMemOwnSpaceExt(SHM_CTL_FTOK_ID);

	len = sizeof (struct sockaddr);
	while(1)
	{	
		//printf("sd before accept: |%d|\n", sd_to_clients);
		if(( nsd_to_clients = accept(sd_to_clients,(struct sockaddr*)sa_clients, &len))<0)
		{
			//printf("sd after accept: |%d|\n", sd_to_clients);
			perror("error:proxy_init.c:Daemon_foreground():accept");
			continue;
		}
		data_chan_link_status = Get_data_channel_link_status(sem_ctl_id, proxy_control_info);
		if (HOLDON == data_chan_link_status)/*link status is hold on*/
		{	
			printf("\033[01;32mDatabase indicate HOLD ON!\033[0m\n");
			j = rand();/*make sure the that child process get the "real" rand data when uses rand()*/
		
			/* Determine the route plan to business servers, and get the link status*/
			business_route = Judge_business_route(sem_ctl_id, proxy_control_info, &nsleep_time);
			if (business_route >= 0)/* one of business servers is ok at least*/
			{
				printf("Route to business server[%d]\n", business_route);
				sa_server = &sa_server_array[business_route];
			}
			else/*connect to none if all of the business servers are down*/
			{
				printf("Route to none of business servers\n");
				
				/*tell the client wrong information*/
				/*get error info*/
				bzero(error_info+LOCALHOST_ID_LENGTH, INTERAL_ERROR_INFO_LENGTH_AT_HEADER-LOCALHOST_ID_LENGTH+1);
				memcpy(error_info+LOCALHOST_ID_LENGTH, BUSINESS_ERROR_INFO, strlen(BUSINESS_ERROR_INFO));
				
				/*recv the packet from client*/
				bzero(buf_pack,MAXPACKETSIZE);
				count = recv(nsd_to_clients,buf_pack,MAXPACKETSIZE,0);

				/*fill the internal error flag and information*/
				memcpy(buf_pack + INTERAL_SUCCESS_FLAG_START_POSITION_AT_HEADER, "01", INTERAL_SUCCESS_FLAG_LENGTH_AT_HEADER);
				memcpy(buf_pack + INTERAL_ERROR_INFO_START_POSITION_AT_HEADER, error_info, strlen(error_info));
				
				/*send the packet to client*/
				count = send(nsd_to_clients,buf_pack,strlen(buf_pack),0);

				close(nsd_to_clients);
				sleep(nsleep_time);
				continue;
			}

			/*fork a child process*/
			if((pid = fork()) < 0)
			{
				perror("error:proxy_init.c:Dameon_foreground():fork");
				close(nsd_to_clients);
				sleep(1);
				continue;
			}
			else if (0 == pid)
			{
				/* In the first child process*/
				/* Close the listening socket description */
				close(sd_to_clients);
				if((pid = fork()) < 0)
				{
					perror("proxy_init.c:Daemon_foreground():fork()");
					exit(1);
				}
				else if (pid > 0)
				{
					exit(0);
				}
				else
				{
					/* In the second child process */
					bzero(buf_pack,MAXPACKETSIZE);
					
					/*get existed semaphore and share memory*/
					sem_data_id = GetExistedSemphoreExt(SEM_DATA_FTOK_ID);
   					proxy_data_info = (proxy_data_channel *)MappingShareMemOwnSpaceExt(SHM_DATA_FTOK_ID);

					/* CLIENTS:Receving data from clients */
					count = recv(nsd_to_clients,buf_pack,MAXPACKETSIZE,0);
					printf("PID %d response one client\n",getpid());

					//added March 21, 2009
					//Judge whether the transaction can be executed or not
					if(0 ==Query_trans_enable(buf_pack))
					{
						//Deny
						/*get error info*/
						bzero(error_info+LOCALHOST_ID_LENGTH, INTERAL_ERROR_INFO_LENGTH_AT_HEADER-LOCALHOST_ID_LENGTH+1);
						memcpy(error_info+LOCALHOST_ID_LENGTH, ADMISSION_CONTROL_INFO, strlen(ADMISSION_CONTROL_INFO));
						
						/*fill the internal error flag and information*/
						memcpy(buf_pack + INTERAL_SUCCESS_FLAG_START_POSITION_AT_HEADER, "01", INTERAL_SUCCESS_FLAG_LENGTH_AT_HEADER);
						memcpy(buf_pack + INTERAL_ERROR_INFO_START_POSITION_AT_HEADER, error_info, strlen(error_info));
						
						/*send the packet to client*/
						count = send(nsd_to_clients,buf_pack,strlen(buf_pack),0);

						close(nsd_to_clients);
						exit(0);
					}
					/*judge whether reach the upper limit*/
					if (-1 == Judge_connection_limit(sem_data_id, proxy_data_info, buf_pack, &data_chan_type))
					{
						/*reach the connection limit*/
						/*get error info*/
						bzero(error_info+LOCALHOST_ID_LENGTH, INTERAL_ERROR_INFO_LENGTH_AT_HEADER-LOCALHOST_ID_LENGTH+1);
						memcpy(error_info+LOCALHOST_ID_LENGTH, UP_CONNECTION_LIMIT_INFO, strlen(UP_CONNECTION_LIMIT_INFO));
						
						/*fill the internal error flag and information*/
						memcpy(buf_pack + INTERAL_SUCCESS_FLAG_START_POSITION_AT_HEADER, "01", INTERAL_SUCCESS_FLAG_LENGTH_AT_HEADER);
						memcpy(buf_pack + INTERAL_ERROR_INFO_START_POSITION_AT_HEADER, error_info, strlen(error_info));
						
						/*send the packet to client*/
						count = send(nsd_to_clients,buf_pack,strlen(buf_pack),0);

						close(nsd_to_clients);
						exit(0);
					}
					
					/*save the child pid to the share memory*/
					Save_data_chan_info(sem_data_id, proxy_data_info, data_chan_type, buf_pack);

#ifdef DEBUG_TRANSMIT_FROM_CLIENT_DATA
					printf("Packet recv from client(data channel): length:|%d|, content:|%s|\n", strlen(buf_pack), buf_pack);
#endif

					/* BUSINESS:Prepare the socket for server */
					if((sd_to_server = socket(AF_INET,SOCK_STREAM,0))<0)
					{
						perror("error:proxy_init.c:Daemon_foreground():socket");
						exit(1);
					}

					/* BUSINESS:Connect the server */
					if(0 > connect_server_retry(sd_to_server,(struct sockaddr *) sa_server, len))
					{
						perror("error:proxy_init.c:Daemon_foreground():connect");
						exit(1);
					}

					/* BUSINESS:Send the data to server */
					count = multi_send(sd_to_server,buf_pack,strlen(buf_pack),0);
					printf("PID %d re-transmit the data to business\n",getpid());

#ifdef DEBUG_TRANSMIT_TO_BUSINESS_DATA
					printf("Packet send to business(data channel): length:|%d|, content:|%s|\n", strlen(buf_pack), buf_pack);
#endif

					/* BUSINESS:Receive the reponse data from server */
					bzero(buf_pack, MAXPACKETSIZE);
					count = multi_recv(sd_to_server,buf_pack,MAXPACKETSIZE,0);
					close(sd_to_server);
					printf("PID %d receive the data from business\n",getpid());

#ifdef DEBUG_TRANSMIT_FROM_BUSINESS_DATA
					printf("Packet recv from business(data channel): length:|%d|, content:|%s|\n", strlen(buf_pack), buf_pack);
#endif

					/* CLIENTS:Send messages to clients */
					count = send(nsd_to_clients,buf_pack,strlen(buf_pack),0);
					printf("PID %d send the result to client\n",getpid());

#ifdef DEBUG_TRANSMIT_TO_CLIENT_DATA
					printf("Packet send to client(data channel): length:|%d|, content:|%s|\n", strlen(buf_pack), buf_pack);
#endif

					/*delete the child pid from the share memory*/
					Delete_data_chan_info(sem_data_id, proxy_data_info);
					
					/*unmap share memory*/
					success = UnmappingShareMem((void *)proxy_data_info);

					/* Free all resources */
					close(nsd_to_clients);
					exit(0);
				}
			}			
			/* Close the connection socket description */
			close(nsd_to_clients);
			/* In the parent process */
			if(waitpid(pid,NULL,0)!=pid)
			{
				perror("proxy_init.c:Daemon_foreground():waitpid");
			}
			usleep(nsleep_time);
			continue;
		}
		else/*link status is cut off or accept off*/
		{
			if (CUTOFF == data_chan_link_status)
			{
				printf("\033[01;31mDatabase indicate cut off!\033[0m\n");
			} 
			else
			{
				printf("\033[01;35mAdministrator indicate cut off!\033[0m\n");
			}
			
			/*tell the client wrong information*/
			/*get error info*/
			bzero(error_info+LOCALHOST_ID_LENGTH, INTERAL_ERROR_INFO_LENGTH_AT_HEADER-LOCALHOST_ID_LENGTH+1);
			memcpy(error_info+LOCALHOST_ID_LENGTH, DATABASE_CHECK_INFO, strlen(DATABASE_CHECK_INFO));
				
			/*recv the packet from client*/
			bzero(buf_pack,MAXPACKETSIZE);
			count = recv(nsd_to_clients,buf_pack,MAXPACKETSIZE,0);

			/*fill the internal error flag and information*/
			memcpy(buf_pack + INTERAL_SUCCESS_FLAG_START_POSITION_AT_HEADER, "01", INTERAL_SUCCESS_FLAG_LENGTH_AT_HEADER);
			memcpy(buf_pack + INTERAL_ERROR_INFO_START_POSITION_AT_HEADER, error_info, strlen(error_info));
				
			/*send the packet to client*/
			count = send(nsd_to_clients,buf_pack,strlen(buf_pack),0);
			close(nsd_to_clients);
				
			continue;
		}
	}
	success = UnmappingShareMem((void *)proxy_control_info);/*unmap the existed share memory*/
}

/* *************************************************
 * Function Name: 
 * 		int Connect_Server_Retry(int sockfd, const struct sockaddr *addr, socklen_t alen)
 *
 * Input:
 *		int sockfd: the socket for setup the control channel with proxy
 *		const struct sockaddr *addr: the structure contain the destination IP and port
 *		socklen_t alen: the length of the structure sockaddr 
 *
 * Output:
 *		none
 *
 * Return:
 * 		1 ---> success
 * 		0 ---> error
 * *************************************************/
int connect_server_retry(int sockfd, const struct sockaddr *addr, socklen_t alen)
{
	int nsec;

	for(nsec = 1; nsec <= MAXSLEEP; nsec <<=1){
		if(connect(sockfd,addr,alen)==0){
			return 0;
		}
		if(nsec<=MAXSLEEP/2)
			sleep(nsec);
	}
	return -1;
}

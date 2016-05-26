/* *************************************************
 * File name:
 * 		bk_init.cc
 * Description:
 * 		The program is run at the database server.
 * 		It initilizes all child process.
 * 		After that, the bk_server begins to work.
 * Author:
 * 		Zhiwei Yan, jerod.yan@gmail.com
 * Date:
 * 		2007-07-07
 * *************************************************/
#include "bk_init.h"

/* *************************************************
* Function Name: 
* 		int Init_parameters()
* Input: 
* 		all parameters;
* Output:
* 		1 ---> success
* 		-1 ---> failure
* *************************************************/
int Init_parameters(void)
{
	int success = 0;

	/* decide whether it is DA database */
	if(0!=strcmp(global_par.system_par.database_self_ip_address,global_par.system_par.localhost_ip_address)) {
		strcpy(global_par.system_par.database_brother_ip_address,global_par.system_par.database_self_ip_address);
		strcpy(global_par.system_par.database_self_ip_address,global_par.system_par.localhost_ip_address);
	}

	return success;
}


/* *************************************************
 * Function Name: 
 * 		int Init_finance_control_process(int port,int *welcome_sd, struct sockaddr_in &sa)
 * Input: 
 * 		int *welcome_sd ---> the socket for accept connections
 * 		struct sockaddr_in *sa ---> the pointer of socket address
 * Ouput:
 *		1 ---> success
 *		-1 ---> failure
 * *************************************************/
int Init_finance_control_process(int port,int *welcome_sd, struct sockaddr_in *sa)
{
	int reuse =1;
	/* Check input parameters */
	if(1024>port||NULL==welcome_sd||NULL==sa)
	{
		perror("error@:port_001");
		return -1;
	}

	/* Create a socket */
	if((*welcome_sd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("error@():socket_002");
		return -1;
	}

	/* Bind the socket to the port, and allow all IP connect with it */
	bzero(sa,sizeof(struct sockaddr_in));
	sa->sin_family = AF_INET;
	sa->sin_port = htons(port);
	if(INADDR_ANY)
		sa->sin_addr.s_addr = htonl(INADDR_ANY);
	if (setsockopt(*welcome_sd,SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof(int))<0){
		perror("error:bk_init.c:Init_finance_control_process():setsockopt()");
	}
	while(1)
	{
		if(0==bind(*welcome_sd,(struct sockaddr *)sa,sizeof(struct sockaddr_in))) {
			/* success for bind operation */
			break;
		} else {
			sleep(5);
			printf("\nContinuing Binding comm_port for business machines \n");
			continue;
		}
	}

	listen(*welcome_sd,BACKLOG);
	return 1;
}

/* *************************************************
 * Function Name: 
 * 		int Init_monitor_process(void)
 * Input: 
 * 		NONE;
 * Ouput: 
 * 		1 ---> success
 * 		-1 ---> failure
 * *************************************************/
int Init_monitor_process(void)
{
	pid_t pid = 0;
	void * mem_process_ptr = NULL;

	int success = 0;
	int semid_process = 0;
	int kill_flag = 0;
	if((pid = fork()) < 0)
	{
		perror("error:bk_init.c:Init_monitor_process():fork");
		return -1;
	}
	else if (0 == pid)
	{
		/* In the first child process*/
		if((pid = fork()) < 0)
		{
			perror("error:bk_init.c:Init_monitor_process():fork2");
			exit(1);
		}
		else if (pid > 0)
		{
			exit(0);/*first child exit normally*/
		}
		
		/* this is the second child process*/
		semid_process = GetExistedSemphoreExt(SEM_PROCESS_FTOK_ID);
		mem_process_ptr = MappingShareMemOwnSpaceExt(SHM_PROCESS_FTOK_ID);
		while(1)
		{
			/* Kill all invalid process */
			success = AcquireAccessRight(semid_process);
			kill_flag = Kill_invalid_process(((struct ShareMemProcess *)mem_process_ptr)->process_table,MAX_PROCESS_NUMBRER);
			success = ReleaseAccessRight(semid_process);

			/* wait for constant seconds, and check it again. */
			sleep(DELAY_MONITOR_TIME);
		}

		/*exit unormal*/
		success = UnmappingShareMem((void*)mem_process_ptr);
		exit(0);
	}
	
	if(waitpid(pid, NULL, 0)!=pid)
	{
		perror("bk_init.c:Init_monitor_process:waitpid");
	}

	return 1;
}



/* *************************************************
 * Function Name: 
 * 		int Init_counter_process(void)
 * Input: 
 * 		NONE;
 * Ouput: 
 * 		1 ---> success
 * 		-1 ---> failure
 * *************************************************/
int Init_counter_process(void)
{
	pid_t pid = 0;
	void * mem_ptr = NULL;
	int success = 0;
	int semid = 0;
	if((pid = fork()) < 0) 
	{
		perror("error:bk_init.c:Init_counter_process():fork");
		return -1;
	}
	else if (0 == pid)
	{
		/* In the first child process*/
		if((pid = fork()) < 0)
		{
			perror("error:bk_init.c:Init_monitor_process():fork2");
			exit(1);
		}
		else if (pid > 0)
		{
			exit(0); 		/* Return original parent */
		}

		semid = GetExistedSemphoreExt(SEM_PROCESS_FTOK_ID);
		mem_ptr = MappingShareMemOwnSpaceExt(SHM_PROCESS_FTOK_ID);
		while(1)
		{
			success = AcquireAccessRight(semid);
			/* increasing the lifetime of the processes. */
			success = Increase_process_life_time(((struct ShareMemProcess *)mem_ptr)->process_table,MAX_PROCESS_NUMBRER);
			success = ReleaseAccessRight(semid);
			sleep(1);
		}
		
		/*exit unnormal*/
		success = UnmappingShareMem((void*)mem_ptr);
		exit(0);
	}

	if(waitpid(pid, NULL, 0)!=pid)
	{
		perror("bk_init.c:Init_monitor_process:waitpid");
	}

	return 1;
}


/* *************************************************
 * Function Name: 
 * 		int Init_auto_renew_deposit_process(void)
 * Input: 
 * 		NONE;
 * Ouput: 
 * 		1 ---> success
 * 		-1 ---> failure
 * *************************************************/
int Init_auto_renew_deposit_process(void)
 {
 	pid_t pid = 0;

	/* Start renew_process */	
	if((pid = fork()) < 0)
	{
		perror("error:bk_init.c:Init_renew_deposit_process():fork");
		return -1;
	} 
	else if (0 == pid)
	{
		/* In the first child process*/
		if((pid = fork()) < 0)
		{
			perror("error:bk_init.c:Init_renew_deposit_process():fork2");
			exit(1);
		}
		else if (pid > 0)
		{
			exit(0); 		/* Return original parent */
		}

		/* Start renew Database A deposit */
		Insert_pid_process_table(getpid(), RENEW_DEPOSIT_PROCESS_DEADLINE, RENEW_DEPOSIT_PROCESS);
		/* You add codes here */
		//Do_auto_refresh_deposit_process();
		exit(0);
	}
	
	if(waitpid(pid, NULL, 0)!=pid)
		perror("bk_init.c:Init_renew_deposit_process:waitpid");
	
	return 1;
 	
 }
 
 
 /* *************************************************
 * Function Name: 
 * 		int Init_auto_renew_balance_control_process(void)
 * Input: 
 * 		NONE;
 * Ouput: 
 * 		1 ---> success
 * 		-1 ---> failure
 * *************************************************/
int Init_auto_renew_balance_process(void)
 {
 	pid_t pid = 0;

	/* Start renew_process */	
	if((pid = fork()) < 0)
	{
		perror("error:bk_init.c:Init_renew_deposit_process():fork");
		return -1;
	} 
	else if (0 == pid)
	{
		/* In the first child process*/
		if((pid = fork()) < 0)
		{
			perror("error:bk_init.c:Init_renew_deposit_process():fork2");
			exit(1);
		}
		else if (pid > 0)
		{
			exit(0); 		/* Return original parent */
		}

		/* Start renew Database A deposit */
		Insert_pid_process_table(getpid(), RENEW_BALANCE_PROCESS_DEADLINE, RENEW_BALANCE_PROCESS);
		/* You add codes here */
		//Do_auto_refresh_balance_process();
		exit(0);
	}
	
	if(waitpid(pid, NULL, 0)!=pid)
		perror("bk_init.c:Init_renew_deposit_process:waitpid");
	
	return 1;
	
 }
  
 /* *************************************************
 * Function Name: 
 *		int Daemon_finance_control_server(int welcome_sd,struct sockaddr_in *sa_business)
 * Input:
 * 		int welcome_sd ---> Listen socket description
 * Ouput: 
 * 		-1 ---> failure
 * 		1 ---> success
 * *************************************************/
int Daemon_finance_control_server(int welcome_sd,struct sockaddr_in *sa_business)
{
	char buf_recv[MAX_PACKET_SIZE] = "00";
	char buf_send[MAX_PACKET_SIZE] = "00";
	int count = 0;
	pid_t pid = 0;
	int success = 0;

	int connection_sd = 0;

	socklen_t len = 0;
    
    /*execution time*/
	struct timeval tpstart,tpend;
	float timeuse = 0;

	int sem_id = 0;
	struct UpdateProtection *mem_protection_ptr = NULL;

	/*Check the input parameters*/
	if(0==welcome_sd)
	{
		perror("error:bk_init.c:Daemon_db_server():welcome_sd");
	}

	len = sizeof (struct sockaddr);

	/* Initialize protection semaphore and One block memory for process*/
	sem_id = InitialSem(SEM_PROTECTION_FTOK_ID);
   	mem_protection_ptr = (struct UpdateProtection *)InitialShm(sizeof(struct UpdateProtection), SHM_PROTECTION_FTOK_ID);
	bzero(mem_protection_ptr, sizeof(struct UpdateProtection));

	/* Enter the Daemon */
	while(1)
	{
		fflush(NULL);
		printf("\nThe daemon process of finance control is waiting for connections .... \n");
		if(( connection_sd = accept(welcome_sd,(struct sockaddr*)sa_business,&len))<0)
		{
			perror("error:bk_init.c:Daemon_finance_control_server():accept");
			continue;
		}

		fflush(NULL);
		if((pid = fork()) < 0) 
		{
			perror("error:bk_init.c:Daemon_finance_control_server():fork");
			close(connection_sd);
			continue;
		} 
		else if (0 == pid) 
		{

			/* In the first child process*/
			/* Close the listening socket description */
			close(welcome_sd);
			if((pid = fork()) < 0)
			{
				perror("error:bk_init.c:Daemon_finance_control_server():fork2");
				exit(1);
			}
			else if (pid > 0)
			{
				exit(0); 		/* Return original parent */
			}

			/* In the grandchild process */
			success = Insert_pid_process_table(getpid(),ACCOUNT_SERVER_PROCESS_DEADLINE,ACCOUNT_SERVER_PROCESS);

			/* BUSINESS:Receiving data from business servers*/
			bzero(buf_recv, MAX_PACKET_SIZE);
			if (0 >= (count = recv(connection_sd,buf_recv, MAX_PACKET_SIZE, 0)))
			{
				perror("error:bk_init.c:Daemon_finance_control_server():recv");
				close(connection_sd);
				success = Remove_pid_process_table(getpid());
				exit(1);
			}
			

#if _PRIMARY_FININACE_CONTROL_DEBUG_ > 5
			printf("\nFININACE_CONTROL Process From Terminal :\n|%s|\n",buf_recv);
#endif

			printf("\rPID %d response one business FININACE_CONTROL request.\n",getpid());


			/*start time*/
            		gettimeofday(&tpstart,NULL);

			/* Add implementation here */
			success = Do_database_fininace_control_procedures(connection_sd, buf_recv, count);
			
			/*end time*/
            		gettimeofday(&tpend,NULL);
			/*calculate the time*/
            		timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec) + tpend.tv_usec-tpstart.tv_usec;
            		timeuse/=1000000;
            		printf("\t\033[32mResponse financial packet successfully, used time:%f!\033[0m\n", timeuse);

			bzero(buf_send, MAX_PACKET_SIZE);
			if (1 == success)
			{
				strcpy(buf_send, "SUCCESS");
			}
			else
			{
				strcpy(buf_send, "FAILURE");
			}

			if(send(connection_sd, buf_send, strlen(buf_send),0)==-1)
        		{
            			perror("error@bk_init.c:Daemon_finance_control_server:send");
				close(connection_sd);
				success = Remove_pid_process_table(getpid());
            			exit(1);
        		}

			close(connection_sd);
			success = Remove_pid_process_table(getpid());
			exit(0);
		}

		/* Close the connection socket description in the parent process*/
		close(connection_sd);
		/* In the parent process */
		if(waitpid(pid,NULL,0)!=pid)
		{
			perror("bk_init.c:Daemon_db_server:waitpid");
		}

		continue;
	}

	return -1;
}


/*!
 *****************************************************************************
 *
 * \brief 
 *    Do_fininace_control_for_updating_all_reckoning_date
 *    更新全部终端的上次出帐日期(新的尝试，只更新出帐日期，让终端自己点击刷新)
 * \par Input:
 *    conn_db_a: pointer to a database 
 *    packet : packet recieved
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Do_fininace_control_for_updating_all_reckoning_date(PGconn *conn_db_a, char *packet)
{
	char reckoning_date[FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_LENGTH + 1] = "00";
	char query_string_db[COMM_LENGTH];

	long long int i = 0;
	long long int record_sum = 0;
	
	int success = 0;
	
	struct Terminal_ID *p_terminal_id_array = NULL;

	PGresult *res_db = NULL;
        
	/*get reckoning date from packet*/
	bzero(reckoning_date, FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_LENGTH+1);
	memcpy(reckoning_date, packet+FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_LENGTH);
		
	/*get all of terminal id in terminal_manage table at self database*/
	bzero(query_string_db, COMM_LENGTH);
	sprintf(query_string_db, "SELECT terminal_id FROM terminal_manage ORDER BY terminal_id;");
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db_a, query_string_db);
	if (PQresultStatus(res_db) != PGRES_TUPLES_OK)
	{
		perror("error@Do_auto_refresh_deposit_process():PGRES_TUPLES_OK");
		printf("%s\n",PQerrorMessage(conn_db_a));
		PQclear(res_db);
		res_db = NULL;
	}
	/* Get the record sum of terminal_id in terminal_manage table */
	record_sum = (long long int)PQntuples(res_db);
		
	/* Malloc the space for array of terminal_id*/
	p_terminal_id_array = (struct Terminal_ID *)malloc(record_sum * (sizeof(struct Terminal_ID)));
	if (NULL == p_terminal_id_array)
	{
		perror("error@Do_auto_refresh_deposit_process():malloc==NULL");
		PQclear(res_db);
		res_db = NULL;
	}
		
	bzero(p_terminal_id_array, record_sum * (sizeof(struct Terminal_ID)));
	for (i=0; i<record_sum; i++)
	{
		/*update bank deposit in terminal_manage at the both databases*/
		strcpy(p_terminal_id_array[i].terminal_id, PQgetvalue(res_db, i, 0));
	}
		
	/*update reconking date for each terminal */
	for (i=0; i<record_sum; i++)
	{
		/*update bank deposit in terminal_manage at the both databases*/
		success = Update_appointed_terminal_reckoning_date (conn_db_a,p_terminal_id_array[i].terminal_id,reckoning_date);
		printf("\033[32mTerminal_ID:|%s| has been updated the last_reckoning_date!\033[0m\n", p_terminal_id_array[i].terminal_id);
		fflush(NULL);
	}
		
	/* free resources*/
	PQclear(res_db);
	res_db = NULL;
	free(p_terminal_id_array);
	p_terminal_id_array = NULL;

	return 1;
}
/*!
 *****************************************************************************
 *
 * \brief 
 *    Do_fininace_control_for_updating_appointed_terminal_reckoning_date
 *    财务终端要求更新某个终端的上次出帐日期
 *    1.更新终端的上次出帐日期
 *    2.计算并更新固定帐
 *    3.查询银行并更新终端的银行存款
 * \par Input:
 *    conn_db_a: pointer to a database 
 *    packet : packet recieved
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Do_fininace_control_for_updating_appointed_terminal_reckoning_date(PGconn *conn_db_a, char *packet)
{
	int success = 0;
	char reckoning_date[FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_LENGTH + 1] = "00";
	char terminal_id[FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_LENGTH + 1] = "00";


	/*get appointed terminal id and reckoning date from packet*/
	bzero(terminal_id, FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_LENGTH + 1);
	memcpy(terminal_id, packet+FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_LENGTH);
	bzero(reckoning_date, FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_LENGTH+1);
	memcpy(reckoning_date, packet+FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_LENGTH);

	success = Update_appointed_terminal_reckoning_date(conn_db_a,terminal_id, reckoning_date);
	return 1;
}

/*!
 *****************************************************************************
 *
 * \brief 
 *    Do_fininace_control_for_updating_appointed_terminal_reckoning_date
 *    财务终端要求更新某个终端的上次出帐日期
 *    1.更新终端的上次出帐日期
 *    2.计算并更新固定帐
 *    3.查询银行并更新终端的银行存款
 * \par Input:
 *    conn_db_a: pointer to a database 
 *    packet : packet recieved
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Update_appointed_terminal_reckoning_date(PGconn *conn_db_a, char *terminal_id, char* reckoning_date)
{
	//int success = 0;
	char yesterday_datebase[FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_LENGTH + 1] = "00";

	char query_string_db[COMM_LENGTH];
	PGresult *res_db = NULL;
	
	//char str_current_constant_money[COMM_LENGTH];
	//unsigned long long int current_constant_money = 0;
	//char *e = NULL;
	
	/*get the date of yesterday*/
	bzero(query_string_db, SQL_STRING_LENGTH); 
	sprintf(query_string_db,"select current_date-1;");
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db_a, query_string_db);
	if (PQresultStatus(res_db) != PGRES_TUPLES_OK) 
	{
		perror("error@Do_fininace_control_for_updating_appointed_terminal_reckoning_date:PQresultStatus");
		printf("%s\n",PQerrorMessage(conn_db_a));
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}
	bzero(yesterday_datebase, FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_LENGTH + 1); 
	strcpy(yesterday_datebase, PQgetvalue(res_db,0,0));

	/*0.1 为了避免出现超支情况，先将银行存款设为0，不让收费*/
	/*设置好固定帐和出账日期后再将银行存款设为有效值*/
	if (-1 == Update_appointed_terminal_bank_deposit(terminal_id, conn_db_a, (char*)"0"))
	{
		return -1;
	}
	//LOG("(财务刷新出账日期，First Set Deposit =|0|)");
	
       /*1.更新终端的上次出帐日期*/
	if (-1 == Update_appointed_terminal_latest_reckoning_date(terminal_id, reckoning_date, conn_db_a))
	{
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}
		
       /*2.计算并更新固定帐*/
	if (-1 == Update_appointed_terminal_total_constant_money(terminal_id, reckoning_date, yesterday_datebase, conn_db_a))
	{
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}


       /*3.查询银行并更新终端的银行存款*/
	if (-1 == Query_bank_appointed_terminal_deposit(terminal_id, conn_db_a))
	{
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}


	PQclear(res_db);
	res_db = NULL;

	return 1;
}

/*!
 *****************************************************************************
 *
 * \brief 
 *    Do_fininace_control_for_updating_appointed_terminal_deposit
 *    财务终端直接置银行存款值，当银行链路中断时需要这样做
 * \par Input:
 *    conn_db_a: pointer to a database 
 *    packet : packet recieved
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Do_fininace_control_for_updating_appointed_terminal_deposit(PGconn *conn_db_a, char *packet)
{
	char terminal_id[FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_LENGTH + 1] = "00";
	char bank_deposit[FINANCIAL_DEPOSIT_IN_FORWARD_PACKET_LENGTH + 1] = "00";

	/*get appointed terminal id and bank deposit from packet*/
	bzero(terminal_id, FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_LENGTH + 1);
	memcpy(terminal_id, packet+FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_LENGTH);
	bzero(bank_deposit, FINANCIAL_DEPOSIT_IN_FORWARD_PACKET_LENGTH + 1);
	memcpy(bank_deposit, packet+FINANCIAL_DEPOSIT_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_DEPOSIT_IN_FORWARD_PACKET_LENGTH);

	/*Update appointed terminal bank deposit in terminal_manage table at both databases*/
	if(-1 == Update_manual_appointed_terminal_deposit(terminal_id, bank_deposit, conn_db_a))
	{
		return -1;
	}
	//LOG("(Manual Set Deposit=|%s|)",bank_deposit);

	return 1;
}

/*!
 *****************************************************************************
 *
 * \brief 
 *    Do_fininace_control_for_requesting_update_appointed_terminal_deposit
 *    底下的终端要求更新自己的银行存款
 *    1.先查询一下自己的存款，
 *    2.再查询得到所有未处理的缴费记录的总金额
 *    3.相减得到可用余额后更新terminal_accounts表的可用余额项
 * \par Input:
 *    conn_db_a: pointer to a database 
 *    packet : packet recieved
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Do_fininace_control_for_requesting_update_appointed_terminal_deposit(PGconn *conn_db_a, char *packet)
{
	char terminal_id[FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_LENGTH + 1] = "00";
	char reckoning_date[FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_LENGTH + 1] = "00";
	//char terminal_id[FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_LENGTH + 1] = "00";
	char yesterday_datebase[FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_LENGTH + 1] = "00";

	char query_string_db[COMM_LENGTH];
	PGresult *res_db = NULL;

	/*get appointed terminal id and reckoning date from packet*/
	bzero(terminal_id, FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_LENGTH + 1);
	memcpy(terminal_id, packet+FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_TERMINAL_ID_IN_FORWARD_PACKET_LENGTH);

	/*get the date of yesterday*/
	bzero(query_string_db, SQL_STRING_LENGTH); 
	sprintf(query_string_db,"select current_date-1;");
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db_a, query_string_db);
	if (PQresultStatus(res_db) != PGRES_TUPLES_OK) 
	{
		perror("error@Do_fininace_control_for_updating_appointed_terminal_reckoning_date:PQresultStatus");
		printf("%s\n",PQerrorMessage(conn_db_a));
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}
	bzero(yesterday_datebase, FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_LENGTH + 1); 
	strcpy(yesterday_datebase, PQgetvalue(res_db,0,0));
	/*得到上次出帐日期*/
	/*get the date of yesterday*/
	bzero(query_string_db, SQL_STRING_LENGTH); 
	sprintf(query_string_db,
				"SELECT latest_reckoning_date FROM terminal_manage WHERE terminal_id=\'%s\';", 
				terminal_id);
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db_a, query_string_db);
	if (PQresultStatus(res_db) != PGRES_TUPLES_OK) 
	{
		perror("error@Do_fininace_control_for_updating_appointed_terminal_reckoning_date:PQresultStatus");
		printf("%s\n",PQerrorMessage(conn_db_a));
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}
	bzero(reckoning_date, FINANCIAL_RECKONING_DATE_IN_FORWARD_PACKET_LENGTH+1);
	strcpy(reckoning_date, PQgetvalue(res_db, 0, 0));

	/*0.1 为了避免出现超支情况，先将银行存款设为0，不让收费*/
	/*设置好固定帐和出账日期后再将银行存款设为有效值*/
	if (-1 == Update_appointed_terminal_bank_deposit(terminal_id, conn_db_a, (char*)"0"))
	{
		return -1;
	}
	//LOG("(终端自己刷新银行存款，First Set Deposit =|0|)");
	
    /*计算并更新terminal_manage表中的固定帐*/
	if (-1 == Update_appointed_terminal_total_constant_money(terminal_id, reckoning_date, yesterday_datebase, conn_db_a))
	{
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}
	/*update deposit and virtual available money in TABLE "terminal_manage"*/
	if (-1 == Query_bank_appointed_terminal_deposit(terminal_id, conn_db_a))
	//if (-1 == Query_bank_appointed_terminal_deposit_and_virtual_money(terminal_id, conn_db_a))
	{
		return -1;
	}

	return 1;
}

/*!
 *****************************************************************************
 *
 * \brief 
 *    Do_fininace_control_for_generating_day_report
 *
 * \par Input:
 *    conn_db_a: pointer to a database 
 *    packet : packet recieved
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Do_fininace_control_for_generating_day_report(PGconn *conn_db_a, char *packet)
{
	char day_report_date[FINANCIAL_DAY_REPORT_DATE_IN_FORWARD_PACKET_LENGTH + 1] = "00";
	
	/*get day report date from packet*/
	bzero(day_report_date, FINANCIAL_DAY_REPORT_DATE_IN_FORWARD_PACKET_LENGTH + 1);
	memcpy(day_report_date, packet+FINANCIAL_DAY_REPORT_DATE_IN_FORWARD_PACKET_START_POSITION, FINANCIAL_DAY_REPORT_DATE_IN_FORWARD_PACKET_LENGTH);
	
	/*generate date report view*/
	if ( -1 == Genarate_day_report_view(day_report_date, conn_db_a))
	{
		return -1;
	}
	
	return 1;
}

/*!
 *****************************************************************************
 *
 * \brief 
 *    Do_fininace_control_for_generating_month_report
 *
 * \par Input:
 *    conn_db_a: pointer to a database 
 *    packet : packet recieved
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Do_fininace_control_for_generating_month_report(PGconn *conn_db_a, char *packet)
{
	char month_report_start_date[FINANCIAL_MONTH_REPORT_START_DATE_IN_FORWARD_PACKET_LENGTH + 1] = "00";
	char month_report_end_date[FINANCIAL_MONTH_REPORT_END_DATE_IN_FORWARD_PACKET_LENGTH + 1] = "00";
	char month_report_table_name[COMPANY_NAME_LENGTH] = "00";
	char separator[10] = "\r\n";
	char *item = NULL;

	/*get month report start and end date from packet*/
	bzero(month_report_start_date, FINANCIAL_MONTH_REPORT_START_DATE_IN_FORWARD_PACKET_LENGTH + 1);
	memcpy(month_report_start_date, packet+FINANCIAL_MONTH_REPORT_START_DATE_IN_FORWARD_PACKET_START_POSITION, FINANCIAL_MONTH_REPORT_START_DATE_IN_FORWARD_PACKET_LENGTH);
	bzero(month_report_end_date, FINANCIAL_MONTH_REPORT_END_DATE_IN_FORWARD_PACKET_LENGTH + 1);
	memcpy(month_report_end_date, packet+FINANCIAL_MONTH_REPORT_END_DATE_IN_FORWARD_PACKET_START_POSITION, FINANCIAL_MONTH_REPORT_END_DATE_IN_FORWARD_PACKET_LENGTH);
	bzero(month_report_table_name, COMPANY_NAME_LENGTH);
	strcpy(month_report_table_name, packet+FINANCIAL_MONTH_REPORT_COMPANY_IN_FORWARD_PACKET_START_POSITION);
    //printf("Company_name: |%s|\n\n", month_report_table_name);

	item = strtok(month_report_table_name, separator);
    //printf("Company_name: |%s|\n\n", item);
	
	/*generate month report view*/
	//if ( -1 == Genarate_month_report_view(month_report_start_date, month_report_end_date, month_report_table_name, conn_db_a))
	if ( -1 == Genarate_month_report_view(month_report_start_date, month_report_end_date, item, conn_db_a))
	{
		return -1;
	}

	return 1;
}


/*!
 *****************************************************************************
 *
 * \brief 
 *    Do_fininace_control_for_generating_month_terminal_statistics
 *
 * \par Input:
 *    conn_db_a: pointer to a database 
 *    packet : packet recieved
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Do_fininace_control_for_generating_month_terminal_statistics(PGconn *conn_db_a, char *packet)
{
	char month_statistics_start_date[FINANCIAL_MONTH_TERMINAL_STATISTICS_START_DATE_IN_PACKET_LENGTH + 1] = "00";
	char month_statistics_end_date[FINANCIAL_MONTH_TERMINAL_STATISTICS_END_DATE_IN_PACKET_LENGTH + 1] = "00";

	/*get month report start and end date from packet*/
	bzero(month_statistics_start_date, FINANCIAL_MONTH_TERMINAL_STATISTICS_START_DATE_IN_PACKET_LENGTH + 1);
	memcpy(month_statistics_start_date, packet+FINANCIAL_MONTH_TERMINAL_STATISTICS_START_DATE_IN_PACKET_START_POSITION, FINANCIAL_MONTH_TERMINAL_STATISTICS_START_DATE_IN_PACKET_LENGTH);
	bzero(month_statistics_end_date, FINANCIAL_MONTH_TERMINAL_STATISTICS_END_DATE_IN_PACKET_LENGTH + 1);
	memcpy(month_statistics_end_date, packet+FINANCIAL_MONTH_TERMINAL_STATISTICS_END_DATE_IN_PACKET_START_POSITION, FINANCIAL_MONTH_TERMINAL_STATISTICS_END_DATE_IN_PACKET_LENGTH);

	/*generate month report view*/
	if ( -1 == Generate_month_terminal_statistics(month_statistics_start_date, month_statistics_end_date, conn_db_a))
	{
		return -1;
	}

	return 1;
}

/*!
 *****************************************************************************
 *
 * \brief 
 *    Do_fininace_control_for_generating_month_worker_statistics
 *
 * \par Input:
 *    conn_db_a: pointer to a database 
 *    packet : packet recieved
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Do_fininace_control_for_generating_month_worker_statistics(PGconn *conn_db_a, char *packet)
{
	char month_statistics_start_date[FINANCIAL_MONTH_WORKER_STATISTICS_START_DATE_IN_PACKET_LENGTH + 1] = "00";
	char month_statistics_end_date[FINANCIAL_MONTH_WORKER_STATISTICS_END_DATE_IN_PACKET_LENGTH + 1] = "00";

	/*get month report start and end date from packet*/
	bzero(month_statistics_start_date, FINANCIAL_MONTH_WORKER_STATISTICS_START_DATE_IN_PACKET_LENGTH + 1);
	memcpy(month_statistics_start_date, packet+FINANCIAL_MONTH_WORKER_STATISTICS_START_DATE_IN_PACKET_START_POSITION, FINANCIAL_MONTH_WORKER_STATISTICS_START_DATE_IN_PACKET_LENGTH);
	bzero(month_statistics_end_date, FINANCIAL_MONTH_WORKER_STATISTICS_END_DATE_IN_PACKET_LENGTH + 1);
	memcpy(month_statistics_end_date, packet+FINANCIAL_MONTH_WORKER_STATISTICS_END_DATE_IN_PACKET_START_POSITION, FINANCIAL_MONTH_WORKER_STATISTICS_END_DATE_IN_PACKET_LENGTH);

	/*generate month report view*/
	if ( -1 == Generate_month_worker_statistics(month_statistics_start_date, month_statistics_end_date, conn_db_a))
	{
		return -1;
	}

	return 1;
}


/*!
 *****************************************************************************
 *
 * \brief 
 *    Do_database_fininace_control_procedures: carry out the packet from accountant client. 
 *
 * \par Input:
 *    connection_sd: id for socket 
 *    packet : packet recieved
 *    pkt_len : packet length
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Do_database_fininace_control_procedures(int connection_sd, char *packet, int pkt_len)
{
	int success = 0;
	
	PGconn *conn_db_a = NULL;

	/*connect to self database*/
	conn_db_a = Connect_db_server(global_par.system_par.database_user[0], global_par.system_par.database_password[0],\
		   global_par.system_par.database_name, global_par.system_par.database_self_ip_address);
	
	/*we must connection self database*/
	if (NULL == conn_db_a)
	{
		perror("error@Do_database_fininace_control_procedures():connect");
		return -1;
	}
	
	if (0 == memcmp(packet+FINANCIAL_TYPE_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_UPDATE_ALL_RECKONING_DATE_TYPE, FINANCIAL_TYPE_IN_FORWARD_PACKET_LENGTH))
	{
		success = Do_fininace_control_for_updating_all_reckoning_date(conn_db_a, packet);
	} 
	else if (0 == memcmp(packet+FINANCIAL_TYPE_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_UPDATE_APPOINTED_RECKONING_DATE_TYPE, FINANCIAL_TYPE_IN_FORWARD_PACKET_LENGTH))
	{
		success = Do_fininace_control_for_updating_appointed_terminal_reckoning_date(conn_db_a, packet);
	}
	else if (0 == memcmp(packet+FINANCIAL_TYPE_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_UPDATE_APPOINTED_DEPOSIT_TYPE, FINANCIAL_TYPE_IN_FORWARD_PACKET_LENGTH))
	{
		success = Do_fininace_control_for_updating_appointed_terminal_deposit(conn_db_a, packet);
	}
	else if (0 == memcmp(packet+FINANCIAL_TYPE_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_REQUEST_UPDATE_DEPOSIT_TYPE, FINANCIAL_TYPE_IN_FORWARD_PACKET_LENGTH))
	{
		success = Do_fininace_control_for_requesting_update_appointed_terminal_deposit(conn_db_a, packet);
	}
	else if (0 == memcmp(packet+FINANCIAL_TYPE_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_DAY_REPORT_TYPE, FINANCIAL_TYPE_IN_FORWARD_PACKET_LENGTH))
	{
		success = Do_fininace_control_for_generating_day_report(conn_db_a, packet);
	}
	else if (0 == memcmp(packet+FINANCIAL_TYPE_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_MONTH_REPORT_TYPE, FINANCIAL_TYPE_IN_FORWARD_PACKET_LENGTH))
	{
		success = Do_fininace_control_for_generating_month_report(conn_db_a, packet);
	}
	else if (0 == memcmp(packet+FINANCIAL_CREATE_TERMINAL_MANAGE_VIEW_START_POSTION, FINANCIAL_CREATE_TERMINAL_MANAGE_VIEW, FINANCIAL_CREATE_TERMINAL_MANAGE_VIEW_LENGTH))
	{
		success = Generate_terminal_manage_view(conn_db_a);
	}
	else if (0 == memcmp(packet+FINANCIAL_TYPE_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_MONTH_TERMINAL_STATISTICS_TYPE, FINANCIAL_TYPE_IN_FORWARD_PACKET_LENGTH))
	{
		success = Do_fininace_control_for_generating_month_terminal_statistics(conn_db_a, packet);
	}
	else if (0 == memcmp(packet+FINANCIAL_TYPE_IN_FORWARD_PACKET_START_POSTION, FINANCIAL_MONTH_WORKER_STATISTICS_TYPE, FINANCIAL_TYPE_IN_FORWARD_PACKET_LENGTH))
	{
		success = Do_fininace_control_for_generating_month_worker_statistics(conn_db_a, packet);
	}
	else
	{
		printf("\033[01;31mDaemon_finance_control_server: receive a invalid packet!\033[0m\n");
		success = -1;
	}
	
	PQfinish(conn_db_a);
	conn_db_a = NULL;

	return success;
}

/*!
 *****************************************************************************
 *
 * \brief 
 *    Genarate_date_report_view: generate day report view. 
 *
 * \par Input:
 *    day_report_date: day report date 
 *	  conn_db: pointer to a database
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Genarate_day_report_view(char *day_report_date, PGconn *conn_db)
{
	int i = 0;
	int charge_index = -1;
	int reversal_index = -1;
	int charge_trade_code_index = -1;
	int reversal_trade_code_index = -1;

	PGresult *res_db = NULL;
	char drop_string_db[COMM_LENGTH];
	char create_string_db[COMM_LENGTH*60];
	unsigned int sql_len = 0;
	
	/*drop day_report_view*/
	bzero(drop_string_db, COMM_LENGTH);
	strcpy(drop_string_db, "DROP VIEW day_report_view;");
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, drop_string_db);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
	{
		perror("error@Genarate_day_report_view():PGRES_COMMAND_OK");
		printf("%s\n",PQerrorMessage(conn_db));
		PQclear(res_db);
		res_db = NULL;
	}

	for (i=0; i<global_par.company_num; i++)
	{
		/*drop company_day_report_full_view*/
		bzero(drop_string_db, COMM_LENGTH);
		sprintf(drop_string_db, "DROP VIEW %s_day_report_full_view", global_par.company_par_array[i].company_name); 
		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, drop_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Genarate_day_report_view():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
		}

		bzero(drop_string_db, COMM_LENGTH);
		sprintf(drop_string_db, "DROP VIEW %s_day_report_view", global_par.company_par_array[i].company_name);
		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, drop_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Genarate_day_report_view():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
		}
	}


	/*create each company day report view*/
	for (i=0; i<global_par.company_num; i++)
	{
		/*get charge trade code valid value*/
		for (charge_index=0; charge_index<global_par.company_par_array[i].packet_count; ++charge_index)
		{
			if (2 == global_par.company_par_array[i].packet_important_level[charge_index])
			{
				break;
			}
		}
		if (charge_index == global_par.company_par_array[i].packet_count)
		{
			printf("error@Genarate_day_report_view():no charge packet found!\n");
			return -1;
		}


		/*get reversal trade code valid value*/
		for (reversal_index=0; reversal_index<global_par.company_par_array[i].packet_count; ++reversal_index)
		{
			if (1 == global_par.company_par_array[i].packet_important_level[reversal_index])
			{
				break;
			}
		}
		if (reversal_index == global_par.company_par_array[i].packet_count)
		{
			printf("error@Genarate_day_report_view():no reversal packet found!\n");
			return -1;
		}		
		
		/*create the appointed company day report(layer one, only charge terminal)*/
		charge_trade_code_index = global_par.company_par_array[i].pkt_par_array[charge_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
		reversal_trade_code_index = global_par.company_par_array[i].pkt_par_array[reversal_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
		bzero(create_string_db, COMM_LENGTH*60);
		sprintf(create_string_db,
			"CREATE VIEW %s_day_report_view (terminal_id, charge_money) AS SELECT DISTINCT terminal_manage.terminal_id, sum(money) FROM terminal_manage, %s WHERE terminal_manage.terminal_id = %s.terminal_id AND record_date = \'%s\' AND trade_code= \'%s\' AND serial_number NOT IN (SELECT serial_number FROM %s WHERE trade_code=\'%s\' AND record_date = \'%s\') GROUP BY terminal_manage.terminal_id ORDER BY terminal_manage.terminal_id;",
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name,
			 day_report_date,
			 global_par.company_par_array[i].pkt_par_array[charge_index][BACKWARD_POSITION].item_par_array[charge_trade_code_index].valid_value,
			 global_par.company_par_array[i].company_name,
  			 global_par.company_par_array[i].pkt_par_array[reversal_index][BACKWARD_POSITION].item_par_array[reversal_trade_code_index].valid_value,
			 day_report_date);

		/*print SQL statement*/
		//printf("%s_day_report_view:%s\n", 
		//	global_par.company_par_array[i].company_name, 
		//	create_string_db);

		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, create_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Genarate_day_report_view():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
			return -1;
		}

		/*create the appointed company day report(layer two, all terminal)*/
		bzero(create_string_db, COMM_LENGTH*60);
		sprintf(create_string_db, 
			 "CREATE VIEW %s_day_report_full_view AS (SELECT DISTINCT terminal_id, charge_money FROM %s_day_report_view ORDER BY terminal_id) UNION (SELECT DISTINCT terminal_id, 0 AS charge_money FROM terminal_manage WHERE  terminal_id NOT IN (SELECT terminal_id FROM %s_day_report_view) ORDER BY terminal_id);",
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name);

		/*print SQL statement*/
		//printf("%s_day_report_full_view:%s\n", 
		//	global_par.company_par_array[i].company_name, 
		//	create_string_db);

		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, create_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Genarate_day_report_view():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
			return -1;
		}
	}	
	
	/*create all company day report view*/
	bzero(create_string_db, COMM_LENGTH*60);
	//strcpy(create_string_db, "CREATE VIEW day_report_view (terminal_id, ");
	strcpy(create_string_db, "CREATE VIEW day_report_view (terminal_id, terminal_type, duo_kou, shao_kou, duo_kou_yuan, shao_kou_yuan, ");
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_charge_money, ");
	}
	/*delete the last two character', '*/
	//sql_len = strlen(create_string_db);
	//memset(create_string_db+sql_len-2, 0, 2);

	strcat(create_string_db, "total_charge_money, ");
	strcat(create_string_db, "deduction_account, ");
	strcat(create_string_db, "deduction_account_name, ");
	strcat(create_string_db, "bonus_account, ");
	strcat(create_string_db, "bonus_account_name, ");
	strcat(create_string_db, "internal_serial, ");
	strcat(create_string_db, "remark");

	//strcat(create_string_db, ") AS SELECT DISTINCT terminal_manage.terminal_id, ");
	//strcat(create_string_db, ") AS SELECT DISTINCT terminal_manage.terminal_id, terminal_manage.terminal_type, terminal_manage.duo_kou, terminal_manage.shao_kou, ");
	strcat(create_string_db, ") AS SELECT DISTINCT terminal_manage.terminal_id, terminal_manage.terminal_type, terminal_manage.shao_kou, terminal_manage.duo_kou, to_char(terminal_manage.shao_kou / 100, '9999999990.99'), to_char(terminal_manage.duo_kou / 100, '9999999990.99'), ");
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, "to_char(");
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_day_report_full_view.charge_money/100, \'9999999990.99\'), ");
	}

	strcat(create_string_db, "to_char((");
	/*total_sum_money string*/
	for (i=0; i<global_par.company_num; i++)
	{
		if(0 == memcmp(global_par.company_par_array[i].company_name, "bank_card_", 10))
		{
			/*说明是银行卡，代扣网点的钱应当减去*/
			strcat(create_string_db,"(-");
			strcat(create_string_db, global_par.company_par_array[i].company_name);
			strcat(create_string_db, "_day_report_full_view.charge_money) + ");
		}
		else
		{
			/*说明是收费业务，代扣网点的钱应当加上*/
			strcat(create_string_db, global_par.company_par_array[i].company_name);
			strcat(create_string_db, "_day_report_full_view.charge_money + ");
		}
	}
	/*delete the last two character'+ '*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-2, 0, 2);
	strcat(create_string_db, ")/100, \'9999999990.99\')");

	strcat(create_string_db, ", deduction_account");
	strcat(create_string_db, ", deduction_account_name");
	strcat(create_string_db, ", bonus_account");
	strcat(create_string_db, ", bonus_account_name");
	strcat(create_string_db, ", internal_serial");
	strcat(create_string_db, ", remark");
	strcat(create_string_db, ", chan_pin_lei_bie");
	strcat(create_string_db, ", chan_pin_zi_lei");

	strcat(create_string_db, " FROM terminal_manage, ");
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_day_report_full_view, ");
	}
	/*delete the last two character', '*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-2, 0, 2);

	strcat(create_string_db, " WHERE ");
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, "terminal_manage.terminal_id = ");
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_day_report_full_view.terminal_id AND ");
	}	
	/*delete the last fine character' AND ', and add ";"*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-5, 0, 5);
	strcat(create_string_db, ";");

	/*print SQL statement*/
	printf("day_report_view:%s\n", create_string_db);

	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, create_string_db);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
	{
		perror("error@Genarate_day_report_view():PGRES_COMMAND_OK");
		printf("%s\n",PQerrorMessage(conn_db));
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}
	PQclear(res_db);
	res_db = NULL;

	return 1;
}

/*!
 *****************************************************************************
 *
 * \brief 
 *    Genarate_month_report_view: generate month report view. 
 *
 * \par Input:
 *    month_report_start_date: month report start date
 *    month_report_end_date: month report end date
 *	  table_name: the table name for the company
 *	  conn_db: pointer to a database
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Genarate_month_report_view(char *month_report_start_date, char *month_report_end_date, char *table_name, PGconn *conn_db)
{
	//int i = 0;
	int company_index = -1;
	int charge_index = -1;
	int reversal_index = -1;
	int charge_trade_code_index = -1;
	int reversal_trade_code_index = -1;

	PGresult *res_db = NULL;
	//char drop_string_db[COMM_LENGTH];
	char create_string_db[COMM_LENGTH*60];
	//unsigned int sql_len = 0;

	/*drop day_report_view*/
// 	bzero(drop_string_db, COMM_LENGTH);
// 	strcpy(drop_string_db, "DROP VIEW %s_month_report_view;", table_name);
// 	PQclear(res_db);
// 	res_db = NULL;
// 	res_db = PQexec(conn_db, drop_string_db);
// 	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
// 	{
// 		perror("error@Genarate_month_report_view():PGRES_COMMAND_OK");
// 		printf("%s\n",PQerrorMessage(conn_db));
// 		PQclear(res_db);
// 		res_db = NULL;
// 	}
	
	/*get company index*/
	for (company_index=0; company_index<global_par.company_num; ++company_index)
	{
		if (0 == strcasecmp(global_par.company_par_array[company_index].company_name, table_name))
		{
			break;
		}
	}
	if (company_index == global_par.company_num)
	{
		printf("error@Genarate_month_report_view():no such company found!\n");
		return -1;
	}

	/*get charge trade code valid value*/
	for (charge_index=0; charge_index<global_par.company_par_array[company_index].packet_count; ++charge_index)
	{
		if (2 == global_par.company_par_array[company_index].packet_important_level[charge_index])
		{
			break;
		}
	}
	if (charge_index == global_par.company_par_array[company_index].packet_count)
	{
		printf("error@Genarate_month_report_view():no charge packet found!\n");
		return -1;
	}


	/*get reversal trade code valid value*/
	for (reversal_index=0; reversal_index<global_par.company_par_array[company_index].packet_count; ++reversal_index)
	{
		if (1 == global_par.company_par_array[company_index].packet_important_level[reversal_index])
		{
			break;
		}
	}
	if (reversal_index == global_par.company_par_array[company_index].packet_count)
	{
		printf("error@Genarate_month_report_view():no reversal packet found!\n");
		return -1;
	}		
		
	/*create the appointed company month report*/
	charge_trade_code_index = global_par.company_par_array[company_index].pkt_par_array[charge_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
	reversal_trade_code_index = global_par.company_par_array[company_index].pkt_par_array[reversal_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
	bzero(create_string_db, COMM_LENGTH*60);
	sprintf(create_string_db,
		"CREATE OR REPLACE VIEW month_report_view AS SELECT record_date AS date, to_char(sum(money)/100, \'9999999990.99\') AS money, count(id) AS count FROM %s WHERE trade_code = \'%s\' AND record_date >= \'%s\' AND record_date <= \'%s\' AND serial_number NOT IN (SELECT serial_number FROM %s WHERE trade_code = \'%s\' AND record_date >= \'%s\' AND record_date <= \'%s\') GROUP BY record_date ORDER BY record_date;",
		 global_par.company_par_array[company_index].company_name,
		 global_par.company_par_array[company_index].pkt_par_array[charge_index][BACKWARD_POSITION].item_par_array[charge_trade_code_index].valid_value,
		 month_report_start_date,
		 month_report_end_date,
		 global_par.company_par_array[company_index].company_name,
		 global_par.company_par_array[company_index].pkt_par_array[reversal_index][BACKWARD_POSITION].item_par_array[reversal_trade_code_index].valid_value,
		 month_report_start_date,
		 month_report_end_date
		 );

	/*print SQL statement*/
	printf("%s_month_report_view:%s\n", 
		global_par.company_par_array[company_index].company_name, 
		create_string_db);
	LOG(INFO)<<"month_report_view:"<<create_string_db;
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, create_string_db);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
	{
		perror("error@Genarate_month_report_view():PGRES_COMMAND_OK");
		printf("%s\n",PQerrorMessage(conn_db));
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}

	PQclear(res_db);
	res_db = NULL;	

	return 1;
}


/*!
 *****************************************************************************
 *
 * \brief 
 *    Generate_terminal_manage_view: generate terminal manage view. 
 *
 * \par Input:
 *	  conn_db: pointer to a database
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Generate_terminal_manage_view(PGconn *conn_db)
{
	int i = 0;
	int charge_index = -1;
	int reversal_index = -1;
	int charge_trade_code_index = -1;
	int reversal_trade_code_index = -1;
	
	PGresult *res_db = NULL;
	char drop_string_db[COMM_LENGTH];
	char create_string_db[COMM_LENGTH*60];
	unsigned int sql_len = 0;
	

	/* Check the input parameters */
	if(NULL == conn_db)
	{
		perror("error@Generate_terminal_manage_view:NULL");
		return -1;
	}
	
	/*drop terminal_manage_view*/
	bzero(drop_string_db, COMM_LENGTH);
	strcpy(drop_string_db, "DROP VIEW terminal_manage_view;");
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, drop_string_db);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
	{
		perror("error@Generate_terminal_manage_view():PGRES_COMMAND_OK");
		printf("%s\n",PQerrorMessage(conn_db));
		PQclear(res_db);
		res_db = NULL;
	}

	for (i=0; i<global_par.company_num; i++)
	{
		/*drop terminal_comapny_full_view*/
		bzero(drop_string_db, COMM_LENGTH);
		sprintf(drop_string_db, "DROP VIEW terminal_%s_full_view", global_par.company_par_array[i].company_name); 
		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, drop_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Generate_terminal_manage_view():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
		}

		/*drop terminal_comapny_charge_view*/
		bzero(drop_string_db, COMM_LENGTH);
		sprintf(drop_string_db, "DROP VIEW terminal_%s_charge_view", global_par.company_par_array[i].company_name);
		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, drop_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Generate_terminal_manage_view():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
		}
	}

	/*create each company terminal manage view*/
	for (i=0; i<global_par.company_num; i++)
	{
		/*get charge trade code valid value*/
		for (charge_index=0; charge_index<global_par.company_par_array[i].packet_count; ++charge_index)
		{
			if (2 == global_par.company_par_array[i].packet_important_level[charge_index])
			{
				break;
			}
		}
		if (charge_index == global_par.company_par_array[i].packet_count)
		{
			printf("error@Generate_terminal_manage_view():no charge packet found!\n");
			return -1;
		}

		/*get reversal trade code valid value*/
		for (reversal_index=0; reversal_index<global_par.company_par_array[i].packet_count; ++reversal_index)
		{
			if (1 == global_par.company_par_array[i].packet_important_level[reversal_index])
			{
				break;
			}
		}
		if (reversal_index == global_par.company_par_array[i].packet_count)
		{
			printf("error@Generate_terminal_manage_view():no reversal packet found!\n");
			return -1;
		}
		
		/*create the appointed company terminal view(layer one, only charge terminal)*/
		charge_trade_code_index = global_par.company_par_array[i].pkt_par_array[charge_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
		reversal_trade_code_index = global_par.company_par_array[i].pkt_par_array[reversal_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
		bzero(create_string_db, COMM_LENGTH*60);
		sprintf(create_string_db,
			"CREATE VIEW terminal_%s_charge_view AS SELECT DISTINCT terminal_manage.terminal_id, sum(money) AS charge_money FROM terminal_manage, %s WHERE terminal_manage.terminal_id = %s.terminal_id AND record_date = CURRENT_DATE AND trade_code = \'%s\'AND serial_number NOT IN (SELECT serial_number FROM %s WHERE trade_code = \'%s\' AND record_date = CURRENT_DATE) GROUP BY terminal_manage.terminal_id ORDER BY terminal_manage.terminal_id;",
			global_par.company_par_array[i].company_name,
			global_par.company_par_array[i].company_name,
			global_par.company_par_array[i].company_name,
			global_par.company_par_array[i].pkt_par_array[charge_index][BACKWARD_POSITION].item_par_array[charge_trade_code_index].valid_value,
			global_par.company_par_array[i].company_name,
			global_par.company_par_array[i].pkt_par_array[reversal_index][BACKWARD_POSITION].item_par_array[reversal_trade_code_index].valid_value);

		/*print SQL statement*/
		printf("terminal_%s_charge_view:%s\n", 
			global_par.company_par_array[i].company_name, 
			create_string_db);

		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, create_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Generate_terminal_manage_view():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
			return -1;
		}

		/*create the appointed company terminal manage(layer two, all terminal)*/
		bzero(create_string_db, COMM_LENGTH*60);
		sprintf(create_string_db, 
			 "CREATE VIEW terminal_%s_full_view AS (SELECT DISTINCT terminal_id, charge_money FROM terminal_%s_charge_view ORDER BY terminal_id) UNION (SELECT DISTINCT terminal_id, 0 AS charge_money FROM terminal_manage WHERE terminal_id NOT IN ( SELECT terminal_id FROM terminal_%s_charge_view) ORDER BY terminal_manage.terminal_id);",
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name);

		/*print SQL statement*/
		printf("terminal_%s_full_view:%s\n", 
			global_par.company_par_array[i].company_name, 
			create_string_db);

		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, create_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Generate_terminal_manage_view():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
			return -1;
		}
	}	
	
	/*create terminal manage view*/
	bzero(create_string_db, COMM_LENGTH*60);
	strcpy(create_string_db, "CREATE VIEW terminal_manage_view AS SELECT DISTINCT terminal_manage.terminal_id, terminal_manage.chan_pin_lei_bie, terminal_manage.terminal_type,  terminal_manage.shao_kou, to_char(terminal_manage.shao_kou / 100, '9999999990.99') AS shao_kou_yuan, terminal_manage.duo_kou, to_char(terminal_manage.duo_kou / 100, '9999999990.99') AS duo_kou_yuan, terminal_manage.chan_pin_zi_lei, bank_deposit ");
	for (i=0; i<global_par.company_num; i++)
	{
		if(0 == memcmp(global_par.company_par_array[i].company_name, "bank_card_", 10))
		{
			/*说明是银行卡，代扣网点的钱应当减去*/
			strcat(create_string_db, " + terminal_");
			strcat(create_string_db, global_par.company_par_array[i].company_name);
			strcat(create_string_db, "_full_view.charge_money");
		}
		else
		{
			/*说明是收费业务，代扣网点的钱应当加上*/
			strcat(create_string_db, " - terminal_");
			strcat(create_string_db, global_par.company_par_array[i].company_name);
			strcat(create_string_db, "_full_view.charge_money");
		}
	}

	strcat(create_string_db, " - terminal_manage.total_constant_money AS available_money, to_char((terminal_manage.bank_deposit");
	
	for (i=0; i<global_par.company_num; i++)
	{
		if(0 == memcmp(global_par.company_par_array[i].company_name, "bank_card_", 10))
		{
			/*说明是银行卡，代扣网点的钱应当减去*/
			strcat(create_string_db, " + terminal_");
			strcat(create_string_db, global_par.company_par_array[i].company_name);
			strcat(create_string_db, "_full_view.charge_money");
		}
		else
		{
			/*说明是收费业务，代扣网点的钱应当加上*/
			strcat(create_string_db, " - terminal_");
			strcat(create_string_db, global_par.company_par_array[i].company_name);
			strcat(create_string_db, "_full_view.charge_money");
		}
	}
	
	strcat(create_string_db, " - terminal_manage.total_constant_money) / 100, '9999999990.99') AS available_money_yuan, ");

	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, "to_char(terminal_");
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_full_view.charge_money / 100, '9999999990.99') AS ");
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_today_money, ");
	}

	strcat(create_string_db, "to_char((");
	for (i=0; i<global_par.company_num; i++)
	{
		if(0 == memcmp(global_par.company_par_array[i].company_name, "bank_card_", 10))
		{
			/*说明是银行卡，代扣网点的钱应当减去*/
			strcat(create_string_db,"(-");
			strcat(create_string_db, "terminal_");
			strcat(create_string_db, global_par.company_par_array[i].company_name);
			strcat(create_string_db, "_full_view.charge_money) + ");
		}
		else
		{
			/*说明是收费业务，代扣网点的钱应当加上*/
			strcat(create_string_db, "terminal_");
			strcat(create_string_db, global_par.company_par_array[i].company_name);
			strcat(create_string_db, "_full_view.charge_money + ");
		}
	}

	/*delete the last three character' + '*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-3, 0, 3);
	
	strcat(create_string_db, ") / 100::numeric, '9999999990.99') AS total_sum_money, to_char(terminal_manage.total_constant_money / 100, '9999999990.99') AS total_constant_money, remark, internal_serial, apply_refresh_flag, to_char(terminal_manage.bank_deposit / 100, '9999999990.99') AS bank_deposit, latest_reckoning_date");

	strcat(create_string_db, " FROM terminal_manage, ");
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, "terminal_");
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_full_view, ");
	}

	/*delete the last two character', '*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-2, 0, 2);

	strcat(create_string_db, " WHERE ");

	for (i=0; i<global_par.company_num; i++)
	{	
		strcat(create_string_db, "terminal_manage.terminal_id = terminal_");
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_full_view.terminal_id AND ");
	}

	/*delete the last four character' AND '*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-4, 0, 4);

	strcat(create_string_db, "ORDER BY terminal_manage.terminal_id;");

	/*print SQL statement*/
	printf("terminal_manage_view:%s\n", create_string_db);

	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, create_string_db);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
	{
		perror("error@Generate_terminal_manage_view():PGRES_COMMAND_OK");
		printf("%s\n",PQerrorMessage(conn_db));
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}
	PQclear(res_db);
	res_db = NULL;		
		
	return 1;
}



/*!
 *****************************************************************************
 *
 * \brief 
 *    Generate_month_terminal_statistics: generate month statistics of terminals. 
 *
 * \par Input:
 *    month_report_start_date: month report start date
 *    month_report_end_date: month report end date
 *	  conn_db: pointer to a database
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Generate_month_terminal_statistics(char *month_statistics_start_date, char *month_statistics_end_date, PGconn *conn_db)
{
	int i = 0;
	int charge_index = -1;
	int reversal_index = -1;
	int charge_trade_code_index = -1;
	int reversal_trade_code_index = -1;

	PGresult *res_db = NULL;
	char drop_string_db[COMM_LENGTH];
	char create_string_db[COMM_LENGTH*60];
	unsigned int sql_len = 0;
	
	/*drop day_report_view*/
	bzero(drop_string_db, COMM_LENGTH);
	strcpy(drop_string_db, "DROP VIEW month_terminal_statistics_view;");
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, drop_string_db);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
	{
		perror("error@Generate_month_terminal_statistics():PGRES_COMMAND_OK");
		printf("%s\n",PQerrorMessage(conn_db));
		PQclear(res_db);
		res_db = NULL;
	}

	for (i=0; i<global_par.company_num; i++)
	{
		/*drop company_day_report_full_view*/
		bzero(drop_string_db, COMM_LENGTH);
		sprintf(drop_string_db, "DROP VIEW %s_month_terminal_statistics_full_view", global_par.company_par_array[i].company_name); 
		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, drop_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Generate_month_terminal_statistics():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
		}

		bzero(drop_string_db, COMM_LENGTH);
		sprintf(drop_string_db, "DROP VIEW %s_month_terminal_statistics_view", global_par.company_par_array[i].company_name);
		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, drop_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Generate_month_terminal_statistics():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
		}
	}


	/*create each company day report view*/
	for (i=0; i<global_par.company_num; i++)
	{
		/*get charge trade code valid value*/
		for (charge_index=0; charge_index<global_par.company_par_array[i].packet_count; ++charge_index)
		{
			if (2 == global_par.company_par_array[i].packet_important_level[charge_index])
			{
				break;
			}
		}
		if (charge_index == global_par.company_par_array[i].packet_count)
		{
			printf("error@Generate_month_terminal_statistics():no charge packet found!\n");
			return -1;
		}


		/*get reversal trade code valid value*/
		for (reversal_index=0; reversal_index<global_par.company_par_array[i].packet_count; ++reversal_index)
		{
			if (1 == global_par.company_par_array[i].packet_important_level[reversal_index])
			{
				break;
			}
		}
		if (reversal_index == global_par.company_par_array[i].packet_count)
		{
			printf("error@Generate_month_terminal_statistics():no reversal packet found!\n");
			return -1;
		}		
		
		/*create the appointed company day report(layer one, only charge terminal)*/
		charge_trade_code_index = global_par.company_par_array[i].pkt_par_array[charge_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
		reversal_trade_code_index = global_par.company_par_array[i].pkt_par_array[reversal_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
		bzero(create_string_db, COMM_LENGTH*60);
		sprintf(create_string_db,
			"CREATE VIEW %s_month_terminal_statistics_view (terminal_id, charge_money, charge_counts) AS SELECT DISTINCT terminal_manage.terminal_id, sum(money), count(id) FROM terminal_manage, %s WHERE terminal_manage.terminal_id = %s.terminal_id AND record_date >= \'%s\' AND record_date <= \'%s\' AND trade_code= \'%s\' AND serial_number NOT IN (SELECT serial_number FROM %s WHERE trade_code=\'%s\' AND record_date >= \'%s\' AND record_date <= \'%s\') GROUP BY terminal_manage.terminal_id ORDER BY terminal_manage.terminal_id;",
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name,
			 month_statistics_start_date,
			 month_statistics_end_date,
			 global_par.company_par_array[i].pkt_par_array[charge_index][BACKWARD_POSITION].item_par_array[charge_trade_code_index].valid_value,
			 global_par.company_par_array[i].company_name,
  			 global_par.company_par_array[i].pkt_par_array[reversal_index][BACKWARD_POSITION].item_par_array[reversal_trade_code_index].valid_value,
			 month_statistics_start_date,
		     month_statistics_end_date
		     );

		/*print SQL statement*/
		printf("%s_month_terminal_statistics_view:%s\n\n\n", 
			global_par.company_par_array[i].company_name, 
			create_string_db);

		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, create_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Generate_month_terminal_statistics():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
			return -1;
		}

		/*create the appointed company day report(layer two, all terminal)*/
		bzero(create_string_db, COMM_LENGTH*60);
		sprintf(create_string_db, 
			 "CREATE VIEW %s_month_terminal_statistics_full_view AS (SELECT DISTINCT terminal_id, charge_money, charge_counts FROM %s_month_terminal_statistics_view ORDER BY terminal_id) UNION (SELECT DISTINCT terminal_id, 0 AS charge_money, 0 AS charge_counts FROM terminal_manage WHERE  terminal_id NOT IN (SELECT terminal_id FROM %s_month_terminal_statistics_view) ORDER BY terminal_id);",
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name);

		/*print SQL statement*/
		printf("%s_month_terminal_statistics_full_view:%s\n\n\n", 
			global_par.company_par_array[i].company_name, 
			create_string_db);

		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, create_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Generate_month_terminal_statistics():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
			return -1;
		}
	}	
	
	/*create all company day report view*/
	bzero(create_string_db, COMM_LENGTH*60);
	strcpy(create_string_db, "CREATE VIEW month_terminal_statistics_view (terminal_id, ");
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_charge_money, ");
	}
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_charge_counts, ");
	}

	strcat(create_string_db, "total_charge_money, ");
	strcat(create_string_db, "total_charge_counts, ");
	strcat(create_string_db, "deduction_account, ");
	strcat(create_string_db, "deduction_account_name, ");
	strcat(create_string_db, "bonus_account, ");
	strcat(create_string_db, "bonus_account_name, ");
	strcat(create_string_db, "internal_serial, ");
	strcat(create_string_db, "remark");

	strcat(create_string_db, ") AS SELECT DISTINCT terminal_manage.terminal_id, ");
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, "to_char(");
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_month_terminal_statistics_full_view.charge_money/100, \'9999999990.99\'), ");
	}

	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_month_terminal_statistics_full_view.charge_counts, ");
	}

	strcat(create_string_db, "to_char((");
	/*total_sum_money string*/
	for (i=0; i<global_par.company_num; i++)
	{
		if(0 == memcmp(global_par.company_par_array[i].company_name, "bank_card_", 10))
		{
			/*说明是银行卡，代扣网点的钱应当减去*/
			strcat(create_string_db,"(-");
			strcat(create_string_db, global_par.company_par_array[i].company_name);
			strcat(create_string_db, "_month_terminal_statistics_full_view.charge_money) + ");
		}
		else
		{
			/*说明是收费业务，代扣网点的钱应当加上*/
			strcat(create_string_db, global_par.company_par_array[i].company_name);
			strcat(create_string_db, "_month_terminal_statistics_full_view.charge_money + ");
		}
	}
	/*delete the last two character'+ '*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-2, 0, 2);
	strcat(create_string_db, ")/100, \'9999999990.99\'), ");

	/*total_sum_counts string*/
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_month_terminal_statistics_full_view.charge_counts + ");
	}
	/*delete the last two character'+ '*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-2, 0, 2);

	strcat(create_string_db, ", deduction_account");
	strcat(create_string_db, ", deduction_account_name");
	strcat(create_string_db, ", bonus_account");
	strcat(create_string_db, ", bonus_account_name");
	strcat(create_string_db, ", internal_serial");
	strcat(create_string_db, ", remark");

	strcat(create_string_db, " FROM terminal_manage, ");
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_month_terminal_statistics_full_view, ");
	}
	/*delete the last two character', '*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-2, 0, 2);

	strcat(create_string_db, " WHERE ");
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, "terminal_manage.terminal_id = ");
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_month_terminal_statistics_full_view.terminal_id AND ");
	}	
	/*delete the last fine character' AND ', and add ";"*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-5, 0, 5);
	strcat(create_string_db, ";");

	/*print SQL statement*/
	printf("month_terminal_statistics_view:%s\n\n\n", create_string_db);

	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, create_string_db);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
	{
		perror("error@Generate_month_terminal_statistics():PGRES_COMMAND_OK");
		printf("%s\n",PQerrorMessage(conn_db));
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}
	PQclear(res_db);
	res_db = NULL;

	return 1;
	
}


/*!
 *****************************************************************************
 *
 * \brief 
 *    Generate_month_worker_statistics: generate month statistics of workers. 
 *
 * \par Input:
 *    month_report_start_date: month report start date
 *    month_report_end_date: month report end date
 *	  conn_db: pointer to a database
 *
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Generate_month_worker_statistics(char *month_statistics_start_date, char *month_statistics_end_date, PGconn *conn_db)
{
	int i = 0;
	int charge_index = -1;
	int reversal_index = -1;
	int charge_trade_code_index = -1;
	int reversal_trade_code_index = -1;

	PGresult *res_db = NULL;
	char drop_string_db[COMM_LENGTH];
	char create_string_db[COMM_LENGTH*60];
	unsigned int sql_len = 0;
	
	/*drop day_report_view*/
	bzero(drop_string_db, COMM_LENGTH);
	strcpy(drop_string_db, "DROP VIEW month_worker_statistics_view;");
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, drop_string_db);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
	{
		perror("error@Generate_month_worker_statistics():PGRES_COMMAND_OK");
		printf("%s\n",PQerrorMessage(conn_db));
		PQclear(res_db);
		res_db = NULL;
	}

	for (i=0; i<global_par.company_num; i++)
	{
		/*drop company_day_report_full_view*/
		bzero(drop_string_db, COMM_LENGTH);
		sprintf(drop_string_db, "DROP VIEW %s_month_worker_statistics_full_view", global_par.company_par_array[i].company_name); 
		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, drop_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Generate_month_worker_statistics():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
		}

		bzero(drop_string_db, COMM_LENGTH);
		sprintf(drop_string_db, "DROP VIEW %s_month_worker_statistics_view", global_par.company_par_array[i].company_name);
		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, drop_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Generate_month_worker_statistics():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
		}
	}


	/*create each company day report view*/
	for (i=0; i<global_par.company_num; i++)
	{
		/*get charge trade code valid value*/
		for (charge_index=0; charge_index<global_par.company_par_array[i].packet_count; ++charge_index)
		{
			if (2 == global_par.company_par_array[i].packet_important_level[charge_index])
			{
				break;
			}
		}
		if (charge_index == global_par.company_par_array[i].packet_count)
		{
			printf("error@Generate_month_worker_statistics():no charge packet found!\n");
			return -1;
		}


		/*get reversal trade code valid value*/
		for (reversal_index=0; reversal_index<global_par.company_par_array[i].packet_count; ++reversal_index)
		{
			if (1 == global_par.company_par_array[i].packet_important_level[reversal_index])
			{
				break;
			}
		}
		if (reversal_index == global_par.company_par_array[i].packet_count)
		{
			printf("error@Generate_month_worker_statistics():no reversal packet found!\n");
			return -1;
		}		
		
		/*create the appointed company day report(layer one, only charge terminal)*/
		charge_trade_code_index = global_par.company_par_array[i].pkt_par_array[charge_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
		reversal_trade_code_index = global_par.company_par_array[i].pkt_par_array[reversal_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
		bzero(create_string_db, COMM_LENGTH*60);
		sprintf(create_string_db,
			"CREATE VIEW %s_month_worker_statistics_view (terminal_id, worker_id, charge_money, charge_counts) AS SELECT DISTINCT worker_manage.terminal_id, worker_manage.worker_id, sum(money), count(id) FROM worker_manage, %s WHERE worker_manage.terminal_id = %s.terminal_id AND worker_manage.worker_id = %s.worker_id AND record_date >= \'%s\' AND record_date <= \'%s\' AND trade_code= \'%s\' AND serial_number NOT IN (SELECT serial_number FROM %s WHERE trade_code=\'%s\' AND record_date >= \'%s\' AND record_date <= \'%s\') GROUP BY worker_manage.terminal_id, worker_manage.worker_id ORDER BY worker_manage.terminal_id, worker_manage.worker_id;",
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name,
			 month_statistics_start_date,
			 month_statistics_end_date,
			 global_par.company_par_array[i].pkt_par_array[charge_index][BACKWARD_POSITION].item_par_array[charge_trade_code_index].valid_value,
			 global_par.company_par_array[i].company_name,
  			 global_par.company_par_array[i].pkt_par_array[reversal_index][BACKWARD_POSITION].item_par_array[reversal_trade_code_index].valid_value,
			 month_statistics_start_date,
		     month_statistics_end_date
		     );

		/*print SQL statement*/
		printf("%s_month_worker_statistics_view:%s\n\n\n", 
			global_par.company_par_array[i].company_name, 
			create_string_db);

		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, create_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Generate_month_worker_statistics():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
			return -1;
		}

		/*create the appointed company day report(layer two, all terminal)*/
		bzero(create_string_db, COMM_LENGTH*60);
		sprintf(create_string_db, 
             "CREATE VIEW %s_month_worker_statistics_full_view AS (SELECT DISTINCT terminal_id, worker_id, charge_money, charge_counts FROM %s_month_worker_statistics_view ORDER BY terminal_id, worker_id) UNION (SELECT DISTINCT terminal_id, worker_id, 0 AS charge_money, 0 AS charge_counts FROM worker_manage WHERE (terminal_id, worker_id) NOT IN (SELECT terminal_id, worker_id FROM %s_month_worker_statistics_view) ORDER BY terminal_id, worker_id);",
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name,
			 global_par.company_par_array[i].company_name);

		/*print SQL statement*/
		printf("%s_month_worker_statistics_full_view:%s\n\n\n", 
			global_par.company_par_array[i].company_name, 
			create_string_db);

		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, create_string_db);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
		{
			perror("error@Generate_month_worker_statistics():PGRES_COMMAND_OK");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
			return -1;
		}
	}	
	
	/*create all company day report view*/
	bzero(create_string_db, COMM_LENGTH*60);
	strcpy(create_string_db, "CREATE VIEW month_worker_statistics_view (terminal_id, worker_id, ");
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_charge_money, ");
	}

	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_charge_counts, ");
	}

	strcat(create_string_db, "total_charge_money, ");
	strcat(create_string_db, "total_charge_counts, ");
	strcat(create_string_db, "worker_name");

	strcat(create_string_db, ") AS SELECT DISTINCT worker_manage.terminal_id, worker_manage.worker_id, ");
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, "to_char(");
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_month_worker_statistics_full_view.charge_money/100, \'9999999990.99\'), ");
	}

	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_month_worker_statistics_full_view.charge_counts, ");
	}

	strcat(create_string_db, "to_char((");
	/*total_sum_money string*/
	for (i=0; i<global_par.company_num; i++)
	{
		if(0 == memcmp(global_par.company_par_array[i].company_name, "bank_card_", 10))
		{
			/*说明是银行卡，代扣网点的钱应当减去*/
			strcat(create_string_db,"(-");
			strcat(create_string_db, global_par.company_par_array[i].company_name);
			strcat(create_string_db, "_month_worker_statistics_full_view.charge_money) + ");
		}
		else
		{
			/*说明是收费业务，代扣网点的钱应当加上*/
			strcat(create_string_db, global_par.company_par_array[i].company_name);
			strcat(create_string_db, "_month_worker_statistics_full_view.charge_money + ");
		}
	}
	/*delete the last two character'+ '*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-2, 0, 2);
	strcat(create_string_db, ")/100, \'9999999990.99\'), ");

	/*total_sum_counts string*/
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_month_worker_statistics_full_view.charge_counts + ");
	}
	/*delete the last two character'+ '*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-2, 0, 2);

	strcat(create_string_db, ", worker_name");

	strcat(create_string_db, " FROM worker_manage, ");
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_month_worker_statistics_full_view, ");
	}
	/*delete the last two character', '*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-2, 0, 2);

	strcat(create_string_db, " WHERE ");
	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, "worker_manage.terminal_id = ");
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_month_worker_statistics_full_view.terminal_id AND ");
	}	

	for (i=0; i<global_par.company_num; i++)
	{
		strcat(create_string_db, "worker_manage.worker_id = ");
		strcat(create_string_db, global_par.company_par_array[i].company_name);
		strcat(create_string_db, "_month_worker_statistics_full_view.worker_id AND ");
	}	
	
	/*delete the last fine character' AND ', and add ";"*/
	sql_len = strlen(create_string_db);
	memset(create_string_db+sql_len-5, 0, 5);
	strcat(create_string_db, ";");

	/*print SQL statement*/
	printf("month_worker_statistics_view:%s\n\n\n", create_string_db);

	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, create_string_db);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
	{
		perror("error@Generate_month_worker_statistics():PGRES_COMMAND_OK");
		printf("%s\n",PQerrorMessage(conn_db));
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}
	PQclear(res_db);
	res_db = NULL;

	return 1;
	
}


/*********************************************************
 *project: Line communication charges supermarket
 *filename: update_terminal.c
 *version: 0.1
 *purpose: some function have relationship with update terminal information
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-7-5
 *********************************************************/
 
 #include "update_terminal.h"
 
 /* *************************************************
 *	\brief
 *		Update appointed terminal constant money at both databases
 *  
 *	Function Name: 
 *		int Update_appointed_terminal_total_constant_money(char *terminal_id, 
 *														   char *start_date,
 *														   char *end_date, 
 *														   PGconn *conn_db_a,
 *														   PGconn *conn_db_b);
 *	Input:
 *		char *terminal_id: pointer to terminal id
 *		char *start_date: pointer to start date
 *		char *end_date: pointer to end_date
 *		PGconn *conn_db: pointer to a database a
 *		PGconn *conn_db: pointer to a database b
 *
 *	Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Update_appointed_terminal_total_constant_money(char *terminal_id, 
												   char *start_date,
												   char *end_date, 
												   PGconn *conn_db_a)
{	
	int i = 0;
	int success = 0;
	char *e;
	int charge_index = -1;
	int reversal_index = -1;
	char query_string_db[COMM_LENGTH];

	unsigned long long int total_constant_money = 0;
	PGresult *res_db = NULL;
	int charge_trade_code_index = -1;
	int reversal_trade_code_index = -1;

	/* Check the input parameters */
	if(NULL == conn_db_a)
	{
		perror("error@Update_appointed_terminal_total_constant_money():NULL");
		return -1;
	}
	
	/*start_date == end_date, constant money is 0*/
	if (0 == strcmp(start_date, end_date))
	{
		goto update_constant_money;
	}

	/*get total constant money from all company*/
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
			printf("error@Update_appointed_terminal_total_constant_money():no charge packet found!\n");
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
			printf("error@Update_appointed_terminal_total_constant_money():no reversal packet found!\n");
			return -1;
		}				

		/*get constant money from appointed company*/
		charge_trade_code_index = global_par.company_par_array[i].pkt_par_array[charge_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
		reversal_trade_code_index = global_par.company_par_array[i].pkt_par_array[reversal_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
		if(-1 == charge_trade_code_index)
		{
			charge_trade_code_index = global_par.company_par_array[i].pkt_par_array[charge_index][FORWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
		}
		if(-1 == reversal_trade_code_index)
		{
			reversal_trade_code_index = global_par.company_par_array[i].pkt_par_array[reversal_index][FORWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
		}
		bzero(query_string_db, COMM_LENGTH);
		sprintf(query_string_db,
			"SELECT SUM(money) FROM %s WHERE terminal_id=\'%s\' AND record_date>\'%s\' AND record_date<=\'%s\' AND trade_code=\'%s\' AND serial_number NOT IN (SELECT serial_number FROM %s WHERE terminal_id=\'%s\' AND record_date>\'%s\' AND record_date<=\'%s\' AND TRADE_CODE=\'%s\');", 
			global_par.company_par_array[i].company_name, 
			terminal_id,
			start_date, 
			end_date, 
			global_par.company_par_array[i].pkt_par_array[charge_index][BACKWARD_POSITION].item_par_array[charge_trade_code_index].valid_value,
			global_par.company_par_array[i].company_name,
			terminal_id,
			start_date,
			end_date, 
			global_par.company_par_array[i].pkt_par_array[reversal_index][BACKWARD_POSITION].item_par_array[reversal_trade_code_index].valid_value);
		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db_a, query_string_db);
		if (PQresultStatus(res_db) != PGRES_TUPLES_OK)
		{
			perror("error@Update_appointed_terminal_total_constant_money():PGRES_TUPLES_OK");
			printf("%s\n",PQerrorMessage(conn_db_a));
			PQclear(res_db);
			res_db = NULL;
			return -1;
		}

		if(0 == memcmp(global_par.company_par_array[i].company_name, "bank_card_", 10))
		{
			/*说明是银行卡，代扣网点的钱应当减去*/
			total_constant_money -= strtoull(PQgetvalue(res_db, 0, 0), &e, 10);		
		}
		else
		{
			/*说明是收费业务，代扣网点的钱应当加上*/
			total_constant_money += strtoull(PQgetvalue(res_db, 0, 0), &e, 10);		
		}
	}

update_constant_money:
	/*update constant money of appointed terminal at both databases, we only need to make sure that we update slef database successfully*/
	success = Do_update_total_constant_money(terminal_id, total_constant_money, conn_db_a);
	
	PQclear(res_db);
	res_db = NULL;

	return success;
}

 /* *************************************************
 *	\brief
 *		Do update constant money at a database
 *  
 *	Function Name: 
 *		int Do_update_total_constant_money(char *terminal_id,
 *										   unsigned long long int total_constant_money,
 *										   PGconn *conn_db);
 *	Input:
 *		char *terminal_id: pointer to terminal id
 *		unsigned long long int total_constant_money: constant money need to update
 *		PGconn *conn_db: pointer to a database
 *
 *	Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Do_update_total_constant_money(char *terminal_id,
								   unsigned long long int total_constant_money,
								   PGconn *conn_db)
{
	PGresult *res_db = NULL;
	char update_string_db[COMM_LENGTH];
	
	/*update constant money of appointed terminal*/
	bzero(update_string_db, COMM_LENGTH);
	sprintf(update_string_db,
		"UPDATE terminal_manage SET total_constant_money = %llu WHERE terminal_id = \'%s\';", total_constant_money, terminal_id); 
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, update_string_db);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
	{
		perror("error@Update_appointed_terminal_total_constant_money():PGRES_COMMAND_OK");
		printf("%s\n",PQerrorMessage(conn_db));
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}
	
	PQclear(res_db);
	res_db = NULL;

	return 1;
}

/* *************************************************
 *	\brief
 *		Query bank appointed terminal deposit
 *  
 *	Function Name: 
 *		int Query_bank_appointed_terminal_deposit(char *terminal_id, PGconn *conn_db)
 *
 *	Input:
 *		char *terminal_id: pointer to terminal id
 *		PGconn *conn_db_a: pointer to a database a
 *		PGconn *conn_db_b: pointer to a database b
 *
 *	Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Query_bank_appointed_terminal_deposit(char *terminal_id, PGconn *conn_db_a)
{
	int sd_to_server;
	struct sockaddr_in sa_server;
	int count = 0;
	char buf_send[MAX_PACKET_SIZE] = "00";
	char buf_recv[MAX_PACKET_SIZE] = "00";
	char deposit[DEPOSIT_LENGTH + 1] = "00";

	char query_string_db[COMM_LENGTH];
	PGresult *res_db = NULL;

	/* Check the input parameters(self database must be ok)*/
	if(NULL == conn_db_a)
	{
		perror("error@Query_bank_appointed_terminal_deposit():NULL");
		return -1;
	}
	
	/*get the appointed terminal deduction account*/
	bzero(query_string_db, COMM_LENGTH);
	sprintf(query_string_db,"SELECT deduction_account FROM terminal_manage WHERE terminal_id = \'%s\';", terminal_id);
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db_a, query_string_db);
	if (PQresultStatus(res_db) != PGRES_TUPLES_OK)
	{
		perror("error@Query_bank_appointed_terminal_deposit():PGRES_TUPLES_OK");
		printf("%s\n",PQerrorMessage(conn_db_a));
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}	
	
	/*construct packet to send, packet_length(5)+trade_code(3)+response_code(2)+account(20)*/
	bzero(buf_send, MAX_PACKET_SIZE);
	sprintf(buf_send, "0002500100%s", PQgetvalue(res_db, 0, 0));

	if((sd_to_server = socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("error@Query_bank_appointed_terminal_deposit():socket");
		return -1;
	}

	/*Initialize communication with bank server*/
	Init_comm_server(global_par.system_par.china_bank_ip_address, global_par.system_par.china_bank_port, &sa_server);
	
	/*connect to the bank server*/
	if (0 > connect_server_retry(sd_to_server, (struct sockaddr *)&sa_server, sizeof(struct sockaddr)))
	{
		perror("error:Query_bank_appointed_terminal_deposit:connect");
		close(sd_to_server);
		return -1;
	}
	
	/*send packet*/
	if ((count = send(sd_to_server, buf_send, strlen(buf_send), 0)) <= 0)
	{
		perror("error@Query_bank_appointed_terminal_deposit():send");
		close(sd_to_server);
		return -1;
	}

	/*receive packet*/
	bzero(buf_recv, MAX_PACKET_SIZE);
	if((count = recv(sd_to_server, buf_recv, MAX_PACKET_SIZE, 0)) <= 0)
	{
		perror("error@Query_bank_appointed_terminal_deposit():recv");
		close(sd_to_server);
		return -1;
	}

	/*get information from packet*/
	if (0 != strncmp("00", buf_recv+8, 2))
	{
		close(sd_to_server);
		return -1;
	}
	
	/*get deposit from received packet, now get the backward 10bit*/
	bzero(deposit, DEPOSIT_LENGTH+1);
	strncpy(deposit, buf_recv+50+5, 10);

	/*close socket*/
	close(sd_to_server);

	/*update deposit at self database*/
	if (-1 == Update_appointed_terminal_bank_deposit(terminal_id, conn_db_a, deposit))
	{
		return -1;
	}

	//LOG("(Network Query Deposit=|%s|)",deposit);
	
	return 1;
}

/* *************************************************
 *	\brief
 *		Update appointed terminal deposit
 *  
 *	Function Name: 
 *		int Update_appointed_terminal_bank_deposit(char *terminal_id, PGconn *conn_db)
 *
 *	Input:
 *		char *terminal_id: pointer to terminal id
 *		PGconn *conn_db: pointer to a database
 *		char *deposit: pointer to deposit
 *
 *	Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Update_appointed_terminal_bank_deposit(char *terminal_id, PGconn *conn_db, char *deposit)
{	
	char update_string_db[COMM_LENGTH];
	PGresult *res_db = NULL;

	/* Check the input parameters */
	if(NULL == conn_db)
	{
		perror("error@Update_appointed_terminal_bank_deposit():NULL");
		return -1;
	}

	/*update bank deposit of the appointed terminal*/
	bzero(update_string_db, COMM_LENGTH);
	sprintf(update_string_db,
		"UPDATE terminal_manage SET bank_deposit = %s WHERE terminal_id = \'%s\';", deposit, terminal_id); 
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, update_string_db);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
	{
		perror("error@Update_appointed_terminal_bank_deposit():PGRES_COMMAND_OK");
		printf("%s\n",PQerrorMessage(conn_db));
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}

	PQclear(res_db);
	res_db = NULL;
	
	return 1;
}

/* *************************************************
 *	\brief
 *		Update appointed terminal apply_refresh_flag
 *  
 *	Function Name: 
 *		int Query_bank_appointed_terminal_deposit(char *terminal_id, PGconn *conn_db)
 *
 *	Input:
 *		char *terminal_id: pointer to terminal id
 *		PGconn *conn_db: pointer to a database
 *		int refresh_flag: refresh_flag
 *
 *	Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Update_appointed_terminal_apply_refresh_flag(char *terminal_id, int refresh_flag, PGconn *conn_db)
{	
	char update_string_db[COMM_LENGTH];
	PGresult *res_db = NULL;

	/* Check the input parameters */
	if(NULL == conn_db)
	{
		perror("error@Update_appointed_terminal_apply_refresh_flag:NULL");
		return -1;
	}

	/*after update bank deposit, reset apply refresh flag*/
	bzero(update_string_db, COMM_LENGTH);
	sprintf(update_string_db,
		"UPDATE terminal_manage SET apply_refresh_flag = %d WHERE terminal_id = \'%s\';", refresh_flag, terminal_id); 
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, update_string_db);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
	{
		perror("error@Update_appointed_terminal_apply_refresh_flag():PGRES_COMMAND_OK");
		printf("%s\n",PQerrorMessage(conn_db));
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}

	PQclear(res_db);
	res_db = NULL;
	
	return 1;
}

/***************************************************
 *	\brief
 *		update appointed terminal deposit by manual
 *  
 *	Function Name: 
 *		int Update_manual_appointed_terminal_deposit(char *terminal_id,
 *													 unsigned long long int money,
 *													 PGconn *conn_db)
 *
 *	Input:
 *		char *terminal_id: pointer to terminal id
 *		PGconn *conn_db: pointer to a database
 *		unsigned long long int money: money need to update
 *
 *	Output:
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
int Update_manual_appointed_terminal_deposit(char *terminal_id,
											 char *money,
											 PGconn *conn_db)
{
	char update_string_db[COMM_LENGTH];
	PGresult *res_db = NULL;

	/* Check the input parameters */
	if(NULL == conn_db)
	{
		perror("error@Query_bank_appointed_terminal_deposit():NULL");
		return -1;
	}

	bzero(update_string_db, COMM_LENGTH);
	sprintf(update_string_db,
		"UPDATE terminal_manage SET bank_deposit = %s WHERE terminal_id = \'%s\';", money, terminal_id); 
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, update_string_db);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK)
	{
		perror("Update_manual_appointed_terminal_deposit():PGRES_COMMAND_OK");
		printf("%s\n",PQerrorMessage(conn_db));
		PQclear(res_db);
		res_db = NULL;
		return -1;
	}
	
	PQclear(res_db);
	res_db = NULL;
	
	return 1;
}

/*************************************************************************
 *  \brief
 *    update the latest reckoning date in table terminal_manage 
 *    
 *  \par Input:
 *     terminal_id: the ID of certain terminal.
 *     latest_reckoning_date: the latest reckoning date, the form is YYYY-MM-DD.
 *     conn_db: the handle of databse for operation.
 *
 *  \par Output:
 *        
 *  \Return:
 *    1: success
 *    -1: error
************************************************************************/
int Update_appointed_terminal_latest_reckoning_date(char *terminal_id,
													char *latest_reckoning_date,
													PGconn *conn_db)
{
		PGresult *res_db = NULL;
  		char sql_string[SQL_STRING_LENGTH];

		/* Check the input parameters */
		if(NULL == conn_db)
		{
			perror("error@Update_appointed_terminal_latest_reckoning_date():NULL");
			return -1;
		}

		bzero(sql_string, SQL_STRING_LENGTH); 
		sprintf(sql_string,"update terminal_manage set latest_reckoning_date = \'%s\' where terminal_id = \'%s\';", latest_reckoning_date, terminal_id);
		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, sql_string);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK) 
		{
				perror("error@Update_all_terminal_latest_reckoning_date:PQresultStatus");
				printf("%s\n",PQerrorMessage(conn_db));
				PQclear(res_db);
				res_db = NULL;
				return -1;
		}
		PQclear(res_db);
		res_db = NULL;

		return 1;	
}

/*************************************************************************
 *  \brief
 *    update the latest reckoning date of all terminal in table terminal_manage 
 *    
 *  \par Input:
 *     latest_reckoning_date: the latest reckoning date, the form is YYYY-MM-DD.
 *     conn_db: the handle of databse for operation.
 *
 *  \par Output:
 *        
 *  \Return:
 *    1: success
 *    -1: error
************************************************************************/
int Update_all_terminal_latest_reckoning_date(char *latest_reckoning_date, PGconn *conn_db)
{
		PGresult *res_db = NULL;
  		char sql_string[SQL_STRING_LENGTH];

		/* Check the input parameters */
		if(NULL == conn_db)
		{
			perror("error@Update_all_terminal_latest_reckoning_date():NULL");
			return -1;
		}

		bzero(sql_string, SQL_STRING_LENGTH); 
		sprintf(sql_string,"update terminal_manage set latest_reckoning_date = \'%s\';", latest_reckoning_date);
		PQclear(res_db);
		res_db = NULL;
		res_db = PQexec(conn_db, sql_string);
		if (PQresultStatus(res_db) != PGRES_COMMAND_OK) 
		{
				perror("error@Update_all_terminal_latest_reckoning_datePQresultStatus");
				printf("%s\n",PQerrorMessage(conn_db));
				PQclear(res_db);
				res_db = NULL;
				return -1;
		}
		PQclear(res_db);
		res_db = NULL;

		return 1;	
}

/*************************************************************************
 *  \brief
 *    update the virtual money in table terminal_manage 
 *    before update set the virtual_money_refresh_flag to 1, after update, set it to 0
 *    
 *  \par Input:
 *     terminal_id: the ID of certain terminal.
 *     conn_db: the handle of databse for operation.
 *
 *  \par Output:
 *        
 *  \Return:
 *    1: success
 *    -1: error
************************************************************************/
int Update_appointed_terminal_virtual_available_money(char *terminal_id,
															 PGconn *conn_db)
{
	int success = 0;
	PGresult *res_db = NULL;
	char sql_string[SQL_STRING_LENGTH];
	long long int available_money = 0;
	char available_money_string[SQL_STRING_LENGTH];
	char *e;
	
	if(NULL == conn_db)
	{
		printf("connection with database is broken!\n");
		return -1;
	}
	
	bzero(sql_string, SQL_STRING_LENGTH); 
	sprintf(sql_string,"select available_money from terminal_manage_view where terminal_id = \'%s\';", terminal_id);
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, sql_string);
	if (PQresultStatus(res_db) != PGRES_TUPLES_OK) 
	{
			perror("error@Update_appointed_terminal_virtual_available_money:PQresultStatus");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
			return -1;
	}
	bzero(available_money_string, SQL_STRING_LENGTH);
	sprintf(available_money_string, "%s", PQgetvalue(res_db,0,0));
	available_money = strtoll(available_money_string, &e, 10);
	PQclear(res_db);
	res_db = NULL;
			
	/*update the virtual money in table terminal_manage*/
	bzero(sql_string, SQL_STRING_LENGTH); 
	//sprintf(sql_string,"update terminal_manage set virtual_available_money = %lld where terminal_id = \'%s\';", available_money, terminal_id);
	sprintf(sql_string,"update terminal_accounts set virtual_available_money = %lld where terminal_id = \'%s\';", available_money, terminal_id);
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, sql_string);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK) 
	{
			perror("error@Update_appointed_terminal_virtual_available_money:PQresultStatus");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
			return -1;
	}
	PQclear(res_db);
	res_db = NULL;
	
	success = 1;
	return success;
}

/*************************************************************************
 *  \brief
 *    update the virtual money in table terminal_manage wholely with sql 
 *    before update set the virtual_money_refresh_flag to 1, after update, set it to 0
 *    
 *  \par Input:
 *     conn_db: the handle of databse for operation.
 *
 *  \par Output:
 *        
 *  \Return:
 *    1: success
 *    -1: error
************************************************************************/
int Update_all_terminal_virtual_available_money(PGconn *conn_db)
{
	int success = 0;
	PGresult *res_db = NULL;
	char sql_string[SQL_STRING_LENGTH];
	
	/*refresh the virtual_money wholely*/
	bzero(sql_string, SQL_STRING_LENGTH); 
	sprintf(sql_string,"update terminal_virtual_money set virtual_available_money = terminal_manage_view.available_money from terminal_manage_view where terminal_virtual_money.terminal_id = terminal_manage_view.terminal_id;");
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, sql_string);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK) 
	{
			perror("error@Update_all_terminal_virtual_available_money:PQresultStatus");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
			return -1;
	}
	PQclear(res_db);
	res_db = NULL;
	
	bzero(sql_string, SQL_STRING_LENGTH); 
	//sprintf(sql_string,"update terminal_manage set virtual_available_money = terminal_virtual_money.virtual_available_money from terminal_virtual_money where terminal_virtual_money.terminal_id = terminal_manage.terminal_id;");
	sprintf(sql_string,"update terminal_accounts set virtual_available_money = terminal_virtual_money.virtual_available_money from terminal_virtual_money where terminal_virtual_money.terminal_id = terminal_accounts.terminal_id;");
	PQclear(res_db);
	res_db = NULL;
	res_db = PQexec(conn_db, sql_string);
	if (PQresultStatus(res_db) != PGRES_COMMAND_OK) 
	{
			perror("error@Update_all_terminal_virtual_available_money:PQresultStatus");
			printf("%s\n",PQerrorMessage(conn_db));
			PQclear(res_db);
			res_db = NULL;
			return -1;
	}
	PQclear(res_db);
	res_db = NULL;
	
	success = 1;
	return success;
}


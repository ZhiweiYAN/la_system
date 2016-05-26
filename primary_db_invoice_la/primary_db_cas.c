/* *************************************************
 * File name:
 * 		primary_db_cas.cc
 * Description:
 * 		The program is run at the database server.
 * 		It do CHECK, ADD and SUB the vitural money of each client.
 * Author:
 * 		Rui Su, ssurui@gmai.com
 *			Xiaodan Ge gexiaodan@mail.xjtu.edu.cn
 *			Zhiwei Yan, jerod.yan@gmail.com
 * Date:
 * 		2007-07-26
 * *************************************************/

#include "primary_db_cas.h"
/*!
 *****************************************************************************
 *
 * \brief
 *    Daemon_balance_check_server: carry out these commands such as select.
 *
 * \par Input:
 *    socket id for welcome
 *	 socket structure
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
int Daemon_balance_check_server(int welcome_sd,struct sockaddr_in *sa_business)
{
    char buf_recv[MAX_PACKET_SIZE];
    int count = 0;
    pid_t pid = 0;
    int success = 0;

    int connection_sd = 0;

    socklen_t len = 0;

    /*execution time*/
    struct timeval tpstart,tpend;
    float timeuse = 0;

    /*Check the input parameters*/
    if (0==welcome_sd) {
        perror("error:primary_db_cas:Daemon_balance_check_server():welcome_sd");
    }

    len = sizeof (struct sockaddr);

    /* Enter the Daemon */
    while (1) {
        if (ERROR == Get_ownself_server_mode()) {
            OUTPUT_ERROR;
            LOG(INFO)<<"Get_ownself_server_mode, ERROR.";
            return -1;
        }
        fflush(NULL);
        printf("\nThe Balance Check(CAS) Daemon is waiting for connections .... \n");
        if (( connection_sd = accept(welcome_sd,(struct sockaddr*)sa_business,&len))<0) {
            perror("error:primary_db_cas.c:Daemon_balance_check_server():accept");
            continue;
        }

        fflush(NULL);
        if ((pid = fork()) < 0) {
            perror("error:primary_db_cas.c:Daemon_balance_check_server():fork");
            close(connection_sd);
            continue;
        } else if (0 == pid) {

            /* In the first child process*/
            /* Close the listening socket description */
            close(welcome_sd);
            if ((pid = fork()) < 0) {
                perror("error:primary_db_cas.c:Daemon_balance_check_server():fork2");
            } else if (pid > 0) {
                exit(0); 		/* Return original parent */
            }

            /* In the grandchild process */
            /* Allocate memory for receiving data */
            success = Insert_pid_process_table(getpid(),CAS_PROCESS_DEADLINE,CAS_PROCESS);
            bzero(buf_recv, MAX_PACKET_SIZE);

            /* BUSINESS:Receiving data from business servers*/
            if (0 >= (count = recv(connection_sd,buf_recv,MAX_PACKET_SIZE,0))) {
                perror("error:primary_db_cas.c:Daemon_balance_check_server():recv");
                exit(1);
            }

            DBG("\n%s |%s|", "CAS: Recv data from BusiSrv", buf_recv);


            /* Perhaps the packet is driving packet by SYNC status */
            printf("\nCAS PID %d response one business request.\n",getpid());
            if (0==strncmp(buf_recv,"NULL",4)) {
                success = Remove_pid_process_table(getpid());
                close(connection_sd);
                exit(0);
            }

            /*start time*/
            gettimeofday(&tpstart,NULL);

            /* Database Procedures */
            success = Do_balance_check_procedures(connection_sd,buf_recv,count);

            /*end time*/
            gettimeofday(&tpend,NULL);

            /*calculate the time*/
            timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec) + tpend.tv_usec-tpstart.tv_usec;
            timeuse/=1000000;
            printf("\t\033[32mResponse business packet successfully, used time:%f!\033[0m\n", timeuse);

            /* Free all resources */
            success = Remove_pid_process_table(getpid());
            close(connection_sd);
            exit(0);
        }

        /* Close the connection socket description in the parent process*/
        close(connection_sd);
        /* In the parent process */
        if (waitpid(pid,NULL,0)!=pid)
            perror("primary_db_cas.c:Daemon_balance_check_server:waitpid");
        continue;

    }

}

/*!
 *****************************************************************************
 *
 * \brief
 *    Do_balance_check_procedures: carry out the packet from business_la.
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
int Do_balance_check_procedures(int connection_sd, char *packet, int pkt_len)
{
    int success = 0;
    PGconn* conn_primary_db = NULL;

    char type_str[TYPE_IN_FORWARD_PACKET_LENGTH+1];
    char terminal_id_str[TERMINAL_ID_IN_FORWARD_PACKET_LENGTH+1];
    char money_str[MONEY_IN_FORWARD_PACKET_LENGTH+1];
    unsigned long long int charge_money = 0;
    char response_code_str[RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH+1];
    char backward_pkt[BACKWARD_PACKET_LENGTH+1];

    char *e;

#ifdef _DEBUG_BALANCE_CHECK_ADD_SUBSTRACT_INFO_
    printf("length of packet from business_la : |%d|\n", pkt_len);
    printf("packet from business_la : |%s|\n", packet);
#endif
    /*get the first 2 byte "type"*/
    bzero(type_str, TYPE_IN_FORWARD_PACKET_LENGTH+1);
    memcpy(type_str, packet+TYPE_IN_FORWARD_PACKET_START_POSTION, TYPE_IN_FORWARD_PACKET_LENGTH);

    /*get the 8 byte "terminal_id"*/
    bzero(terminal_id_str, TERMINAL_ID_IN_FORWARD_PACKET_LENGTH+1);
    memcpy(terminal_id_str, packet+TERMINAL_ID_IN_FORWARD_PACKET_START_POSTION, TERMINAL_ID_IN_FORWARD_PACKET_LENGTH);

    /*get the 10 byte "memory"*/
    bzero(money_str, MONEY_IN_FORWARD_PACKET_LENGTH+1);
    memcpy(money_str, packet+MONEY_IN_FORWARD_PACKET_START_POSTION, MONEY_IN_FORWARD_PACKET_LENGTH);
    charge_money = strtoll(money_str, &e, 10);

    /* Get the access right of the databases */
    /* Only to get the DB semaphore first, and get the database connection*/
    /*connect to the database of primary server*/
    conn_primary_db = Connect_db_server(global_par.system_par.database_user[0], global_par.system_par.database_password[0],\
                                        global_par.system_par.database_name, global_par.system_par.database_self_ip_address);

    /*construct the backward pkt*/
    bzero(backward_pkt, BACKWARD_PACKET_LENGTH+1);
    memcpy(backward_pkt, type_str, TYPE_IN_FORWARD_PACKET_LENGTH);
    memcpy(backward_pkt+TERMINAL_ID_IN_BACKWARD_PACKET_START_POSTION, terminal_id_str, TERMINAL_ID_IN_BACKWARD_PACKET_LENGTH);


    if (0 == (memcmp(type_str, CHECK_MONEY_TYPE, TYPE_IN_FORWARD_PACKET_LENGTH))) {
        /*handle the CHECK_MONEY require*/
        success = Response_business_query_real_money_with_bank_card(terminal_id_str, charge_money, conn_primary_db);
        if (1 == success) {
            /*permit this transaction*/
            bzero(response_code_str, RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH+1);
            memcpy(response_code_str, VALID_RESPONSE, RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH);
        } else {
            /*forbit this transaction*/
            bzero(response_code_str, RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH+1);
            memcpy(response_code_str, INVALID_RESPONSE, RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH);
        }
        memcpy(backward_pkt+RESPONSE_CODE_IN_BACKWARD_PACKET_START_POSTION, response_code_str, RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH);

#ifdef _DEBUG_BALANCE_CHECK_ADD_SUBSTRACT_INFO_
        printf("length of packet to business_la : |%d|\n", strlen(backward_pkt));
        printf("packet to business_la : |%s|\n", backward_pkt);
#endif

    } else if (0 == (memcmp(type_str, ADD_MONEY_TYPE, TYPE_IN_FORWARD_PACKET_LENGTH))) {
        /*handle the ADD_MONEY require*/
        bzero(response_code_str, RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH+1);
        memcpy(response_code_str, VALID_RESPONSE, RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH);
        memcpy(backward_pkt+RESPONSE_CODE_IN_BACKWARD_PACKET_START_POSTION, response_code_str, RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH);
#ifdef _DEBUG_BALANCE_CHECK_ADD_SUBSTRACT_INFO_
        printf("length of packet to business_la : |%d|\n", strlen(backward_pkt));
        printf("packet to business_la : |%s|\n", backward_pkt);
#endif

    } else if (0 == (memcmp(type_str, SUBSTRCT_MONEY_TYPE, TYPE_IN_FORWARD_PACKET_LENGTH))) {
        /*handle the SUBSTRUCT_MONEY require*/
        bzero(response_code_str, RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH+1);
        memcpy(response_code_str, VALID_RESPONSE, RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH);
        memcpy(backward_pkt+RESPONSE_CODE_IN_BACKWARD_PACKET_START_POSTION, response_code_str, RESPONSE_CODE_IN_BACKWARD_PACKET_LENGTH);
#ifdef _DEBUG_BALANCE_CHECK_ADD_SUBSTRACT_INFO_
        printf("length of packet to business_la : |%d|\n", strlen(backward_pkt));
        printf("packet to business_la : |%s|\n", backward_pkt);
#endif

    }

    /* Free the resource */
    PQfinish((PGconn*)conn_primary_db);
    //conn_primary_db = NULL;
    if (send(connection_sd, backward_pkt, strlen(backward_pkt),0)==-1) {
        success = 0;
        perror("error@primary_db_cas.c:Do_balance_check_procedures:send");
        exit(1);
    }
    DBG("\n%s |%s|", "CAS: Send data To BusiSrv", backward_pkt);

    return success;
}


/*************************************************************************
 *  \brief
 *    实际计算终端的可用余额是否够缴费金额，加上对商通卡的支持
 *
 *  \par Input:
 *     terminal_id: the ID of certain terminal.
 *		 charge_money: the money of charged
 *     conn_db: the handle of databse for operation.
 *
 *  \par Output:
 *
 *  \Return:
 *    1: permit charge operation
 *    -1: forbit charge operation
************************************************************************/
int Response_business_query_real_money_with_bank_card(char *terminal_id,
        unsigned long long int charge_money,
        PGconn *conn_db_a)
{
    int i = 0;
    int success = 0;
    char *e;
    int charge_index = -1;
    int reversal_index = -1;
    char query_string_db[COMM_LENGTH*4];
    char latest_date[COMM_LENGTH];

    unsigned long long int total_constant_money = 0;
    unsigned long long int total_bank_card_money=0;
    unsigned long long int bank_deposit = 0;
    unsigned long long int yesterday_charge_sum = 0;
    PGresult *res_db = NULL;
    int charge_trade_code_index = -1;
    int reversal_trade_code_index = -1;

    /* Check the input parameters */
    if (NULL == conn_db_a) {
        perror("error@Update_appointed_terminal_total_constant_money():NULL");
        return -1;
    }
    /*得到上次出帐日期*/
    bzero(query_string_db, COMM_LENGTH*4);
    sprintf(query_string_db,
            "SELECT latest_reckoning_date FROM terminal_manage WHERE terminal_id=\'%s\';",
            terminal_id);
    DBG("\n%s |%s|", "CAS: SQL:", query_string_db);
    fflush(NULL);

    PQclear(res_db);
    res_db = NULL;


    res_db = PQexec(conn_db_a, query_string_db);
    if (PQresultStatus(res_db) != PGRES_TUPLES_OK) {
        perror("error@Update_appointed_terminal_total_constant_money():PGRES_TUPLES_OK");
        printf("%s\n",PQerrorMessage(conn_db_a));
        PQclear(res_db);
        res_db = NULL;
        return -1;
    }
    bzero(latest_date, COMM_LENGTH);
    strcpy(latest_date, PQgetvalue(res_db, 0, 0));


    /*逐个公司计算的未结算金额，然后相加得到结果*/
    /*get total constant money from all company*/
    for (i=0; i<global_par.company_num; i++) {
        //usleep(500);
        /*get charge trade code valid value*/
        for (charge_index=0; charge_index<global_par.company_par_array[i].packet_count; ++charge_index) {
            if (2 == global_par.company_par_array[i].packet_important_level[charge_index]) {
                break;
            }
        }
        if (charge_index == global_par.company_par_array[i].packet_count) {
            printf("error@Update_appointed_terminal_total_constant_money():no charge packet found!\n");
            return -1;
        }


        /*get reversal trade code valid value*/
        for (reversal_index=0; reversal_index<global_par.company_par_array[i].packet_count; ++reversal_index) {
            if (1 == global_par.company_par_array[i].packet_important_level[reversal_index]) {
                break;
            }
        }
        if (reversal_index == global_par.company_par_array[i].packet_count) {
            printf("error@Update_appointed_terminal_total_constant_money():no reversal packet found!\n");
            return -1;
        }

        /*get constant money from appointed company*/
        charge_trade_code_index = global_par.company_par_array[i].pkt_par_array[charge_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
        reversal_trade_code_index = global_par.company_par_array[i].pkt_par_array[reversal_index][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
        if (-1 == charge_trade_code_index) {
            charge_trade_code_index = global_par.company_par_array[i].pkt_par_array[charge_index][FORWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
        }
        if (-1 == reversal_trade_code_index) {
            reversal_trade_code_index = global_par.company_par_array[i].pkt_par_array[reversal_index][FORWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
        }
        //printf("before execute |%s|\n", global_par.company_par_array[i].company_name);
        bzero(query_string_db, COMM_LENGTH*4);
        sprintf(query_string_db,
                "SELECT SUM(money) FROM %s WHERE terminal_id=\'%s\' AND record_date=current_date AND trade_code=\'%s\' AND serial_number NOT IN (SELECT serial_number FROM %s WHERE terminal_id=\'%s\' AND record_date=current_date AND TRADE_CODE=\'%s\');",
                //"SELECT SUM(money) FROM %s WHERE terminal_id=\'%s\' AND record_date>\'%s\' AND record_date<=current_date AND trade_code=\'%s\' AND serial_number NOT IN (SELECT serial_number FROM %s WHERE terminal_id=\'%s\' AND record_date>\'%s\' AND record_date<=current_date AND TRADE_CODE=\'%s\');",
                global_par.company_par_array[i].company_name,
                terminal_id,
                //latest_date,
                global_par.company_par_array[i].pkt_par_array[charge_index][BACKWARD_POSITION].item_par_array[charge_trade_code_index].valid_value,
                global_par.company_par_array[i].company_name,
                terminal_id,
                //latest_date,
                global_par.company_par_array[i].pkt_par_array[reversal_index][BACKWARD_POSITION].item_par_array[reversal_trade_code_index].valid_value);
        PQclear(res_db);
        res_db = NULL;
        DBG("CAS: SQL |%s|\n", query_string_db);
        res_db = PQexec(conn_db_a, query_string_db);
        if (PQresultStatus(res_db) != PGRES_TUPLES_OK) {
            perror("error@Update_appointed_terminal_total_constant_money():PGRES_TUPLES_OK");
            printf("%s\n",PQerrorMessage(conn_db_a));
            PQclear(res_db);
            res_db = NULL;
            return -1;
        }
        //printf("end execute |%s|\n", global_par.company_par_array[i].company_name);

        if (0 == memcmp(global_par.company_par_array[i].company_name, "bank_card_", 10)) {
            /*说明是银行卡，银行卡总额缴费应该增加*/
            total_bank_card_money+= strtoll(PQgetvalue(res_db, 0, 0), &e, 10);
            //total_constant_money -= strtoll(PQgetvalue(res_db, 0, 0), &e, 10);
        } else {
            /*说明是收费业务，代扣网点的钱应当加上*/
            total_constant_money += strtoll(PQgetvalue(res_db, 0, 0), &e, 10);
        }

        //total_constant_money += strtoull(PQgetvalue(res_db, 0, 0), &e, 10);
    }

    /*得到银行存款*/
    bzero(query_string_db, COMM_LENGTH*4);
    sprintf(query_string_db,
            "SELECT bank_deposit FROM terminal_manage WHERE terminal_id=\'%s\';",
            terminal_id);
    DBG("CAS: SQL:|%s|\n",query_string_db);
    fflush(NULL);
    PQclear(res_db);
    res_db = NULL;
    res_db = PQexec(conn_db_a, query_string_db);
    if (PQresultStatus(res_db) != PGRES_TUPLES_OK) {
        perror("error@Update_appointed_terminal_total_constant_money():PGRES_TUPLES_OK");
        printf("%s\n",PQerrorMessage(conn_db_a));
        PQclear(res_db);
        res_db = NULL;
        return -1;
    }
    bank_deposit = strtoull(PQgetvalue(res_db, 0, 0), &e, 10);

    /*得到未出帐金额*/
    bzero(query_string_db, COMM_LENGTH*4);
    sprintf(query_string_db,
            "SELECT total_constant_money FROM terminal_manage WHERE terminal_id=\'%s\';",
            terminal_id);
    DBG("CAS: SQL:|%s|\n",query_string_db);
    fflush(NULL);

    PQclear(res_db);
    res_db = NULL;
    res_db = PQexec(conn_db_a, query_string_db);
    if (PQresultStatus(res_db) != PGRES_TUPLES_OK) {
        perror("error@Update_appointed_terminal_total_constant_money():PGRES_TUPLES_OK");
        printf("%s\n",PQerrorMessage(conn_db_a));
        PQclear(res_db);
        res_db = NULL;
        return -1;
    }

    yesterday_charge_sum = strtoull(PQgetvalue(res_db, 0, 0), &e, 10);

    //printf("bank=|%llu|\n", bank_deposit);
    //printf("charge_money=|%llu|\n", charge_money);
    //printf("total_constant_money=|%llu|\n", total_constant_money);
    if ((bank_deposit +total_bank_card_money)>=(charge_money + total_constant_money + yesterday_charge_sum)) {
        success = 1;
    } else {
        success = -1;
    }

    return success;
}



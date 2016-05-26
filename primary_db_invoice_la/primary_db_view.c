/*
 * =====================================================================================
 *
 *       Filename:  primary_db_view.c
 *
 *    Description:  执行查询的相关函数与过程
 *    				query function and process
 *
 *        Version:  1.0
 *        Created:  6/20/2010 2:17:50 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhi-wei YAN (Jerod YAN), jerod.yan@gmail.com
 *        Company:  DrumTm
 *
 * =====================================================================================
 */

#include "primary_db_view.h"
/*!
 *****************************************************************************
 *
 * \brief
 *    Do_db_viewer_procedures: according to the packet from client,
 *    it carry the SQL command from the packet, and carrys out
 *		the commands into database.
 *
 * \par Input:
 *    TCP socket id.
 *    Packet pointer.
 *	 Packet length
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
int Do_db_viewer_procedures(int connection_sd,char *packet,int packet_size)
{
    int success = 0;
    PGconn* conn_db = NULL;
    int com_id = 0;

    assert( NULL != packet);
    assert( 0 <= connection_sd);
    assert( 0 <= packet_size);

    conn_db = Connect_db_server(global_par.system_par.database_user[1],
                                global_par.system_par.database_password[1],
                                global_par.system_par.database_name,
                                global_par.system_par.localhost_ip_address);


    if (NULL==conn_db) {
        OUTPUT_ERROR;
        return -1;
    }

    com_id = Get_company_id(packet, packet_size);
    if ((0 == strcmp(global_par.company_par_array[com_id].packet_encoding,"utf8"))
        ||(0 == strcmp(global_par.company_par_array[com_id].packet_encoding,"UTF8") )) {
        success = PQsetClientEncoding(conn_db, "utf8");
    } else {
        success = PQsetClientEncoding(conn_db, "gbk");
    }
    DBG("\n%s |%s|",
        "Viewer sets the client encoding: ",
        global_par.company_par_array[com_id].packet_encoding);


    /* Check whether the packet is the inner one or not */
    switch (Get_inner_pkt_flag(packet)) {
    case INNER_PACKET_REPEAT_INVOICE_FLAG:
        /* Only search the ownself database */
        DBG("\n%s |%s|",
            "Viewer searches the invoice_table according to the packet",
            packet);
		LOG(INFO)<< "Viewer searches the invoice_table according to the packet:" 
			<< packet;
        success = Search_pkt_invoice_table(connection_sd,packet,strlen(packet),conn_db);
        break;
    case INNER_PACKET_TERMIAL_QUERY_FLAG:
        DBG("\n%s |%s|",
            "Viewer executes the SQL query from the client",
            packet);
		LOG(INFO)<< "Viewer executes the SQL query from the client:" << packet; 
        /* From the query from termials */
        success = Query_db_from_client(connection_sd,packet,strlen(packet),conn_db);
        break;
    case INNER_PACKET_REVERSAL_ASK_FLAG:
        DBG("\n%s |%s|",
            "Viewer judge the inner packet of reversal_ask",
            packet);
		LOG(INFO) << "Viewer judge the inner packet of reversal_ask: " << packet;
        /* check the forward_packet */
        success = Check_forward_packet(connection_sd,packet,strlen(packet),conn_db);
        break;
    default:
        break;
    }

    /* Free the resource */
    PQfinish((PGconn*)(conn_db));
    conn_db = NULL;

    return 1;
}

/*!
 *****************************************************************************
 *
 * \brief
 *    Query_db_from_client: according to the packet from client,
 *    it extracts the SQL command from the packet, and carrys out
 *		the commands into database.
 *
 * \par Input:
 *    packet: the pointer of buffer contained the packet to be parsed.
 *    connection_sd: socket description to business server.
 *		conn_db: the pointer to the connection with DB
 * \par Output:
 *    none
 *
 * \return
 *    1 in case of success
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    about the macro defination 'TEMP_LENGTH'
 *
 * \note
 *    Avoid the Insert SQL , send message to server business via a socket
 *****************************************************************************/
int Query_db_from_client(int connection_sd,char *pkt, size_t pkt_size,PGconn* conn_db)
{
    enum ServerMode mode = READY;

    char *query_string  = NULL;
    char *pkt_header = NULL;
    char *pkt_body = NULL;
    char *results_string = NULL;
    ssize_t count = 0;

    char *p = NULL; /*! for moving the pointer */
    int success = 0;
    PGresult *res = NULL;
    int m = 0;
    int n = 0;
    int field_sum = 0;
    int record_sum = 0;
    size_t memory_space_len = 0;

    pkt_header = (char *)malloc(pkt_size+1);
    if (NULL == pkt_header) {
        OUTPUT_ERROR;
        return -1;
    }
    pkt_body = (char *)malloc(pkt_size+1);
    if (NULL == pkt_body) {
        OUTPUT_ERROR;
        return -1;
    }
    query_string = (char *)malloc(pkt_size*5);
    if (NULL == query_string) {
        OUTPUT_ERROR;
        return -1;
    }

    bzero(query_string,pkt_size*5);
    bzero(pkt_body,pkt_size+1);
    bzero(pkt_header,pkt_size+1);

    /* Cut the packet into two parts header and body */
    memcpy(pkt_header,pkt,PACKET_HEADER_LENGTH);
    memcpy(pkt_body,pkt+PACKET_HEADER_LENGTH,pkt_size-PACKET_HEADER_LENGTH);

    strcpy(query_string,pkt_body);
    DBG("\n%s |%s|",
        "Viewer: SQL clause from client:",
        query_string);

    /* Get the server mode */
    mode = Get_ownself_server_mode();
    switch (mode) {
    case READY:
    case TWINS:
    case ALONE:
        /* ************************************************* */
        /* On OWNSELF DATABASE */
        /* ************************************************* */
        printf("\033[32m\rCarry out the SQL command on OWNSELF database.\033[0m\n ");
        /* Send the query to primary database */
        res = PQexec(conn_db, query_string);
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            OUTPUT_ERROR;
            perror(query_string);
            perror(PQerrorMessage(conn_db));

            LOG(INFO)<<query_string;
            LOG(INFO)<<PQerrorMessage(conn_db);

            printf("%s\n",PQerrorMessage(conn_db));
            /* 返回一个错误信息 */
            success =  Change_packet_response_code(pkt, strlen(pkt),DB_ERROR_POSTGRESQL);
            count = multi_send(connection_sd,pkt,strlen(pkt),0);
            DBG("\n%s |%s|",
                "VIEW Process To Busi_SRV",
                pkt);
            success = 0;
            PQclear(res);
            break;
        }

        /* Construct the packet for clients */
        field_sum = (int)PQnfields(res);
        record_sum = (int)PQntuples(res);

        memory_space_len = field_sum*record_sum*30+PACKET_HEADER_LENGTH+1;
        results_string = (char *)malloc(memory_space_len);
        if (NULL == results_string) {
            OUTPUT_ERROR;
            return -1;
        }
        bzero(results_string,memory_space_len);
        memcpy(results_string,pkt_header,PACKET_HEADER_LENGTH);

        p = results_string+PACKET_HEADER_LENGTH;
        /* The first line 20 bytes*/
        sprintf(p,"%010d%010d\n",record_sum,field_sum);

        /* The detail records from database */
        for (m=0; m<record_sum; m++) {
            /* ATTENTION: set i value as 01 */
            for (n=0; n<field_sum; n++) {
                /* according to type , and generate values */
                strcat(p,"|");
                strcat(p,PQgetvalue(res,m,n));
            }
            strcat(p,"|\n");
        }
        count = multi_send(connection_sd,results_string,strlen(results_string),0);
        DBG("\n%s |%s|",
            "VIEW Process To Busi_SRV",
            results_string);
		LOG(INFO)<< "VIEW Process to Busi_SRV: " << results_string;
        break;
    default:
        printf("\rPID %d be blocked due to its server status.\n",getpid());
        break;
    }

    free(query_string);
    free(pkt_body);
    free(pkt_header);
    free(results_string);
    PQclear(res);

    res = NULL;
    query_string = NULL;
    pkt_body = NULL;
    pkt_header = NULL;
    results_string = NULL;

    return success;
}
/*!!
 *****************************************************************************
 *
 * \brief
 *    Check_forward_packet(): check the reversal packet. The rules is:
 *    	1. One phone number reversal times < 3 in this month.
 *    	2. One client or terminal ,reversal times < 30 today.
 *
 * \par Input:
 *    conn_db: the handle of the connection with the database
 *    packet: the pointer of the packet.
 *    packet_size: the length of the packet.
 *		connection_sd: the socket describtion.
 * \par Output:
 * 	  none
 *
 * \return
 *    1 in case of success, the packet is permitted for the insert action.
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/

int Check_forward_packet(int connection_sd,char *pkt, size_t pkt_size,PGconn* conn_db)
{
    int success = 0;
    int count = 0;
    success =  Check_forward_reversal_packet_valid(conn_db,pkt, pkt_size);

    count = multi_send(connection_sd,pkt,pkt_size,0);

    DBG("SEND TO BUSI: |%s|\n",pkt);
    return 1;

}

/*!
 *****************************************************************************
 *
 * \brief
 *    Search_pkt_invoice_table(): Recover the last action context.
 *
 * \par Input:
 *    conn_db: the handle of the connection with the database
 *    packet: the pointer of the packet.
 *    packet_size: the length of the packet.
 *	 onnection_sd: the socket describtion.
 * \par Output:
 * 	  none
 *
 * \return
 *    1 in case of success, the packet is permitted for the insert action.
 *    0 or negative error code in case of failure
 *
 * \par Side effects
 *    none
 *
 * \note
 *    none
 *****************************************************************************/
int Search_pkt_invoice_table(int connection_sd,char *pkt, size_t pkt_size,PGconn* conn_db)
{
    enum ServerMode mode = READY;

    char sn[TEMP_LENGTH];
    char *query_string  = NULL;
    char *pkt_send = NULL;
    char * results_string = NULL;
    int  results_string_len = 0;

    int success = 0;
    int com_id = 0;
    int pkt_id = 0;
    int serial_number_item_index = 0;
    int sn_len = 0;
    char *e = NULL;
    PGresult *res = NULL;
    ssize_t count = 0;

    struct CommonPacketHeader common_pkt_header ;

    assert(NULL != conn_db);
    assert(NULL != pkt);
    assert(0 <= connection_sd);
    assert(0 <= pkt_size);

    /* 只有发票数据库才存有下行链路包 */
    /* Only the invoice DB saves the downlink pkt */
    if (INVOICE_DB!=Get_ownself_server_type()) {
        return -1;
    }

    /* Get the common packet header */
    bzero(&common_pkt_header,sizeof(struct CommonPacketHeader));
    success = Get_common_header(pkt,&common_pkt_header);
	
    /* Get the company id */
    com_id = strtol(common_pkt_header.company_id,&e,10);
    pkt_id = strtol(common_pkt_header.service_id,&e,10);
    DBG("\nCOMID=%d, PKTID=%d\n", com_id,pkt_id);
    fflush(NULL);
	
    /* Get the trade code value from the forward packet*/
    serial_number_item_index = global_par.company_par_array[com_id].pkt_par_array[pkt_id][BACKWARD_POSITION].item_index[SERIAL_NUMBER_ITEM_INDEX];
    sn_len = global_par.company_par_array[com_id].pkt_par_array[pkt_id][BACKWARD_POSITION].item_par_array[serial_number_item_index].len;

    DBG("\nSN_NUM_ITME_IDX=%d, LEN_SN=%d\n", serial_number_item_index,sn_len);

    bzero(sn, TEMP_LENGTH);
    memcpy(sn, pkt+PACKET_HEADER_LENGTH,sn_len);

    /* Create SQL query command strings */
    query_string = (char *)malloc(MAXPACKETSIZE_DB);
    if (NULL == query_string) {
        OUTPUT_ERROR;
        goto END;
    }
    bzero(query_string,MAXPACKETSIZE_DB);

    /* Create search query string of SQL */
    sprintf(query_string,"UPDATE %s SET fetch_invoice = fetch_invoice +1 WHERE trim(SERIAL_NUMBER)=trim(\'%s\') AND TERMINAL_ID=\'%s\';SELECT back_pkt_utf8,fetch_invoice FROM %s WHERE trim(SERIAL_NUMBER)=trim(\'%s\') AND TERMINAL_ID=\'%s\' order by ID DESC;",
            global_par.company_par_array[com_id].company_name,
            sn,
            common_pkt_header.terminal_id,
            global_par.company_par_array[com_id].company_name,
            sn,
            common_pkt_header.terminal_id
           );

    /* Get the server mode */
    mode = Get_ownself_server_mode();
    switch (mode) {
    case READY:
    case TWINS:
    case ALONE:
        printf("\033[32m\rGetting a record from INVOICE database.\033[0m\n ");
        /* Send the query to  database */
        PQclear(res);
        DBG("\n%s |%s|\n", "View: Query SQL:", query_string);
        res = PQexec(conn_db, query_string);

        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            OUTPUT_ERROR;
            perror(query_string);
            perror(PQerrorMessage(conn_db));

            LOG(INFO)<<query_string;
            LOG(INFO)<<PQerrorMessage(conn_db);

            PQclear(res);
            res = NULL;
            break;
        }
        /* If there are more than one records, return the first one */
        if (PQntuples(res)>=1) {

            /* Only return first tuple*/
            results_string = PQgetvalue(res,0,0);
            results_string_len = PQgetlength(res, 0, 0);

            pkt_send = (char *)malloc(results_string_len+PACKET_HEADER_LENGTH);
            if (NULL == pkt_send) {
                OUTPUT_ERROR;
                goto END;
            }

            bzero(pkt_send,results_string_len+PACKET_HEADER_LENGTH);
            // memcpy(pkt_send,pkt,PACKET_HEADER_LENGTH);
            memcpy(pkt_send,results_string, results_string_len);

            count = multi_send(connection_sd,pkt_send,results_string_len,0 );
            DBG("\n%s |%s|\n", "View: Send data to BusiSrv:", pkt_send);

        } else {
            success = Change_packet_response_code(pkt,PACKET_HEADER_LENGTH,DB_ERROR_NO_RECORDS);
            count = multi_send(connection_sd,pkt,PACKET_HEADER_LENGTH,0 );
            DBG("\n%s |%s|\n", "View: Send data to BusiSrv:", pkt);

        }
        break;
    default:
        success = Change_packet_response_code(pkt,PACKET_HEADER_LENGTH,DB_ERROR_FIXING);
        count = multi_send(connection_sd,pkt,PACKET_HEADER_LENGTH,0 );
        DBG("\n%s |%s|\n", "View: Send data to BusiSrv:", pkt);
        break;
    }
END:
    free(query_string);
    results_string = NULL;

    free(pkt_send);
    pkt_send = NULL;

    free(results_string);
    query_string  = NULL;

    PQclear(res);
    res = NULL;

    return 1;
}



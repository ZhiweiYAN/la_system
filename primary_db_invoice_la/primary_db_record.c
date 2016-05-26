/*
 * =====================================================================================
 *
 *       Filename:  primary_db_record.c
 *
 *    Description:  record the payment packet into the database
 *
 *        Version:  1.0
 *        Created:  6/14/2010 10:17:07 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhi-wei YAN (Jerod YAN), jerod.yan@gmail.com
 *        Company:  DrumTm
 *
 * =====================================================================================
 */
#include "primary_db_record.h"
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Do_database_record_procedures()
 *  Description:  (处理来自交易通讯端口的数据包)
 *  			  Deal with the packets from the payment socket.
 * =====================================================================================
 */
int Do_database_record_procedures(int connection_sd,char *packet,int packet_size)
{
    int success = 0;

    PGconn* conn_db = NULL;

    //int im_level = 0;
    char send_busi[MAXPACKETSIZE];
    ssize_t count = 0;
    int com_id = 0;

    /* Get database connection pointers of primary db. */
    bzero(send_busi, MAXPACKETSIZE);
    conn_db = Connect_db_server(global_par.system_par.database_user[0],
                                global_par.system_par.database_password[0],
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

    if (0!=success) {
        OUTPUT_ERROR;
    }

    /* Check whether the packet is the inner one or not */
    switch (Get_inner_pkt_flag(packet)) {
    case OUTER_PACKET_FLAG:
        //im_level=Get_import_level(packet);
        //if (im_level>0&&im_level<=10) {
        success = Record_pkt_regular_table(packet,packet_size,conn_db,send_busi);
        //}

        //if (im_level>=11) {
        /* For  printing invoice */
        //ssurui modify 20100424
        //    success = Get_back_pkt_for_business_srv(packet,packet_size,send_busi);
        //}
        break;
    case INNER_PACKET_REPEAT_INVOICE_FLAG:
        break;
    case INNER_PACKET_TERMIAL_QUERY_FLAG:
        break;
    case INNER_PACKET_REVERSAL_ASK_FLAG:
        break;
    default:
        break;
    }

    count = multi_send(connection_sd, send_busi, strlen(send_busi),0 );
    DBG("\n%s |%s|\n","Record send to BusiSrv",send_busi);

    if (0>count) {
        OUTPUT_ERROR;
    }

    /* Free the resource */
    PQfinish((PGconn*)(conn_db));
    conn_db = NULL;

    return 1;
}


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Record_pkt_regular_table
 *  Description:  record the payment packets
 * =====================================================================================
 */
int Record_pkt_regular_table( char *pkt, int pkt_size,
                              PGconn *conn_db,
                              char* backward_pkt)
{
    char query_string[MAX_QUERY_LENGTH];
    int success = 0;
    PGresult *res = NULL;
    char *back_pkt = NULL;
    char *fwd_pkt = NULL;

    struct CompoundPacketInfo com_pkt_info;

    if (NULL==pkt) {
        OUTPUT_ERROR;
        return -1;
    }
    if (0>=pkt_size) {
        OUTPUT_ERROR;
        LOG(INFO)<<pkt;
        return -1;
    }

    if (NULL==conn_db) {
        OUTPUT_ERROR;
        LOG(INFO)<<pkt;
        return -1;
    }

    if (NULL==backward_pkt) {
        OUTPUT_ERROR;
        LOG(INFO)<<pkt;
        return -1;
    }

    bzero(&com_pkt_info,sizeof(struct CompoundPacketInfo));
    success = Get_compound_pkt_info(pkt, pkt_size,&com_pkt_info);

    /* allocate the backward packet memory and reconstruct the backward packet */
    back_pkt = (char *)malloc(pkt_size+1);
    if (NULL==back_pkt) {
        OUTPUT_ERROR;
        return -1;
    }
    bzero(back_pkt,pkt_size+1);

    memcpy(back_pkt, pkt, PACKET_HEADER_LENGTH);
    memcpy(back_pkt+PACKET_HEADER_LENGTH,
           pkt+PACKET_HEADER_LENGTH+com_pkt_info.forward_pkt_len,
           com_pkt_info.backward_pkt_len);
    memset(back_pkt+PACKET_HEADER_LENGTH-ERROR_MEMO_LENGTH,
           ' ',
           ERROR_MEMO_LENGTH);

    /* allocate the forward packet memory and reconstruct the forward packet */
    fwd_pkt = (char *)malloc(pkt_size+1);
    if (NULL==fwd_pkt) {
        OUTPUT_ERROR;
        return -1;
    }
    bzero(fwd_pkt,pkt_size+1);
    memcpy(fwd_pkt, pkt, PACKET_HEADER_LENGTH);
    memcpy(fwd_pkt+PACKET_HEADER_LENGTH,
           pkt+PACKET_HEADER_LENGTH,
           com_pkt_info.forward_pkt_len);
    memset(fwd_pkt+PACKET_HEADER_LENGTH-ERROR_MEMO_LENGTH,
           ' ',
           ERROR_MEMO_LENGTH);

    //SQL string is created
    bzero(query_string,MAX_QUERY_LENGTH);

    if (INVOICE_DB == Get_ownself_server_type()) {
        success = Generate_company_record_with_invoice_from_two_packet(fwd_pkt,
                  strlen(fwd_pkt),
                  back_pkt,strlen(back_pkt),
                  query_string,
                  MAX_QUERY_LENGTH);
    } else {
        success = Generate_company_record_from_two_packet(fwd_pkt,
                  strlen(fwd_pkt),
                  back_pkt,strlen(back_pkt),
                  query_string,
                  MAX_QUERY_LENGTH);

    }

    /* Send the query to primary database */
    res = PQexec(conn_db, query_string);
    DBG("\n%s |%s|\n","Record: SQL string", query_string);
	LOG(INFO)<<"Record: SQL string: "<<query_string;

    /* Did the record action fail in the primary database? */
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        OUTPUT_ERROR;
        perror(query_string);
        perror(PQerrorMessage(conn_db));

        LOG(INFO)<<query_string;
        LOG(INFO)<<PQerrorMessage(conn_db);

        success = Change_packet_response_code(back_pkt,strlen(back_pkt),DB_ERROR_RECORDSQL);
    }
    memcpy(backward_pkt, back_pkt, strlen(back_pkt));

    PQclear(res);
    res = NULL;

    free(back_pkt);
    back_pkt = NULL;
    free(fwd_pkt);
    fwd_pkt = NULL;

    return success;
}


int Get_back_pkt_for_business_srv(char *pkt,int pkt_size, char* backward_pkt)
{
    int success = 0;
    char *back_pkt = NULL;

    struct CompoundPacketInfo com_pkt_info;

    bzero(&com_pkt_info,sizeof(struct CompoundPacketInfo));
    success = Get_compound_pkt_info(pkt, pkt_size,&com_pkt_info);

    back_pkt = (char *)malloc(pkt_size+1);
    if (NULL==back_pkt) {
        OUTPUT_ERROR;
        return -1;
    }
    bzero(back_pkt,pkt_size+1);

    /* Reconstruct the backward packet */
    memcpy(back_pkt,pkt,PACKET_HEADER_LENGTH);
    memcpy(back_pkt+PACKET_HEADER_LENGTH,pkt+PACKET_HEADER_LENGTH+com_pkt_info.forward_pkt_len,com_pkt_info.backward_pkt_len);
    memset(back_pkt+PACKET_HEADER_LENGTH-ERROR_MEMO_LENGTH,' ',ERROR_MEMO_LENGTH);
    memcpy(backward_pkt, back_pkt,strlen(back_pkt));

    free(back_pkt);
    back_pkt = NULL;

    return 1;
}


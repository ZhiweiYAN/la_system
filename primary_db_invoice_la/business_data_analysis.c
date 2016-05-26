/*
 * =====================================================================================
 *
 *       Filename:  business_data_analysis.c
 *
 *    Description:  Analyze the packets from business machines
 *    				(分析来自业务机的数据包)
 *
 *        Version:  1.0
 *        Created:  6/12/2010 10:49:24 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhi-wei YAN (Jerod YAN), jerod.yan@gmail.com
 *        Company:  DrumTm
 *
 * =====================================================================================
 */

#include "business_data_analysis.h"

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Get_company_id
 *  Description:  得到业务（或者公司）的索引号
 * =====================================================================================
 */
int Get_company_id(char *pkt, int pkt_size)
{
    int success = 0;
    int com_id = 0;
    char *e = NULL;

    char company_id[10];
    bzero(company_id, 10);

    success = Get_common_header_company_id(pkt, company_id);

    /* Get the company id */
    com_id = strtol(company_id,&e,10);

    return com_id;

}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Generate_company_record_from_two_packet
 *  Description:  (根据上行包和下行包，生成记录到主数据库或备数据库的SQL命令的字符串)
 *  			  Generate the SQL string according the forward packet or backward
 *  			  packet. The SQL command will be executed on MAIN_DB or SLAVE_DB.
 * =====================================================================================
 */
int Generate_company_record_from_two_packet(char *fwd_pkt, int fwd_pkt_size,
        char* back_pkt, int back_pkt_size,
        char* query_string, int query_string_size)
{
    int i = 0;
    int success = 0;
    char *e = NULL;

    char tmp_field_value[TEMP_LENGTH];
    char tmp_field_name[TEMP_LENGTH];

    int com_id = 0;
    int sevice_id = 0;
    int field_sum = 0;

    char company_name[COMPANY_NAME_LENGTH];

    char field_names[TEMP_LENGTH];
    char field_names_common_pkt_header[TEMP_LENGTH];
    char field_values[TEMP_LENGTH];
    char field_values_common_pkt_header[TEMP_LENGTH];

    char *company_pkt = NULL;
    item_parameters *p = NULL;
    struct CommonPacketHeader *common_pkt_header = NULL;

    char curr_time[6];
    bzero(curr_time,6);
    success = Get_curr_time(curr_time);

    common_pkt_header = (struct CommonPacketHeader *)malloc(sizeof(struct CommonPacketHeader));
    if (NULL == common_pkt_header) {
        MALLOC_ERROR;
        return -1;
    }

    bzero(common_pkt_header,sizeof(struct CommonPacketHeader));
    bzero(field_names_common_pkt_header,TEMP_LENGTH);
    bzero(field_values_common_pkt_header,TEMP_LENGTH);
    bzero(field_names,TEMP_LENGTH);
    bzero(field_values,TEMP_LENGTH);

    /* Get the common packet header */
    success = Get_common_header(back_pkt, common_pkt_header);

    /* Get the company id */
    com_id = strtol(common_pkt_header->company_id,&e,10);

    /* Get the service type or id */
    sevice_id = strtol(common_pkt_header->service_id,&e,10);

    /* Get the backward packet format, such as commany_name*/
    bzero(company_name,COMPANY_NAME_LENGTH);
    strcpy(company_name, global_par.company_par_array[com_id].company_name);
    field_sum = global_par.company_par_array[com_id].pkt_par_array[sevice_id][BACKWARD_POSITION].item_count;
    p = global_par.company_par_array[com_id].pkt_par_array[sevice_id][BACKWARD_POSITION].item_par_array;

    /* Cut backward packet header */
    company_pkt = back_pkt + PACKET_HEADER_LENGTH ;

    /* Exact the field value from the back packet */
    for (i=0; i<field_sum; i++,p++) {

        /* Skip the field not to be recorded */
        if (0==p->db_record) {
            continue;
        }
        /* The field will be insert into the database */
        bzero(tmp_field_value,TEMP_LENGTH);
        bzero(tmp_field_name,TEMP_LENGTH);

        /* 从数据包中取一个字段，对于从头开始计算位置的字段 */
        /* Take a filed from the packet if the position is calculated from the FRONT of the packet */
        if (1==p->db_record&&0==p->direction) {
            memcpy(tmp_field_value,company_pkt+(p->start_pos),p->len);
        }
        /* 从数据包中取一个字段，对于从尾巴开始计算位置的字段 */
        /* Take a filed from the packet if the position is calculated from the END of the packet */
        if (1==p->db_record&&1==p->direction) {
            memcpy(tmp_field_value,company_pkt+(back_pkt_size-1-p->start_pos-PACKET_HEADER_LENGTH),p->len);
        }
        strcpy(tmp_field_name,p->db_alias_name);
        strcat(field_names,tmp_field_name);
        strcat(field_names,",");
        /* Different format for inserting the record*/
        switch (p->db_alias_type) {

        case 0:
            strcat(field_values,tmp_field_value);
            strcat(field_values,",");
            break;
        case 1:
            strcat(field_values,"\'");
            strcat(field_values,tmp_field_value);
            strcat(field_values,"\'");
            strcat(field_values,",");
            break;
        case 2:
            strcat(field_values,"DATE ");
            strcat(field_values,"\'");
            strcat(field_values,tmp_field_value);
            strcat(field_values,"\'");
            strcat(field_values,",");
            break;
        case 3:
            strcat(field_values,"\'");
            strcat(field_values,tmp_field_value);
            strcat(field_values,"\'");
            strcat(field_values,",");
            break;
        default:
            break;
        }
    }

    /* Get the forward packet format */
    field_sum = global_par.company_par_array[com_id].pkt_par_array[sevice_id][FORWARD_POSITION].item_count;
    p = global_par.company_par_array[com_id].pkt_par_array[sevice_id][FORWARD_POSITION].item_par_array;

    /* Cut forward packet header */
    company_pkt = fwd_pkt + PACKET_HEADER_LENGTH ;
    for (i=0; i<field_sum; i++,p++) {

        /* Skip the field not to be recorded */
        if (0==p->db_record) {
            continue;
        }
        /* The field will be insert into the database */
        bzero(tmp_field_value,TEMP_LENGTH);
        bzero(tmp_field_name,TEMP_LENGTH);

        /* 从数据包中取一个字段，对于从头开始计算位置的字段 */
        /* Take a filed from the packet if the position is calculated from the FRONT of the packet */
        if (1==p->db_record&&0==p->direction) {
            memcpy(tmp_field_value,company_pkt+(p->start_pos),p->len);
        }
        /* 从数据包中取一个字段，对于从尾巴开始计算位置的字段 */
        /* Take a filed from the packet if the position is calculated from the END of the packet */
        if (1==p->db_record&&1==p->direction) {
            memcpy(tmp_field_value,company_pkt+(back_pkt_size-1-p->start_pos-PACKET_HEADER_LENGTH),p->len);
        }
        strcpy(tmp_field_name,p->db_alias_name);
        strcat(field_names,tmp_field_name);
        strcat(field_names,",");
        /* Different format for inserting the record*/
        switch (p->db_alias_type) {

        case 0:
            strcat(field_values,tmp_field_value);
            strcat(field_values,",");
            break;
        case 1:
            strcat(field_values,"\'");
            strcat(field_values,tmp_field_value);
            strcat(field_values,"\'");
            strcat(field_values,",");
            break;
        case 2:
            strcat(field_values,"DATE ");
            strcat(field_values,"\'");
            strcat(field_values,tmp_field_value);
            strcat(field_values,"\'");
            strcat(field_values,",");
            break;
        case 3:
            strcat(field_values,"\'");
            strcat(field_values,tmp_field_value);
            strcat(field_values,"\'");
            strcat(field_values,",");
            break;
        default:
            break;
        }
    }


    sprintf(field_names_common_pkt_header,
            "COMPANY_ID,SERVICE_ID,INNER_FLAG, TERMINAL_ID, WORKER_ID, CONTRACT_ID,PHONE_NUMBER,MONEY,RECORD_TIME");
    sprintf(field_values_common_pkt_header,
            "\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',%s,\'%s\'",
            common_pkt_header->company_id,
            common_pkt_header->service_id,
            common_pkt_header->inner_flag,
            common_pkt_header->terminal_id,
            common_pkt_header->worker_id,
            common_pkt_header->contract_id,
            common_pkt_header->phone_number,
            common_pkt_header->money,curr_time);

    bzero(query_string,query_string_size);
    sprintf(query_string,
            "INSERT INTO %s (%s %s) VALUES (%s %s);",
            company_name,
            field_names,
            field_names_common_pkt_header,
            field_values,
            field_values_common_pkt_header);

    fflush(NULL);
    free(common_pkt_header);
    common_pkt_header = NULL;
    return 1;
}


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Generate_company_record_with_invoice_from_two_packet
 *  Description:  生成可以记录交费发票的SQL插入命令
 *  			  Generate the SQL command string which can record all fields include
 *  			  invoice pkt(backward packet) and forward packet.
 * =====================================================================================
 */

int Generate_company_record_with_invoice_from_two_packet(char *fwd_pkt, int fwd_pkt_size,
        char* back_pkt, int back_pkt_size,
        char* query_string, int query_string_size)
{
    int i = 0;
    int success = 0;
    char *e = NULL;

    char tmp_field_value[TEMP_LENGTH];
    char tmp_field_name[TEMP_LENGTH];

    int com_id = 0;
    int sevice_id = 0;
    int field_sum = 0;

    char company_name[COMPANY_NAME_LENGTH];

    char field_names[TEMP_LENGTH];
    char field_names_common_pkt_header[TEMP_LENGTH];
    char field_values[TEMP_LENGTH];
    char field_values_common_pkt_header[TEMP_LENGTH];

    char *company_pkt = NULL;
    item_parameters *p = NULL;
    struct CommonPacketHeader *common_pkt_header = NULL;

    char curr_time[6];
    bzero(curr_time,6);
    success = Get_curr_time(curr_time);

    common_pkt_header = (struct CommonPacketHeader *)malloc(sizeof(struct CommonPacketHeader));
    if (NULL == common_pkt_header) {
        MALLOC_ERROR;
        return -1;
    }

    bzero(common_pkt_header,sizeof(struct CommonPacketHeader));
    bzero(field_names_common_pkt_header,TEMP_LENGTH);
    bzero(field_values_common_pkt_header,TEMP_LENGTH);
    bzero(field_names,TEMP_LENGTH);
    bzero(field_values,TEMP_LENGTH);

    /* Get the common packet header */
    success = Get_common_header(back_pkt, common_pkt_header);

    /* Get the company id */
    com_id = strtol(common_pkt_header->company_id,&e,10);

    /* Get the service type or id */
    sevice_id = strtol(common_pkt_header->service_id,&e,10);

    /* Get the backward packet format, such as commany_name*/
    bzero(company_name,COMPANY_NAME_LENGTH);
    strcpy(company_name, global_par.company_par_array[com_id].company_name);
    field_sum = global_par.company_par_array[com_id].pkt_par_array[sevice_id][BACKWARD_POSITION].item_count;
    p = global_par.company_par_array[com_id].pkt_par_array[sevice_id][BACKWARD_POSITION].item_par_array;

    /* Cut backward packet header */
    company_pkt = back_pkt + PACKET_HEADER_LENGTH ;

    /* Exact the field value from the back packet */
    for (i=0; i<field_sum; i++,p++) {

        /* Skip the field not to be recorded */
        if (0==p->db_record) {
            continue;
        }
        /* The field will be insert into the database */
        bzero(tmp_field_value,TEMP_LENGTH);
        bzero(tmp_field_name,TEMP_LENGTH);

        /* 从数据包中取一个字段，对于从头开始计算位置的字段 */
        /* Take a filed from the packet if the position is calculated from the FRONT of the packet */
        if (1==p->db_record&&0==p->direction) {
            memcpy(tmp_field_value,company_pkt+(p->start_pos),p->len);
        }
        /* 从数据包中取一个字段，对于从尾巴开始计算位置的字段 */
        /* Take a filed from the packet if the position is calculated from the END of the packet */
        if (1==p->db_record&&1==p->direction) {
            memcpy(tmp_field_value,company_pkt+(back_pkt_size-1-p->start_pos-PACKET_HEADER_LENGTH),p->len);
        }
        strcpy(tmp_field_name,p->db_alias_name);
        strcat(field_names,tmp_field_name);
        strcat(field_names,",");
        /* Different format for inserting the record*/
        switch (p->db_alias_type) {

        case 0:
            strcat(field_values,tmp_field_value);
            strcat(field_values,",");
            break;
        case 1:
            strcat(field_values,"\'");
            strcat(field_values,tmp_field_value);
            strcat(field_values,"\'");
            strcat(field_values,",");
            break;
        case 2:
            strcat(field_values,"DATE ");
            strcat(field_values,"\'");
            strcat(field_values,tmp_field_value);
            strcat(field_values,"\'");
            strcat(field_values,",");
            break;
        case 3:
            strcat(field_values,"\'");
            strcat(field_values,tmp_field_value);
            strcat(field_values,"\'");
            strcat(field_values,",");
            break;
        default:
            break;
        }
        fflush(NULL);
    }
    /* Get the forward packet format */
    field_sum = global_par.company_par_array[com_id].pkt_par_array[sevice_id][FORWARD_POSITION].item_count;
    p = global_par.company_par_array[com_id].pkt_par_array[sevice_id][FORWARD_POSITION].item_par_array;

    /* Cut forward packet header */
    company_pkt = fwd_pkt + PACKET_HEADER_LENGTH ;
    for (i=0; i<field_sum; i++,p++) {

        /* Skip the field not to be recorded */
        if (0==p->db_record) {
            continue;
        }
        /* The field will be insert into the database */
        bzero(tmp_field_value,TEMP_LENGTH);
        bzero(tmp_field_name,TEMP_LENGTH);

        /* 从数据包中取一个字段，对于从头开始计算位置的字段 */
        /* Take a filed from the packet if the position is calculated from the FRONT of the packet */
        if (1==p->db_record&&0==p->direction) {
            memcpy(tmp_field_value,company_pkt+(p->start_pos),p->len);
        }
        /* 从数据包中取一个字段，对于从尾巴开始计算位置的字段 */
        /* Take a filed from the packet if the position is calculated from the END of the packet */
        if (1==p->db_record&&1==p->direction) {
            memcpy(tmp_field_value,company_pkt+(back_pkt_size-1-p->start_pos-PACKET_HEADER_LENGTH),p->len);
        }
        strcpy(tmp_field_name,p->db_alias_name);
        strcat(field_names,tmp_field_name);
        strcat(field_names,",");
        /* Different format for inserting the record*/
        switch (p->db_alias_type) {

        case 0:
            strcat(field_values,tmp_field_value);
            strcat(field_values,",");
            break;
        case 1:
            strcat(field_values,"\'");
            strcat(field_values,tmp_field_value);
            strcat(field_values,"\'");
            strcat(field_values,",");
            break;
        case 2:
            strcat(field_values,"DATE ");
            strcat(field_values,"\'");
            strcat(field_values,tmp_field_value);
            strcat(field_values,"\'");
            strcat(field_values,",");
            break;
        case 3:
            strcat(field_values,"\'");
            strcat(field_values,tmp_field_value);
            strcat(field_values,"\'");
            strcat(field_values,",");
            break;
        default:
            break;
        }
    }


    sprintf(field_names_common_pkt_header,
            "COMPANY_ID,SERVICE_ID,INNER_FLAG,TERMINAL_ID,WORKER_ID,CONTRACT_ID,PHONE_NUMBER,MONEY,RECORD_TIME,");
    sprintf(field_values_common_pkt_header,
            "\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',\'%s\',%s,\'%s\',",
            common_pkt_header->company_id,
            common_pkt_header->service_id,
            common_pkt_header->inner_flag,
            common_pkt_header->terminal_id,
            common_pkt_header->worker_id,
            common_pkt_header->contract_id,
            common_pkt_header->phone_number,
            common_pkt_header->money,curr_time);

    /* 处理电信的半个汉字问题 */
    bzero(query_string,query_string_size);

    /* deal with the problem of the half part of a chinese character in china telecom packet. */

    success = Fix_invalid_string_with_space((unsigned char*)back_pkt,
                                            back_pkt_size);

    sprintf(query_string,
            "INSERT INTO %s (%s %s FWD_PKT_UTF8,BACK_PKT_UTF8,FETCH_INVOICE) VALUES (%s %s \'%s\',\'%s\',%d);",
            company_name,
            field_names_common_pkt_header,
            field_names,
            field_values_common_pkt_header,
            field_values,
            fwd_pkt,
            back_pkt,
            0);
    fflush(NULL);

    free(common_pkt_header);
    common_pkt_header = NULL;
    return 1;
}



/*!!
 *****************************************************************************
 *
 * \brief
 *    Check_forward_reversal_packet_valid(): check the reversal packet. The rules is:
 *    	1. One phone number reversal times < 3 in this month.
 *    	2. One client or terminal ,reversal times < 30 today.
 *
 * \par Input:
 *    conn_db: the handle of the connection with the database
 *    packet: the pointer of the packet.
 *    packet_size: the length of the packet.
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
int Check_forward_reversal_packet_valid(PGconn* conn_db,char *packet, int packet_size)
{
    PGresult *res = NULL;

    int i = 0;
    int success = 0;
    int com_id = 0;
    int pkt_id = 0;
    int field_sum = 0;
    int valid_val = 0;
    char *e = NULL;
    char *company_pkt = NULL;
    item_parameters *p = NULL;

    char company_name[COMPANY_NAME_LENGTH];
    char query_string[MAX_QUERY_LENGTH];
    char tmp_phone_number[TEMP_LENGTH];
    char tmp_serial_number[TEMP_LENGTH];
    char field_names[TEMP_LENGTH];
    char field_values[TEMP_LENGTH];

    char trade_code[TEMP_LENGTH];
    int service_type_item_index = 0;
    int im_level = 0;

    struct CommonPacketHeader common_pkt_header;

    /* 根据序列号找到终端的ID号 */
    /* Find the terminal id or client id according to the serial number */
    success = Find_clientid_according_serialnumber(conn_db, packet, packet_size);
    if (-1==success) {
        success = Change_packet_response_code(packet,packet_size,DB_ERROR_NO_RECORDS);
        return -1;
    }

    DBG("\n%s |%s|", "There exists a valid serial number. ",packet );


    bzero(field_names,TEMP_LENGTH);
    bzero(field_values,TEMP_LENGTH);
    bzero(tmp_serial_number,TEMP_LENGTH);
    bzero(tmp_phone_number,TEMP_LENGTH);
    bzero(query_string,MAX_QUERY_LENGTH);

    bzero(&common_pkt_header,sizeof(struct CommonPacketHeader));

    /* Get the packet header */
    success = Get_common_header(packet, &common_pkt_header);

    /* Get the company id */
    com_id = strtol(common_pkt_header.company_id,&e,10);

    /*Get the service type*/
    pkt_id = strtol(common_pkt_header.service_id,&e,10);

    /* Get the important level value */
    im_level = global_par.company_par_array[com_id].packet_important_level[pkt_id];

    /* If the packet is not reversal one */
    if (1!=im_level)
        return 1;
    /* These codes in the following point to the reveral packet */

    DBG("\n%s |%s|", "The important level is ONE. ",packet );

    /* Get the trade code value */
    service_type_item_index = global_par.company_par_array[com_id].pkt_par_array[pkt_id][FORWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
    bzero(trade_code,TEMP_LENGTH);
    strcpy(trade_code, global_par.company_par_array[com_id].pkt_par_array[pkt_id][FORWARD_POSITION].item_par_array[service_type_item_index].valid_value);

    company_pkt = packet+PACKET_HEADER_LENGTH;

    DBG("\n%s %s, |%s|", "Trade code is : ",trade_code, packet );

    /* Get the packet format */
    bzero(company_name,COMPANY_NAME_LENGTH);
    strcpy(company_name, global_par.company_par_array[com_id].company_name);
    field_sum = global_par.company_par_array[com_id].pkt_par_array[pkt_id][FORWARD_POSITION].item_count;
    p = global_par.company_par_array[com_id].pkt_par_array[pkt_id][FORWARD_POSITION].item_par_array;

    /* Exact the serial number value from the company packet */
    for (i=0; i<field_sum; i++,p++) {
        if (0==strcmp("SERIAL_NUMBER",p->name)) {
            if (0==p->direction) {
                memcpy(tmp_serial_number,company_pkt+(p->start_pos),p->len);
                continue;
            }
            if (1==p->direction) {
                memcpy(tmp_serial_number,company_pkt+(packet_size-1-p->start_pos-PACKET_HEADER_LENGTH),p->len);
                continue;
            }
        }
    }

    DBG("\n%s %s, |%s|", "tmp_serial_number is : ",tmp_serial_number, packet );

    /* Query the valid of the packet */

    /* SERIAL_NUMBER :serial number, phone number, terminal id, date */
    valid_val = 0;
    bzero(query_string,TEMP_LENGTH);
    sprintf(query_string,
            "SELECT SERIAL_NUMBER FROM %s WHERE SERIAL_NUMBER = \'%s\' AND PHONE_NUMBER = \'%s\' AND TERMINAL_ID = \'%s\' AND RECORD_DATE=CURRENT_DATE AND MONEY=\'%s\';",
            company_name,
            tmp_serial_number,
            common_pkt_header.phone_number,
            common_pkt_header.terminal_id,
            common_pkt_header.money);
    DBG("\nCHECK FORWARD REVERSAL, SQL: |%s|\n", query_string);

    /* Query SQL command to the database */
    res = PQexec(conn_db, query_string);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        OUTPUT_ERROR;
        perror(query_string);
        perror(PQerrorMessage(conn_db));

        LOG(INFO)<<query_string;
        LOG(INFO)<<PQerrorMessage(conn_db);

        PQclear(res);
        res = NULL;
        success = Change_packet_response_code(packet,packet_size,DB_ERROR_POSTGRESQL);
        return -1;
    } else {
        valid_val = PQntuples(res);
        PQclear(res);
        res = NULL;
    }
    /* Number 1 describes only one charging behaviour */
    if (0==valid_val) {
        fflush(NULL);
        printf("\033[031mERROR:There is no such serial number in Database!\033[031m\n");
        success = Change_packet_response_code(packet,packet_size,DB_ERROR_NO_RECORDS);
        fflush(NULL);
        return -1;
    }
    if (2<=valid_val) {
        fflush(NULL);
        printf("\033[031mERROR:The reversal trade had be completed before!\033[031m\n");
        success = Change_packet_response_code(packet,packet_size,DB_ERROR_TWO_RECORDS);
        fflush(NULL);
        return -1;
    }

    /* Terminal_ID : Trade code, Terminal ID, date_oneday*/
    valid_val = 0;
    bzero(query_string,TEMP_LENGTH);

    /* From terminal management table */
    sprintf(query_string,"SELECT reversal_limit FROM Terminal_manage WHERE TERMINAL_ID = \'%s\';",common_pkt_header.terminal_id);
    DBG("CHECK FORWARD REVERSAL, SQL: |%s|\n", query_string);

    /* Query SQL command to the database */
    res = PQexec(conn_db, query_string);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        perror(PQerrorMessage(conn_db));
        PQclear(res);
        res = NULL;
        success = Change_packet_response_code(packet,packet_size,DB_ERROR_POSTGRESQL);
        return -1;
    }
    valid_val = (int) strtol(PQgetvalue(res,0,0),&e,10);
    PQclear(res);
    res = NULL;

    if (valid_val<1) {
        fflush(NULL);
        printf("\033[031mERROR: The reversal number of the terminal exceedsTERMIAL_UP_LIMIT this day!\033[031m\n");
        fflush(NULL);
        success = Change_packet_response_code(packet,packet_size,DB_ERROR_TERIMAL_THAN_THIRTY);
        return -1;
    } else {
        bzero(query_string,TEMP_LENGTH);

        /* From terminal management table */
        sprintf(query_string,"UPDATE Terminal_Manage Set reversal_limit=reversal_limit-1 WHERE TERMINAL_ID = \'%s\';",
                common_pkt_header.terminal_id);
        DBG("CHECK FORWARD REVERSAL, SQL: |%s|\n", query_string);

        /* Query SQL command to the database */
        res = PQexec(conn_db, query_string);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            perror(PQerrorMessage(conn_db));
            PQclear(res);
            res = NULL;
            success = Change_packet_response_code(packet,packet_size,DB_ERROR_POSTGRESQL);
            return -1;
        }
        PQclear(res);
        res = NULL;
    }

    /* verify the reversal limit of the company */
    valid_val = 0;
    bzero(query_string,TEMP_LENGTH);

    /* from terminal management table */
    sprintf(query_string,"SELECT %s_reversal_limit FROM Terminal_manage WHERE TERMINAL_ID = \'%s\';",
            company_name,
            common_pkt_header.terminal_id);
    DBG("CHECK FORWARD REVERSAL, SQL: |%s|\n", query_string);

    /* query the SQL command to  the database */
    res = PQexec(conn_db, query_string);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        perror(PQerrorMessage(conn_db));
        PQclear(res);
        res = NULL;
        success = Change_packet_response_code(packet,packet_size,DB_ERROR_POSTGRESQL);
        return -1;
    }
    valid_val = (int) strtol(PQgetvalue(res,0,0),&e,10);
    PQclear(res);
    res = NULL;
    DBG("REVERSAL LIMIT OF THE COMMANY: |%d|\n", valid_val);

    if (valid_val<1) {
        success = Change_packet_response_code(packet,packet_size,DB_ERROR_TERIMAL_THAN_THIRTY);
        return -1;
    } else {
        bzero(query_string,TEMP_LENGTH);

        /* From terminal management table */
        sprintf(query_string,"UPDATE Terminal_Manage Set %s_reversal_limit=%s_reversal_limit-1 WHERE TERMINAL_ID = \'%s\';" ,
                company_name,
                company_name,
                common_pkt_header.terminal_id);
        DBG("CHECK FORWARD REVERSAL, SQL: |%s|\n", query_string);

        /* Send the query to database */
        res = PQexec(conn_db, query_string);
        /* Did the record action fail in the primary database? */
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            perror(PQerrorMessage(conn_db));
            PQclear(res);
            res = NULL;
            success = Change_packet_response_code(packet,packet_size,DB_ERROR_POSTGRESQL);
            return -1;
        } else {
            PQclear(res);
            res = NULL;
            return 1;
        }
    }
}

/*!!
 *****************************************************************************
 *
 * \brief
 *    Check_backward_reversal_packet_valid: check the reversal packet. The rules is:
 *    	1. There is one charge record in the database.
 *    	2. The charge packet must be recorded today.
 *    	3. One phone number reversal times < 3 in this month.
 *    	4. One client or terminal ,reversal times < 30 today.
 *
 * \par Input:
 *    conn_db: the handle of the connection with the database
 *    packet: the pointer of the packet.
 *    packet_size: the length of the packet.
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
int Check_backward_reversal_packet_valid(PGconn* conn_db,char *packet, int packet_size)
{
    PGresult *res = NULL;

    int i = 0;
    int success = 0;
    int com_id = 0;
    int pkt_id = 0;
    int field_sum = 0;
    int valid_val = 0;
    char *e = NULL;
    char *company_pkt = NULL;
    item_parameters *p = NULL;

    char company_name[COMPANY_NAME_LENGTH];
    char query_string[MAX_QUERY_LENGTH];
    char tmp_phone_number[TEMP_LENGTH];
    char tmp_serial_number[TEMP_LENGTH];
    char field_names[TEMP_LENGTH];
    char field_values[TEMP_LENGTH];

    char * trade_code = NULL;
    int service_type_item_index = 0;
    int im_level = 0;

    struct CommonPacketHeader *common_pkt_header = NULL;

    common_pkt_header = (struct CommonPacketHeader *)malloc(sizeof(struct CommonPacketHeader));
    if (NULL == common_pkt_header) {
        perror("business_data_analysis.c:Generate_company_record():common_pkt_header malloc failure");
        return 0;
    }

    bzero(field_names,TEMP_LENGTH);
    bzero(field_values,TEMP_LENGTH);
    bzero(tmp_serial_number,TEMP_LENGTH);
    bzero(tmp_phone_number,TEMP_LENGTH);
    bzero(query_string,MAX_QUERY_LENGTH);

    bzero(common_pkt_header,sizeof(struct CommonPacketHeader));


    /* Get the packet header */
    success = Get_common_header(packet, common_pkt_header);

    /* Get the company id */
    com_id = strtol(common_pkt_header->company_id,&e,10);

    /*Get the service type*/
    pkt_id = strtol(common_pkt_header->service_id,&e,10);

    /* Get the important level value */

    im_level = global_par.company_par_array[com_id].packet_important_level[pkt_id];

    /* If the packet is not reversal one */
    if (1!=im_level)
        return 1;
    /* These codes in the following point to the reveral packet */

    /* Get the trade code value */
    service_type_item_index = global_par.company_par_array[com_id].pkt_par_array[pkt_id][BACKWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
    trade_code = global_par.company_par_array[com_id].pkt_par_array[pkt_id][BACKWARD_POSITION].item_par_array[service_type_item_index].valid_value;

    company_pkt = packet + PACKET_HEADER_LENGTH ; /* Cut packet header */

    /* Get the packet format */
    bzero(company_name,COMPANY_NAME_LENGTH);
    strcpy(company_name, global_par.company_par_array[com_id].company_name);
    field_sum = global_par.company_par_array[com_id].pkt_par_array[pkt_id][BACKWARD_POSITION].item_count;
    p = global_par.company_par_array[com_id].pkt_par_array[pkt_id][BACKWARD_POSITION].item_par_array;

    /* Get the service number */
    /* Exact the serial number value from the company packet */
    for (i=0; i<field_sum; i++,p++) {
        if (0==strcmp("SERIAL_NUMBER",p->name)) {

            if (1==p->db_record&&0==p->direction) {
                memcpy(tmp_serial_number,company_pkt+(p->start_pos),p->len);
                continue;
            }
            if (1==p->db_record&&1==p->direction) {
                memcpy(tmp_serial_number,company_pkt+(packet_size-1-p->start_pos),p->len);
                continue;
            }
        }
    }

    /* Query the valid of the packet */

    /* SERIAL_NUMBER :serial number, phone number, terminal id, date */
    valid_val = 0;
    bzero(query_string,TEMP_LENGTH);
    sprintf(query_string,"SELECT * FROM %s WHERE SERIAL_NUMBER = \'%s\' AND PHONE_NUMBER = \'%s\' AND TERMINAL_ID = \'%s\' AND RECORD_DATE=CURRENT_DATE;",company_name,tmp_serial_number,common_pkt_header->phone_number,common_pkt_header->terminal_id);

    /* Send the query to  database */
    res = PQexec(conn_db, query_string);
    /* Did the record action fail in the primary database? */
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        perror("error:business_data_analysis.cc:Check_packet_valid_valid()01.");
        printf("%s\n",PQerrorMessage(conn_db));
        PQclear(res);
        res = NULL;
        //success = Set_ownself_server_mode(ERROR);
        success = Change_packet_response_code(packet,packet_size,DB_ERROR_POSTGRESQL);
        return -1;
    }
    valid_val = PQntuples(res);
    PQclear(res);
    res = NULL;

    /* Number 1 describes only one charging behaviour */
    if (1!=valid_val) {
        fflush(NULL);
        printf("\033[031mERROR:The record number in the database is not equals to one!\033[031m\n");
        success = Change_packet_response_code(packet,packet_size,DB_ERROR_NO_RECORDS);
        fflush(NULL);
        return -1;
    }

    /* PHONE_NUMBER : trade code, phone number, date_one month*/
    valid_val = 0;
    bzero(query_string,TEMP_LENGTH);
    sprintf(query_string,"SELECT * FROM %s WHERE TRADE_CODE = \'%s\' AND PHONE_NUMBER = \'%s\' AND RECORD_DATE>= (current_date - CAST(to_char(extract(day from CURRENT_DATE)+1.0,'99') as integer));",company_name,trade_code,common_pkt_header->phone_number);

    PQclear(res);
    res = NULL;
    /* Send the query to  database */
    res = PQexec(conn_db, query_string);
    /* Did the record action fail in the primary database? */
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        perror("error:business_data_analysis.cc:Check_packet_valid_valid()02-2.");
        printf("%s\n",PQerrorMessage(conn_db));
        PQclear(res);
        res = NULL;
        //success = Set_ownself_server_mode(ERROR);
        success = Change_packet_response_code(packet,packet_size,DB_ERROR_POSTGRESQL);
        return -1;
    }
    valid_val = PQntuples(res);
    PQclear(res);
    res = NULL;
    /* Number 3 describes one phone is carry out reversal three times */

    if (valid_val>=NUMBER_UP_LIMIT) {
        fflush(NULL);
        printf("\033[031mERROR:The reversal number of the phone exceeds NUMBER_UP_LIMIT this month!\033[031m\n");
        fflush(NULL);
        success = Change_packet_response_code(packet,packet_size,DB_ERROR_NUMBER_THAN_THREE);
        return -1;
    }


    /* CLIENT_ID : Trade code, Terminal ID, date_oneday*/
    valid_val = 0;
    for (i=0; i<global_par.company_num; i++) {
        bzero(query_string,TEMP_LENGTH);
        sprintf(query_string,"SELECT * FROM %s WHERE TRADE_CODE = \'%s\' AND TERMINAL_ID = \'%s\' AND RECORD_DATE=CURRENT_DATE;",company_name,trade_code,common_pkt_header->terminal_id);
        /* Send the query to  database */
        res = PQexec(conn_db, query_string);
        /* Did the record action fail in the primary database? */
        if (PQresultStatus(res) != PGRES_TUPLES_OK) {
            perror("error:business_data_analysis.cc:Check_packet_valid_valid()03.");
            printf("%s\n",PQerrorMessage(conn_db));
            PQclear(res);
            res = NULL;
            success = Change_packet_response_code(packet,packet_size,DB_ERROR_POSTGRESQL);
            //success = Set_ownself_server_mode(ERROR);
            return -1;
        }
        valid_val = PQntuples(res) + valid_val;
        PQclear(res);
        res = NULL;
    }

    if (valid_val>=TERMIAL_UP_LIMIT) {
        fflush(NULL);
        printf("\033[031mERROR: The reversal number of the terminal exceedsTERMIAL_UP_LIMIT this day!\033[031m\n");
        fflush(NULL);
        success = Change_packet_response_code(packet,packet_size,DB_ERROR_TERIMAL_THAN_THIRTY);
        return -1;
    } else
        return 1;
}


/* *************************************
 *	Function Name:
 *		int Change_packet_response_code(char *common_packet_header, int packet_size,int respone_code)
 *	Input:
 *		char *packet ---> packet
 *		int packet_size ---> packet_length;
 *	Ouput:
 *		1 ---> valid
 *		0 ---> invalid
 * *************************************************/
int Change_packet_response_code(char *common_packet_header, int packet_size,int response_code)
{
    int success = 0;
    if (NULL == common_packet_header) {
        perror("business_data_analysis.c:Change_packet_response_code");
        return 0;
    }

    if (response_code!=0)
        success = Set_common_header_inner_success_id(common_packet_header,FAILURE_FLAG);
    else
        success = Set_common_header_inner_success_id(common_packet_header,SUCCESS_FLAG);

    switch (response_code) {
    case DB_ERROR_RECORDSQL:
        success = Set_common_header_error_memo(common_packet_header, "交费操作未记录，交易失败");
        break;
    case DB_ERROR_POSTGRESQL:
        success = Set_common_header_error_memo(common_packet_header, "数据记录故障，交易失败。");
        break;
    case DB_ERROR_NO_RECORDS:
        success = Set_common_header_error_memo(common_packet_header, "数据库中无原始缴费记录。");
        break;
    case DB_ERROR_TWO_RECORDS:
        success = Set_common_header_error_memo(common_packet_header, "数据库已经完成返销操作。");
        break;
    case DB_ERROR_NUMBER_THAN_THREE:
        success = Set_common_header_error_memo(common_packet_header, "号码本月反销超上限3次。");
        break;
    case DB_ERROR_TERIMAL_THAN_THIRTY:
        success = Set_common_header_error_memo(common_packet_header, "终端今日反销超上限30次。");
        break;
    case DB_ERROR_FIXING:
        success = Set_common_header_error_memo(common_packet_header, "出现错误。系统正在维护。");
        break;
    case DB_ERROR_MAINTAIN:
        success = Set_common_header_error_memo(common_packet_header, "系统维护中，禁止交易。");
        break;
    case DB_ERROR_ONLY_SELECT:
        success = Set_common_header_error_memo(common_packet_header, "数据库只能执行查询语句");
        break;
    default:
        success = Set_common_header_error_memo(common_packet_header, "出现错误。目前正在排除。");
        break;
    }
    return 1;
}


/*
*/
int Get_import_level(char *packet)
{
    int success = 0;
    int com_id = 0;
    int pkt_id = 0;
    int im_level = 0;
    char *e = NULL;
    struct CommonPacketHeader *common_pkt_header = NULL;

    common_pkt_header =(struct CommonPacketHeader *) malloc(sizeof(struct CommonPacketHeader));
    if (NULL == common_pkt_header) {
        perror("business_data_analysis.c:Get_import_level(char *packet)():common_pkt_header malloc failure");
        return 0;
    }

    bzero(common_pkt_header,sizeof(struct CommonPacketHeader));

    /* Get the packet header */
    success = Get_common_header(packet, common_pkt_header);

    /* Get the company id */
    com_id = strtol(common_pkt_header->company_id,&e,10);

    /*Get the service type*/
    pkt_id = strtol(common_pkt_header->service_id,&e,10);

    /* Get the important level value */

    im_level = global_par.company_par_array[com_id].packet_important_level[pkt_id];
    free(common_pkt_header);
    common_pkt_header = NULL;
    return im_level;
}
/*!
 *****************************************************************************
 *
 * \brief
 *    Get_compound_pkt_info(): according to the packet from business servers,
 *    it extract all information about the compound packet, the pkt number,
 *	 forward pkt length, backward pkt length.
 *
 * \par Input:
 *    packet: the pointer of buffer contained the packet to be parsed.
 *    packet_size: the length of the packet.
 *		query_string_size: the maximum length of the query_string space
 * \par Output:
 *    CompoundPacketInfo: the pointer of the space saving CompoundPacketInfo.
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
 **************************************************************************** */
int Get_compound_pkt_info(char *pkt, int pkt_size, struct CompoundPacketInfo *info)
{
    char pkt_number[INFO_NUMBER_LENGTH+1];
    char forward_pkt_len[INFO_FORWARD_PKT_LENGTH+1];
    char backward_pkt_len[INFO_BACKWARD_PKT_LENGTH+1];
    char *end = NULL;

    /* Check input */
    if (NULL==pkt||NULL==info)
        return 0;

    bzero(pkt_number,INFO_NUMBER_LENGTH+1);
    bzero(forward_pkt_len,INFO_FORWARD_PKT_LENGTH+1);
    bzero(backward_pkt_len,INFO_BACKWARD_PKT_LENGTH+1);

    /* Extract info from the packet */
    memcpy(pkt_number,pkt+ERROR_MEMO_POSITION,INFO_NUMBER_LENGTH);
    memcpy(forward_pkt_len,pkt+ERROR_MEMO_POSITION+INFO_NUMBER_LENGTH, INFO_FORWARD_PKT_LENGTH);
    memcpy(backward_pkt_len,pkt+ERROR_MEMO_POSITION+INFO_NUMBER_LENGTH+INFO_FORWARD_PKT_LENGTH, INFO_BACKWARD_PKT_LENGTH);

    /* Translation to number */
    info->pkt_number = strtoul(pkt_number,&end,10);
    info->forward_pkt_len = strtoul(forward_pkt_len,&end,10);
    info->backward_pkt_len = strtoul(backward_pkt_len,&end,10);


    return 1;
}

/* **************************************
* ***************************************/
int Get_inner_pkt_flag(char *packet)
{
    char inner_flag[INNER_FLAG_LENGTH+1];
    long int inner_flag_value = 0;
    char *e = NULL;
    bzero(inner_flag,INNER_FLAG_LENGTH+1);

    memcpy(inner_flag,packet+INNER_FLAG_POSITION,INNER_FLAG_LENGTH);
    //printf("INNER_FLAG= %s\n",inner_flag);
    inner_flag_value = strtol(inner_flag,&e,10);

    return (int)inner_flag_value;

}

/* **************************************
* ***************************************/
int Set_inner_pkt_flag(char *packet,char *flag)
{
    memcpy(packet+INNER_FLAG_POSITION,flag,INNER_FLAG_LENGTH);
    return 1;

}

/* *************************************
 *	Function Name:
 *		int Find_clientid_according_serialnumber(PGconn* conn_db, char* packet, int packet_len)
 *	Input:
 *      PGconn* conn_db ---> the connection handle of database
 *		char *packet ---> packet
 *		int packet_size ---> packet_length;
 *	Ouput:
 *		1 ---> valid
 *		-1 ---> invalid
 * *************************************************/

int Find_clientid_according_serialnumber(PGconn* conn_db, char* packet, int packet_size)
{
    PGresult *res = NULL;

    int i = 0;
    int success = 0;
    int com_id = 0;
    int pkt_id = 0;
    int field_sum = 0;
    int valid_val = 0;
    char *e = NULL;
    char *company_pkt = NULL;
    item_parameters *p = NULL;

    char company_name[COMPANY_NAME_LENGTH];
    char query_string[MAX_QUERY_LENGTH];
    char tmp_phone_number[TEMP_LENGTH];
    char tmp_serial_number[TEMP_LENGTH];
    char field_names[TEMP_LENGTH];
    char field_values[TEMP_LENGTH];

    char * trade_code = NULL;
    int service_type_item_index = 0;
    int im_level = 0;

    struct CommonPacketHeader *common_pkt_header = NULL;

    common_pkt_header = (struct CommonPacketHeader *)malloc(sizeof(struct CommonPacketHeader));
    if (NULL == common_pkt_header) {
        perror("business_data_analysis.c:Generate_company_record():common_pkt_header malloc failure");
        return 0;
    }

    bzero(field_names,TEMP_LENGTH);
    bzero(field_values,TEMP_LENGTH);
    bzero(tmp_serial_number,TEMP_LENGTH);
    bzero(tmp_phone_number,TEMP_LENGTH);
    bzero(query_string,MAX_QUERY_LENGTH);

    bzero(common_pkt_header,sizeof(struct CommonPacketHeader));


    /* Get the packet header */
    success = Get_common_header(packet, common_pkt_header);

    /* Get the company id */
    com_id = strtol(common_pkt_header->company_id,&e,10);

    /*Get the service type*/
    pkt_id = strtol(common_pkt_header->service_id,&e,10);

    /* Get the important level value */

    im_level = global_par.company_par_array[com_id].packet_important_level[pkt_id];

    if (0 != strcmp(common_pkt_header->terminal_id, "XXXXXXXX")) {
        /*the common header has client_id, need not the find client id from database, return 1*/
        \
        return 1;
    }

    /*find serial number from packet*/

    /* These codes in the following point to the reveral packet */

    /* Get the trade code value from the forward packet*/
    service_type_item_index = global_par.company_par_array[com_id].pkt_par_array[pkt_id][FORWARD_POSITION].item_index[SERVICE_TYPE_ITEM_INDEX];
    trade_code = global_par.company_par_array[com_id].pkt_par_array[pkt_id][FORWARD_POSITION].item_par_array[service_type_item_index].valid_value;

    company_pkt = packet + PACKET_HEADER_LENGTH ; /* Cut packet header */
    printf("COMPANY PKT :|%s|\n",company_pkt);

    /* Get the packet format */
    bzero(company_name,COMPANY_NAME_LENGTH);
    strcpy(company_name, global_par.company_par_array[com_id].company_name);
    field_sum = global_par.company_par_array[com_id].pkt_par_array[pkt_id][FORWARD_POSITION].item_count;
    p = global_par.company_par_array[com_id].pkt_par_array[pkt_id][FORWARD_POSITION].item_par_array;
    /* Get the service number */
    /* Exact the serial number value from the company packet */
    printf("ITEM count :|%d|\n",field_sum);
    for (i=0; i<field_sum; i++,p++) {
        printf("P name:%s\n",p->name);
        if (0==strcmp("SERIAL_NUMBER",p->name)) {

            if (0==p->direction) {
                memcpy(tmp_serial_number,company_pkt+(p->start_pos),p->len);
                continue;
            }
            if (1==p->direction) {
                memcpy(tmp_serial_number,company_pkt+(packet_size-1-p->start_pos),p->len);
                continue;
            }
            //printf("INFO:%s -- %d,%s",company_pkt+(p->start_pos),p->len,tmp_serial_number);
        }
    }


    /* 1:SERIAL_NUMBER :serial number, phone number, terminal id, date */
    valid_val = 0;
    bzero(query_string,TEMP_LENGTH);
    sprintf(query_string,"SELECT TERMINAL_ID FROM %s WHERE SERIAL_NUMBER = \'%s\' AND RECORD_DATE=CURRENT_DATE;",company_name,tmp_serial_number);

    printf("The query:|%s|\n",query_string);
    /* Send the query to  database */
    res = PQexec(conn_db, query_string);
    /* Did the record action fail in the primary database? */
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        perror("error:business_data_analysis.cc:Check_packet_valid_valid()01.");
        printf("%s\n",PQerrorMessage(conn_db));
        PQclear(res);
        res = NULL;
        return -1;
    }

    valid_val = PQntuples(res);
    printf("VALID NUMBER :%d\n",valid_val);

    if (1 != valid_val) {
        fflush(NULL);
        printf("\033[031mERROR:There has no coresponding charge record or has more than one record!\033[031m\n");
        fflush(NULL);
        PQclear(res);
        res = NULL;
        return -1;
    } else {
        memcpy(packet+TERMINAL_ID_POSITION, PQgetvalue(res, 0, 0), TERMINAL_ID_LENGTH);
        printf("INner find PKT:|%s|\n",packet);
        PQclear(res);
        res = NULL;
        return 1;
    }

}
/* *********************************
************************************/
int Get_curr_time(char *curr_time)
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p=localtime(&timep); /*取得当地时间*/
    sprintf(curr_time,"%02d:%02d", p->tm_hour, p->tm_min);
    return 1;
}

int transcode_from_gb2312_to_utf8(char *bufin, char *bufout)
{
    char encFrom[]="gbk";
    char encTo[]="utf-8";
    char *sin, *sout;
    int lenin, lenout;
    iconv_t c_pt;

    if ((c_pt = iconv_open(encTo, encFrom)) == (iconv_t)-1) {
        printf("iconv_open false: %s ==> %s\n", encFrom, encTo);
        return -1;
    }
    iconv(c_pt, NULL, NULL, NULL, NULL);
    lenin  = strlen(bufin) + 1;
    lenout = lenin*2;
    sin    = bufin;
    sout   = bufout;
    iconv(c_pt, &sin, (size_t*)&lenin, &sout, (size_t*)&lenout);
    iconv_close(c_pt);
    return 1;
};

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  Fix_invalid_string(char *str, char *fixed_str)
 *  Description:
 * =====================================================================================
 */
int  Fix_invalid_string(unsigned char *str, int str_len, unsigned char *fixed_str)
{
    int i = 0;
    int j = 0;
    while (i<str_len) {
        if ( 0==(*(str+i)&0x80) ) {
            *(fixed_str+j) = *(str+i);
            i++;
            j++;
        } else {
            if (0==(*(str+i+1)&0x80) ) {
                i++;
            } else {
                *(fixed_str+j) = *(str+i);
                i++;
                j++;
                *(fixed_str+j) = *(str+i);
                i++;
                j++;
            }
        }
    }
    return 1;
}

int  Fix_invalid_string_with_space(unsigned char *str, int str_len)
{
    int i = 0;
    while (i<str_len) {
        if ( 0==(*(str+i)&0x80) ) {
            if ( 0==(*(str+i)^0x27) )
                *(str+i) = 0x20;
        } else if (0==(*(str+i+1)&0x80) ) {
            *(str+i) = 0x20;
            if (0==(*(str+i+1)^0x27) )
                *(str+i+1) = 0x20;
            i++;
        } else {
            i++;
        }
        i++;
    }
    return 1;
}

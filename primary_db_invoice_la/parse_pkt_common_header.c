/* *************************************************
 * filename:
 *		parse_pkt_common_header.c
 * author:
 * 		Zhiwei Yan, jerod.yan@gmail.com
 * date:
 * 		2007-04-03
 * *************************************************/
#include "parse_pkt_common_header.h"
/* *************************************************
 * Function Name:
 * 		int Get_common_header(char *packet, struct *common_pkt_header)
 * Input:
 * 		the pointer of common packet
 * Output:
 * 		struct *common_pkt_header
 * 		1 success; -1 failure
 * *************************************************/
int Get_common_header(char *packet, struct CommonPacketHeader *common_pkt_header)
{
    int	success = 0;
    /* check the input value */
    if (NULL==packet||NULL==common_pkt_header) {
        perror("error:parse_pkt_common_header.c:Get_common_header_company_id():01");
        return -1;
    }
#if _DEBUG_ >0
    printf("RECV:%s\n",packet);
#endif

    success = Get_common_header_company_id(packet,common_pkt_header->company_id);
    success = Get_common_header_service_id(packet,common_pkt_header->service_id);
    success = Get_common_header_inner_flag(packet,common_pkt_header->inner_flag);
    success = Get_common_header_terminal_id(packet,common_pkt_header->terminal_id);
    success = Get_common_header_worker_id(packet,common_pkt_header->worker_id);
    success = Get_common_header_contract_id(packet,common_pkt_header->contract_id);
    success = Get_common_header_phone_number(packet,common_pkt_header->phone_number);
    success = Get_common_header_money(packet,common_pkt_header->money);

    return 1;

}
/* *************************************************
 * Function Name:
 * 		int Get_common_header_company_id(char *packet, char *company_id)
 * Input:
 * 		the pointer of common packet
 * Output:
 * 		the pointer of company_id
 * 		1 success; -1 failure
 * *************************************************/
int Get_common_header_company_id(char *packet, char *company_id)
{
    /* check the input value */
    if (NULL==packet||NULL==company_id) {
        perror("error:parse_pkt_common_header.c:Get_common_header_company_id():01");
        return -1;
    }

    /* extract company id from packet */
    memcpy(company_id,packet+COMPANY_ID_POSITION,COMPANY_ID_LENGTH);

    return 1;
}

/* *************************************************
 * Function Name:
 * 		int Get_common_header_service_id(char *packet, char *service_id)
 * Input:
 * 		the pointer of common packet
 * Output:
 * 		the pointer of service_id
 * 		1 success; -1 failure
 * *************************************************/
int Get_common_header_service_id(char *packet, char *service_id)
{
    /* check the input value */
    if (NULL==packet||NULL==service_id) {
        perror("error:parse_pkt_common_header.c:Get_common_header_service_id():01");
        return -1;
    }

    /* extract company id from packet */
    memcpy(service_id,packet+SERVICE_ID_POSITION,SERVICE_ID_LENGTH);

    return 1;
}
/* *************************************************
 * Function Name:
 * 		int Get_common_header_inner_flag(char *packet,char *inner_flag)
 * Input:
 * 		the pointer of common packet
 * Output:
 * 		the pointer of inner_id
 * 		1 success; -1 failure
 * *************************************************/
int Get_common_header_inner_flag(char *packet,char *inner_flag)
{
    /* check the input value */
    if (NULL==packet||NULL==inner_flag) {
        perror("error:parse_pkt_common_header.c:Get_common_header_service_id():01");
        return -1;
    }

    /* extract company id from packet */
    memcpy(inner_flag,packet+INNER_FLAG_POSITION,INNER_FLAG_LENGTH);

    return 1;
}

/* *************************************************
 * Function Name:
 * 		int Get_common_header_terminal_id(char *packet,char *terminal_id)
 * Input:
 * 		the pointer of common packet
 * Output:
 * 		the pointer of terminal_id
 * 		1 success; -1 failure
 * *************************************************/
int Get_common_header_terminal_id(char *packet,char *terminal_id)
{
    /* check the input value */
    if (NULL==packet||NULL==terminal_id) {
        perror("error:parse_pkt_common_header.c:Get_common_header_terminal_id():01");
        return -1;
    }

    /* extract company id from packet */
    memcpy(terminal_id,packet+TERMINAL_ID_POSITION,TERMINAL_ID_LENGTH);

    return 1;
}
/* *************************************************
 * Function Name:
 * 		int Get_common_header_worker_id(char *packet char* worker_id)
 * Input:
 * 		the pointer of common packet
 * Output:
 * 		the pointer of worker id
 * 		1 success; -1 failure
 * *************************************************/
int Get_common_header_worker_id(char *packet, char* worker_id)
{
    /* check the input value */
    if (NULL==packet||NULL==worker_id) {
        perror("error:parse_pkt_common_header.c:Get_common_header_worker_id():01");
        return -1;
    }

    /* extract company id from packet */
    memcpy(worker_id,packet+WORKER_ID_POSITION,WORKER_ID_LENGTH);

    return 1;
}
/* *************************************************
 * Function Name:
 * 		int Get_common_header_contract_id(char *packet,char* contract_id)
 * Input:
 * 		the pointer of common packet
 * Output:
 * 		the pointer of contract_id
 * 		1 success; -1 failure
 * *************************************************/
int Get_common_header_contract_id(char *packet,char* contract_id)
{
    /* check the input value */
    if (NULL==packet||NULL==contract_id) {
        perror("error:parse_pkt_common_header.c:Get_common_header_contract_id():01");
        return -1;
    }

    /* extract company id from packet */
    memcpy(contract_id,packet+CONTRACT_ID_POSITION,CONTRACT_ID_LENGTH);

    return 1;
}
/* *************************************************
 * Function Name:
 * 		int Get_common_header_phone_number(char *packet,char *phone_number)
 * Input:
 * 		the pointer of common packet
 * Output:
 * 		the pointer of phone number
 * 		1 success; -1 failure
 * *************************************************/
int Get_common_header_phone_number(char *packet,char *phone_number)
{
    /* check the input value */
    if (NULL==packet||NULL==phone_number) {
        perror("error:parse_pkt_common_header.c:Get_common_header_contract_id():01");
        return -1;
    }

    /* extract company id from packet */
    memcpy(phone_number,packet+PHONE_NUMBER_POSITION,PHONE_NUMBER_LENGTH);

    return 1;
}
/* *************************************************
 * Function Name:
 * 		int Get_common_header_money(char *packet,char *money)
 * Input:
 * 		the pointer of common packet
 * Output:
 * 		the money of this transaction
 * 		1 success; -1 failure
 * *************************************************/
int Get_common_header_money(char *packet,char *money)
{
    /* check the input value */
    if (NULL==packet||NULL==money) {
        perror("error:parse_pkt_common_header.c:Get_common_header_money():01");
        return -1;
    }

    /* extract company id from packet */
    memcpy(money,packet+MONEY_POSITION,MONEY_LENGTH);

    return 1;
}
/* *************************************************
 * Function Name:
 * 		int Set_common_header_inner_success_id(char *packet,char *inner_success_id)
 * Input:
 * 		the pointer of common packet
 * Output:
 * 		the pointer of inner_success_id
 * 		1 success; -1 failure
 * *************************************************/
int Set_common_header_inner_success_id(char *packet,const char *inner_success_id)
{
    /* check the input value */
    if (NULL==packet||NULL==inner_success_id ) {
        perror("error:parse_pkt_common_header.c:Get_common_header_contract_id():01");
        return -1;
    }

    /* set inner success flag in the packet */
    memcpy(packet+INNER_SUCCESS_FLAG_POSITION,inner_success_id,INNER_SUCCESS_FLAG_LENGTH);

    return 1;
}
/* *************************************************
 * Function Name:
 * 		int Set_common_header_error_memo(char *packet, char *error_memo)
 * Input:
 * 		the pointer of common packet
 * Output:
 * 		the pointer of error_memo
 * 		1 success; -1 failure
 * *************************************************/
int Set_common_header_error_memo(char *packet, const char *error_memo)
{
    /* check the input value */
    if (NULL==packet||NULL==error_memo) {
        perror("error:parse_pkt_common_header.c:Set_common_header_error_memo():01");
        return -1;
    }

    /* extract company id from packet */
    bzero(packet+ERROR_MEMO_POSITION,ERROR_MEMO_LENGTH);
    memcpy(packet+ERROR_MEMO_POSITION,error_memo,ERROR_MEMO_LENGTH);

    return 1;
}


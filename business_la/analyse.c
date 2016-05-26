/*********************************************************
 *project: Line communication charges supermarket
 *filename: analyse.c
 *version: 0.4
 *purpose: functions about parse the affair packets
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-13
 *********************************************************/
 
#include "analyse.h"

/*************************************************************************
 *  \brief
 *    Acquire the enterprise code (ChinaUnicom or ChianMobile). 
 *
 *  \par Input:
 *     buf_pkt: the buffer pointer to the packet which is to be parsed.
 *     len_pkt: the length of this packet.
 *  \par Output:
 *  \Return:
 *     >=0: The server code defined in "common.h"
 *    -1: the function has some error
************************************************************************/
int GetEnterpriseCodeFromHeader(char *buf_pkt, int len_pkt)
{
	int company_index = 5000;
	char str_com_idx[COMPANY_INDEX_LENGTH_AT_HEADER+1];
	char *e;
	/*get the string of company_index*/
	bzero(str_com_idx, COMPANY_INDEX_LENGTH_AT_HEADER+1);
	memcpy(str_com_idx, buf_pkt+COMPANY_INDEX_START_POSITION_AT_HEADER, COMPANY_INDEX_LENGTH_AT_HEADER);
	/*transform to integer*/
	company_index = strtol(str_com_idx, &e, 10);
	/*verify the company index*/
	if(global_par.company_num <= company_index)
	{
		perror("error@analyse.c:GetEnterpriseCode:no such company!!!\n");
		return -1;
	}
	return company_index;
}

/*************************************************************************
 *  \brief
 *    Acquire the Charge money (unit is fen). 
 *
 *  \par Input:
 *     buf_pkt: the buffer pointer to the packet which is to be parsed.
 *     len_pkt: the length of this packet.
 *  \par Output:
 *  \Return:
 *     >=0: The server code defined in "common.h"
 *    -1: the function has some error
************************************************************************/
unsigned long int GetChargeMoneyFromHeader(char *buf_pkt, int len_pkt)
{
	long int charge_money = 0;
	char str_money[CHARGE_MONEY_LENGTH_AT_HEADER+1];
	char *e;
	/*get the string of charge_momney*/
	bzero(str_money, CHARGE_MONEY_LENGTH_AT_HEADER+1);
	memcpy(str_money, buf_pkt+CHARGE_MONEY_START_POSITION_AT_HEADER, CHARGE_MONEY_LENGTH_AT_HEADER);
	/*transform to integer*/
	charge_money = strtol(str_money, &e, 10);
	if(0 > charge_money)
	{
		perror("error@analyse.c:GetChargeMoneyFromHeader:charge money < 0!!!\n");
		return -1;
	}
	return charge_money;
}

/*************************************************************************
 *  \brief
 *    Acquire the service type code (Charge, Query or Reversal). 
 *
 *  \par Input:
 *     buf_pkt: the buffer pointer to the packet which is to be parsed.
 *     len_pkt: the length of this packet.
 *  \par Output:
 *  \Return:
 *     >=0: The server code defined in "common.h"
 *    -1: the function has some error
************************************************************************/
int GetServiceTypeCodeFromHeader(char *buf_pkt, int len_pkt)
{
	int service_type_index = 5000;
	int company_index = 5000;

	char str_ser_type_idx[SERVICE_TYPE_INDEX_LENGTH_AT_HEADER+1];
	char *e;
	
	/*get company index*/
	if(0 > (company_index = GetEnterpriseCodeFromHeader(buf_pkt, len_pkt)))
	{
		perror("error@analyse.c:GetServiceTypeCode:no such company!!!\n");
		return -1;
	}	
	/*get the string of company_index*/
	bzero(str_ser_type_idx, SERVICE_TYPE_INDEX_LENGTH_AT_HEADER+1);
	memcpy(str_ser_type_idx, buf_pkt+SERVICE_TYPE_INDEX_START_POSITION_AT_HEADER, SERVICE_TYPE_INDEX_LENGTH_AT_HEADER);
	/*transform to integer*/
	service_type_index = strtol(str_ser_type_idx, &e, 10);
	/*verify the company index*/
	if(global_par.company_par_array[company_index].packet_count <= service_type_index)
	{
		perror("error@analyse.c:GetServiceTypeCode:no such service type!!!\n");
		return -1;
	}
	return service_type_index;
}

/*************************************************************************
 *  \brief
 *    Acquire the service type code (Charge, Query or Reversal). 
 *
 *  \par Input:
 *     buf_pkt: the buffer pointer to the packet which is to be parsed.
 *     len_pkt: the length of this packet.
 *  \par Output:
 *  \Return:
 *     >=0: The server code defined in "common.h"
 *    -1: the function has some error
************************************************************************/
int GetPhoneNumberFromHeader(char *buf_pkt, int len_pkt, char *phonenumber)
{
	/*get the string of InternalPacketFlag*/
	memcpy(phonenumber, buf_pkt+PHONE_NUMBER_START_POSTION_AT_HEADER, PHONE_NUMBER_LENGTH_AT_HEADER);
	return 1;
}



/*************************************************************************
 *  \brief
 *    Acquire the internal packet flag (0: external (send to company server), 1: internal (only send to li'an database)). 
 *
 *  \par Input:
 *     buf_pkt: the buffer pointer to the packet which is to be parsed.
 *     len_pkt: the length of this packet.
 *  \par Output:
 *  \Return:
 *     >=0: The internal packet flag 
 *    -1: the function has some error
************************************************************************/
int GetInternalPacketFlagFromHeader(char *buf_pkt, int len_pkt)
{
	int internal_packet_flag = 5000;

	char string_internal_packet_flag[INTERNAL_PACKET_FLAG_LENGTH_AT_HEADER+1];
	char *e;
	
	/*get the string of InternalPacketFlag*/
	bzero(string_internal_packet_flag, INTERNAL_PACKET_FLAG_LENGTH_AT_HEADER+1);
	memcpy(string_internal_packet_flag, buf_pkt+INTERNAL_PACKET_FLAG_START_POSTION_AT_HEADER, INTERNAL_PACKET_FLAG_LENGTH_AT_HEADER);
	/*transform to integer*/
	internal_packet_flag = strtol(string_internal_packet_flag, &e, 10);
	return internal_packet_flag;
}

/*************************************************************************
 *  \brief
 *    Acquire the internal packet flag (0: external (send to company server), 1: internal (only send to li'an database)). 
 *
 *  \par Input:
 *     buf_pkt: the buffer pointer to the packet which is to be parsed.
 *     len_pkt: the length of this packet.
 *  \par Output:
 *  \Return:
 *     >=0: The internal packet flag 
 *    -1: the function has some error
************************************************************************/
int SetInternalPacketFlagInHeader(char *buf_pkt, int len_pkt, const char *internal_flag)
{
	/*get the string of InternalPacketFlag*/
	memcpy(buf_pkt+INTERNAL_PACKET_FLAG_START_POSTION_AT_HEADER, internal_flag, INTERNAL_PACKET_FLAG_LENGTH_AT_HEADER);
	/*transform to integer*/
	return 1;
}

/*************************************************************************
 *  \brief
 *    Acquire the internal packet flag (0: external (send to company server), 1: internal (only send to li'an database)). 
 *
 *  \par Input:
 *     buf_pkt: the buffer pointer to the packet which is to be parsed.
 *     len_pkt: the length of this packet.
 *  \par Output:
 *  \Return:
 *     1: vailid 
 *     others: not valid
************************************************************************/
int ValidRespondCodeInBackwardPacket(char *buf_pkt, int len_pkt, int company_code, int service_type_code)
{
	int success = 0;
	char respond_code[TMP_STRING_LENGTH];
	int respond_code_len = 0;
	char valid_respond_code[TMP_STRING_LENGTH];
	int respond_code_item_index = 0;

	/*get respond code from enterprise feedback*/
	bzero(respond_code, TMP_STRING_LENGTH);
	success = GetRespondCodeFromPacket(buf_pkt, BACKWARD_POSITION, company_code, service_type_code, respond_code,&respond_code_len);
	bzero(valid_respond_code, TMP_STRING_LENGTH);
	respond_code_item_index = global_par.company_par_array[company_code].pkt_par_array[service_type_code][BACKWARD_POSITION].item_index[RESPOND_CODE_ITEM_INDEX];
	memcpy(valid_respond_code, global_par.company_par_array[company_code].pkt_par_array[service_type_code][BACKWARD_POSITION].item_par_array[respond_code_item_index].valid_value, respond_code_len);
	//printf("respond_code : |%s|, valid_code : |%s|\n", respond_code, valid_respond_code);
	if(0 == strcmp(respond_code, valid_respond_code))
	{
		return 1;
	}
	else
	{
		return -1;
	}

}

/*************************************************************************
 *  \brief
 *    Acquire the internal packet flag (0: external (send to company server), 1: internal (only send to li'an database)). 
 *
 *  \par Input:
 *     buf_pkt: the buffer pointer to the packet which is to be parsed.
 *     len_pkt: the length of this packet.
 *  \par Output:
 *  \Return:
 *     1: vailid 
 *     others: not valid
************************************************************************/
int ValidInternalSuccessFlagInHeader(char *buf_pkt, int len_pkt)
{

	if(0 == memcmp(buf_pkt+INTERAL_SUCCESS_FLAG_START_POSITION_AT_HEADER, "00", INTERAL_SUCCESS_FLAG_LENGTH_AT_HEADER))
	{
		return 1;
	}
	else
	{
		return -1;
	}

}

/*************************************************************************
 *  \brief
 *    Acquire the internal packet flag (0: external (send to company server), 1: internal (only send to li'an database)). 
 *
 *  \par Input:
 *     buf_pkt: the buffer pointer to the packet which is to be parsed.
 *     len_pkt: the length of this packet.
 *  \par Output:
 *  \Return:
 *     1: success
 *     others: not success
************************************************************************/
int CompactOnePacketInDatabasePacket(char *forward_pkt, unsigned int forward_pkt_len)
{
	int success = 0;
	char pkt_count_str[11];
	char pkt_backward_len_str[11];
	char pkt_forward_len_str[11];

	bzero(pkt_count_str, 11);
	bzero(pkt_backward_len_str, 11);
	bzero(pkt_forward_len_str, 11);

	sprintf(pkt_count_str, "0000000001");
	sprintf(pkt_forward_len_str, "%010u", forward_pkt_len);
	sprintf(pkt_backward_len_str, "0000000000");

	memcpy(forward_pkt+INTERAL_ERROR_INFO_START_POSITION_AT_HEADER, pkt_count_str, 10);
	memcpy(forward_pkt+INTERAL_ERROR_INFO_START_POSITION_AT_HEADER+10, pkt_forward_len_str, 10);
	memcpy(forward_pkt+INTERAL_ERROR_INFO_START_POSITION_AT_HEADER+20, pkt_backward_len_str, 10);
	
	success = 1;
	return success;
}

/*************************************************************************
 *  \brief
 *    Acquire the internal packet flag (0: external (send to company server), 1: internal (only send to li'an database)). 
 *
 *  \par Input:
 *     buf_pkt: the buffer pointer to the packet which is to be parsed.
 *     len_pkt: the length of this packet.
 *  \par Output:
 *  \Return:
 *     1: success
 *     others: not success
************************************************************************/
int CompactTwoPacketInDatabasePacket(char *backward_pkt, unsigned int backward_pkt_len, char *forward_pkt, unsigned int forward_pkt_len)
{
	int success = 0;
	char pkt_count_str[11];
	char pkt_backward_len_str[11];
	char pkt_forward_len_str[11];

	char pkt_tmp[MAXPACKETSIZE];
	
	bzero(pkt_count_str, 11);
	bzero(pkt_backward_len_str, 11);
	bzero(pkt_forward_len_str, 11);

	bzero(pkt_tmp, MAXPACKETSIZE);
	
	sprintf(pkt_count_str, "0000000002");
	sprintf(pkt_backward_len_str, "%010u", backward_pkt_len-PACKET_HEADER_LENGTH);
	sprintf(pkt_forward_len_str, "%010u", forward_pkt_len-PACKET_HEADER_LENGTH);

	//printf("backward pkt : |%s|             %d\n", backward_pkt, strlen(backward_pkt));
	//printf("forward pkt : |%s|             %d\n", forward_pkt, strlen(forward_pkt));
	
	memcpy(pkt_tmp, backward_pkt, PACKET_HEADER_LENGTH);

	memcpy(pkt_tmp+INTERAL_ERROR_INFO_START_POSITION_AT_HEADER, pkt_count_str, 10);
	memcpy(pkt_tmp+INTERAL_ERROR_INFO_START_POSITION_AT_HEADER+10, pkt_forward_len_str, 10);
	memcpy(pkt_tmp+INTERAL_ERROR_INFO_START_POSITION_AT_HEADER+20, pkt_backward_len_str, 10);

	memcpy(pkt_tmp+PACKET_HEADER_LENGTH, forward_pkt+PACKET_HEADER_LENGTH, forward_pkt_len-PACKET_HEADER_LENGTH);
	memcpy(pkt_tmp+forward_pkt_len, backward_pkt+PACKET_HEADER_LENGTH, backward_pkt_len-PACKET_HEADER_LENGTH);

	memcpy(backward_pkt, pkt_tmp, forward_pkt_len+backward_pkt_len-PACKET_HEADER_LENGTH);
	//memset(backward_pkt+forward_pkt_len+backward_pkt_len-PACKET_HEADER_LENGTH,0,1);	
	success = 1;
	return success;
}


/*************************************************************************
 *  \brief
 *    Acquire the client serial number from pkt according enterprise_code. 
 *
 *  \par Input:
 *     buf_pkt: the buffer pointer to the packet which is to be parsed.
 *     len_pkt: the length of this packet.
 *     client_id: the buf contained the client_id.
 *     len_client_id: the length of the client_id.
 *  \par Output:
 *  \Return:
 *     1: success
 *     0: failure
************************************************************************/
int GetClientSerialFromHeader(char *buf_pkt, int len_pkt,  char *client_id, int *len_client_id)
{
	/*get the string of company_index*/
	bzero(client_id, *len_client_id);
	memcpy(client_id, buf_pkt+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, CLIENT_ID_INDEX_LENGTH_AT_HEADER);

	*len_client_id = CLIENT_ID_INDEX_LENGTH_AT_HEADER;
	return 1;
}

/*************************************************************************
 *  \brief
 *    Write internal success flag to packet head. 
 *
 *  \par Input:
 *     char *packet: packet to send
 *     char error_code: error code, if success = "00", else fail
 *	   char *error_info: the information of reason of failure
 *  \par Output:
 *  \Return:
 *    1: Success
 *    -: error 
************************************************************************/
int WriteInternalSuccessFlag(char *packet, const char* error_code, const char *error_info)
{
	memcpy(packet + INTERAL_SUCCESS_FLAG_START_POSITION_AT_HEADER, error_code, INTERAL_SUCCESS_FLAG_LENGTH_AT_HEADER);
	memset(packet + INTERAL_ERROR_INFO_START_POSITION_AT_HEADER, ' ', INTERAL_ERROR_INFO_LENGTH_AT_HEADER);
	memcpy(packet + INTERAL_ERROR_INFO_START_POSITION_AT_HEADER, global_par.system_par.localhost_id, LOCALHOST_ID_LENGTH);
	if(INTERAL_ERROR_INFO_LENGTH_AT_HEADER>=(strlen(error_info)+LOCALHOST_ID_LENGTH))
	{
		memcpy(packet + INTERAL_ERROR_INFO_START_POSITION_AT_HEADER+LOCALHOST_ID_LENGTH, error_info, strlen(error_info));
	}
	else
	{
		memcpy(packet + INTERAL_ERROR_INFO_START_POSITION_AT_HEADER+LOCALHOST_ID_LENGTH, error_info, INTERAL_ERROR_INFO_LENGTH_AT_HEADER-LOCALHOST_ID_LENGTH);
	}
	return 1;
}

/* *************************************************
 *  \brief
 *    get the respond code from packet
 *
 * Input: 
 *		char *pkt ---> the pointer of buffer contained the packet to be filled in
 *      int pkt_postion ---> the direction of packet transmitted (0:forward, 1:backward)
 *      int company_index ---> the index of company
 *		int service_type_index ---> the index of serial type
 *
 * Output:
 *      respond_code: the respond code identify the success flag("00":success, if it is not "00", transaction is failure)
 *		respond_code_len: the length of respond code.
 *		
 * return:
 * 		1 ---> success
 * 		0 ---> error
 * *************************************************/
int GetRespondCodeFromPacket(char *pkt, int pkt_postion, int company_index, int service_type_index, char *respond_code, int *respond_code_len)
{
	int respond_code_item_index = 0;
	int direction = 0;
	int start_pos = 0;
	int len = 0;
	int pkt_len = strlen(pkt);
	
	respond_code_item_index = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_index[RESPOND_CODE_ITEM_INDEX];
	direction = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_par_array[respond_code_item_index].direction;
	start_pos = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_par_array[respond_code_item_index].start_pos;
	len = global_par.company_par_array[company_index].pkt_par_array[service_type_index][pkt_postion].item_par_array[respond_code_item_index].len;
	
	/*verify the respond code exceed the bondary*/
	if(0 == direction) //items directions, 0: positive; 1:negtive.
	{
			memcpy(respond_code, pkt+start_pos,len);
	}
	else
	{
			memcpy(respond_code, pkt+pkt_len-start_pos-1, len);
	}
	*respond_code_len = len;
	
	return 1;	
}



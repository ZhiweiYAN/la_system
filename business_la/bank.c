/*********************************************************
 *project: Line communication charges supermarket
 *filename: bank.c
 *version: 0.4
 *purpose: functions about bank query and withdraw the money 
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-9
 *********************************************************/
 
#include "bank.h"

/*************************************************************************
 *  \brief
 *    Acquire the client has the right to excuting the affairs(Yes or no). 
 *    Detail implementation will be done in the future.
 *
 *  \par Input:
 *     client_serial: the serial NO of the client.
 *     client_len: the string length of client serial.
 *     charge_money: the money substruct.
 *  \par Output:
 *  \Return:
 *    1: has the right
 *    0: has no the right
 *    <0: the function has some error
************************************************************************/
int WarrantExcuteAffair(char *client_serial, int client_len, unsigned long int charge_money, int company_index)
{
    /*Now, nothing to do, just do give it right*/
	//return 1;
	int sock_bank = 0, recv_len = 0, success = 0;
	char query_bank_string[TMP_STRING_LENGTH];
	sock_bank = CreateSocket();
	char tmp_string[TMP_STRING_LENGTH];
	char buf_send[TMP_STRING_LENGTH];
	char buf_recv[TMP_STRING_LENGTH];
    success = ConnectServer(global_par.system_par.bank_ip_address, global_par.system_par.bank_port, sock_bank);
	
	bzero(query_bank_string, TMP_STRING_LENGTH);
	bzero(tmp_string, TMP_STRING_LENGTH);
	bzero(buf_send, TMP_STRING_LENGTH);
	bzero(buf_recv, TMP_STRING_LENGTH);
	sprintf(query_bank_string, "    12          %02d1", company_index);
	memcpy(query_bank_string+6, client_serial, client_len);
	sprintf(tmp_string, "%012lu", charge_money);
	strcat(query_bank_string, tmp_string);
	strcpy(buf_send, query_bank_string);
#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	fflush(NULL);
	printf("send to bank WarrantExcuteAffair packet length= %d \n", strlen(buf_send));
	printf("send to bank WarrantExcuteAffair packet is %s \n", buf_send);
	fflush(NULL);	
#endif
        if(send(sock_bank,buf_send,strlen(buf_send),0)==-1)
        {
        	success = 0;
                perror("error@bank.c:WarrantExcuteAffair:send");
                exit(1);
        }

	/*recieve the feedback packet from bank*/
        recv_len = recv(sock_bank, buf_recv, TMP_STRING_LENGTH, 0);
#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	fflush(NULL);
	printf("recv from bank WarrantExcuteAffair packet length= %d \n", recv_len);
	printf("recv from bank WarrantExcuteAffair packet is %s \n", buf_recv);
	fflush(NULL);	
#endif
        if(recv_len<0)
        {
        	success = 0;
                perror("error@bank.c:WarrantExcuteAffair:recv");
               	exit(1);
        }
	
	close(sock_bank);

	if(0 == memcmp(buf_recv+15, "0", 1))
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

/*************************************************************************
 *  \brief
 *    Subtract the money from client account. 
 *    Detail implementation will be done in the future.
 *
 *  \par Input:
 *     client_serial: the serial NO of the client.
 *     client_len: the string length of client serial.
 *     charge_money: the money substruct.
 *  \par Output:
 *  \Return:
 *    1: success
 *    0: not success
************************************************************************/
int SubtractMoneyFromClientAccount(char *client_serial, int client_len, unsigned long int charge_money, int company_index)
{
    /*Now, nothing to do, just do give it right*/
	//return 1;
	int sock_bank = 0, recv_len = 0, success = 0;
	char query_bank_string[TMP_STRING_LENGTH];
	sock_bank = CreateSocket();
	char tmp_string[TMP_STRING_LENGTH];
	char buf_send[TMP_STRING_LENGTH];
	char buf_recv[TMP_STRING_LENGTH];
    success = ConnectServer(global_par.system_par.bank_ip_address, global_par.system_par.bank_port, sock_bank);
	
	bzero(query_bank_string, TMP_STRING_LENGTH);
	bzero(tmp_string, TMP_STRING_LENGTH);
	bzero(buf_send, TMP_STRING_LENGTH);
	bzero(buf_recv, TMP_STRING_LENGTH);
	sprintf(query_bank_string, "    12          %02d2", company_index);
	memcpy(query_bank_string+6, client_serial, client_len);
	sprintf(tmp_string, "%012lu", charge_money);
	strcat(query_bank_string, tmp_string);

	strcpy(buf_send, query_bank_string);
#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	fflush(NULL);
	printf("send to bank SubtractMoneyFromClientAccount packet length= %d \n", strlen(buf_send));
	printf("send to bank SubtractMoneyFromClientAccount packet is %s \n", buf_send);
	fflush(NULL);	
#endif
        if(send(sock_bank,buf_send,strlen(buf_send),0)==-1)
        {
        	success = 0;
                perror("error@bank.c:SubtractMoneyFromClientAccount:send");
                exit(1);
        }

	/*recieve the feedback packet from bank*/
        recv_len = recv(sock_bank, buf_recv, TMP_STRING_LENGTH, 0);
#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	fflush(NULL);
	printf("recv from bank SubtractMoneyFromClientAccount packet length= %d \n", recv_len);
	printf("recv from bank SubtractMoneyFromClientAccount packet is %s \n", buf_recv);
	fflush(NULL);	
#endif
        if(recv_len<0)
        {
        	success = 0;
                perror("error@bank.c:SubtractMoneyFromClientAccount:recv");
               	exit(1);
        }
	
	close(sock_bank);

	if(0 == memcmp(buf_recv+15, "0", 1))
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

/*************************************************************************
 *  \brief
 *    Add the money to client account. 
 *    Detail implementation will be done in the future.
 *
 *  \par Input:
 *     client_serial: the serial NO of the client.
 *     client_len: the string length of client serial.
 *     reversal_money: the money of reveral operation.
 *  \par Output:
 *  \Return:
 *    1: success
 *    0: not success
************************************************************************/
int AddMoneyToClientAccount(char *client_serial, int client_len, unsigned long int reversal_money, int company_index)
{
    /*Now, nothing to do, just do give it right*/
	//return 1;
	int sock_bank = 0, recv_len = 0, success = 0;
	char query_bank_string[TMP_STRING_LENGTH];
	sock_bank = CreateSocket();
	char tmp_string[TMP_STRING_LENGTH];
	char buf_send[TMP_STRING_LENGTH];
	char buf_recv[TMP_STRING_LENGTH];
    success = ConnectServer(global_par.system_par.bank_ip_address, global_par.system_par.bank_port, sock_bank);
	
	bzero(query_bank_string, TMP_STRING_LENGTH);
	bzero(tmp_string, TMP_STRING_LENGTH);
	bzero(buf_send, TMP_STRING_LENGTH);
	bzero(buf_recv, TMP_STRING_LENGTH);
	sprintf(query_bank_string, "    12          %02d3", company_index);
	memcpy(query_bank_string+6, client_serial, client_len);
	sprintf(tmp_string, "%012lu", reversal_money);
	strcat(query_bank_string, tmp_string);
	strcpy(buf_send, query_bank_string);

#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	fflush(NULL);
	printf("send to bank AddMoneyToClientAccount packet length= %d \n", strlen(buf_send));
	printf("send to bank AddMoneyToClientAccount packet is %s \n", buf_send);
	fflush(NULL);	
#endif
       if(send(sock_bank,buf_send,strlen(buf_send),0)==-1)
        {
        	success = 0;
                perror("error@bank.c:AddMoneyToClientAccount:send");
                exit(1);
        }

	/*recieve the feedback packet from bank*/
        recv_len = recv(sock_bank, buf_recv, TMP_STRING_LENGTH, 0);
#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	fflush(NULL);
	printf("recv from bank AddMoneyToClientAccount packet length= %d \n", recv_len);
	printf("recv from bank AddMoneyToClientAccount packet is %s \n", buf_recv);
	fflush(NULL);	
#endif
        if(recv_len<0)
        {
        	success = 0;
                perror("error@bank.c:AddMoneyToClientAccount:recv");
               	exit(1);
        }
	
	close(sock_bank);
	
	if(0 == memcmp(buf_recv+15, "0", 1))
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

/*************************************************************************
 *  \brief
 *    Acquire the client has the right to excuting the affairs(Yes or no). 
 *    Detail implementation will be done in the future.
 *
 *  \par Input:
 *     client_serial: the serial NO of the client.
 *     client_len: the string length of client serial.
 *     charge_money: the money substruct.
 *  \par Output:
 *  \Return:
 *    1: has the right
 *    0: has no the right
 *    <0: the function has some error
************************************************************************/
int QueryWarrantExcuteAffair(char *client_serial, int client_len, unsigned long int charge_money)
{
    /*Now, nothing to do, just do give it right*/
	int sock_bank = 0, recv_len = 0, success = 0;
	char query_bank_string[TMP_STRING_LENGTH];
	sock_bank = CreateSocket();
	char tmp_string[TMP_STRING_LENGTH];
	char buf_send[TMP_STRING_LENGTH];
	char buf_recv[TMP_STRING_LENGTH];
	/*judge the destination of the packet, read some share varibles*/
  	if(1 == JudgePrimaryDatabaseServer())
   	{
   		/*primary database is alive now*/
   		success = ConnectServerOneTime(global_par.system_par.database_self_ip_address, global_par.system_par.business_virtual_money_port, sock_bank);
   	}
   	else
   	{
   		/*primary database is crush now*/
   		success = ConnectServerOneTime(global_par.system_par.database_brother_ip_address, global_par.system_par.business_virtual_money_port, sock_bank);
	}
	
	if(1 != success)
	{
		close(sock_bank);
      		success = 0;
        	perror("error@bank.c:QueryWarrantExcuteAffair:connect");
       		exit(1);
       	}
	
	bzero(query_bank_string, TMP_STRING_LENGTH);
	bzero(tmp_string, TMP_STRING_LENGTH);
	bzero(buf_send, TMP_STRING_LENGTH);
	bzero(buf_recv, TMP_STRING_LENGTH);
	sprintf(query_bank_string, "00");
	//strcat(query_bank_string, client_serial);
	memcpy(query_bank_string+2, client_serial, CLIENT_ID_INDEX_LENGTH_AT_HEADER);
	sprintf(tmp_string, "%010lu", charge_money);
	strcat(query_bank_string, tmp_string);
	strcpy(buf_send, query_bank_string);
#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	fflush(NULL);
	printf("send QueryWarrantExcuteAffair packet length= %d \n", strlen(buf_send));
	printf("send QueryWarrantExcuteAffair packet is %s \n", buf_send);
	fflush(NULL);	
#endif
        if(send(sock_bank,buf_send,strlen(buf_send),0)==-1)
        {
        	success = 0;
                perror("error@bank.c:WarrantExcuteAffair:send");
                exit(1);
        }

	/*recieve the feedback packet from bank*/
        recv_len = recv(sock_bank, buf_recv, TMP_STRING_LENGTH, 0);
#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	fflush(NULL);
	printf("recv QueryWarrantExcuteAffair packet length= %d \n", recv_len);
	printf("recv QueryWarrantExcuteAffair packet is %s \n", buf_recv);
	fflush(NULL);	
#endif
    	if(recv_len<0)
    	{
      		success = 0;
        	perror("error@bank.c:WarrantExcuteAffair:recv");
       		exit(1);
    	}
	
	close(sock_bank);

	if(0 == memcmp(buf_recv+10, "00", 2))
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

/*************************************************************************
 *  \brief
 *    Subtract the money from client account. 
 *    Detail implementation will be done in the future.
 *
 *  \par Input:
 *     client_serial: the serial NO of the client.
 *     client_len: the string length of client serial.
 *     charge_money: the money substruct.
 *  \par Output:
 *  \Return:
 *    1: success
 *    0: not success
************************************************************************/
int RequireSubtractMoneyFromClientAccount(char *client_serial, int client_len, unsigned long int charge_money)
{
	int sock_bank = 0, recv_len = 0, success = 0;
	char query_bank_string[TMP_STRING_LENGTH];
	sock_bank = CreateSocket();
	char tmp_string[TMP_STRING_LENGTH];
	char buf_send[TMP_STRING_LENGTH];
	char buf_recv[TMP_STRING_LENGTH];
	/*judge the destination of the packet, read some share varibles*/
  	if(1 == JudgePrimaryDatabaseServer())
   	{
   		/*primary database is alive now*/
   		success = ConnectServerOneTime(global_par.system_par.database_self_ip_address, global_par.system_par.business_virtual_money_port, sock_bank);
   	}
   	else
   	{
   		/*primary database is crush now*/
   		success = ConnectServerOneTime(global_par.system_par.database_brother_ip_address, global_par.system_par.business_virtual_money_port, sock_bank);
	}
	
	if(1 != success)
	{
      		success = 0;
		close(sock_bank);
        	perror("error@bank.c:RequireSubtractMoneyFromClientAccount:connect");
		return -1;
       	}
       		
	bzero(query_bank_string, TMP_STRING_LENGTH);
	bzero(tmp_string, TMP_STRING_LENGTH);
	bzero(buf_send, TMP_STRING_LENGTH);
	bzero(buf_recv, TMP_STRING_LENGTH);
	sprintf(query_bank_string, "01");
	strcat(query_bank_string, client_serial);
	sprintf(tmp_string, "%010lu", charge_money);
	strcat(query_bank_string, tmp_string);
	strcpy(buf_send, query_bank_string);
#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	fflush(NULL);
	printf("send RequireSubtractMoneyFromClientAccount packet length= %d \n", strlen(buf_send));
	printf("send RequireSubtractMoneyFromClientAccount packet is %s \n", buf_send);
	fflush(NULL);	
#endif
        if(send(sock_bank,buf_send,strlen(buf_send),0)==-1)
        {
        	success = 0;
		close(sock_bank);
                perror("error@bank.c:WarrantExcuteAffair:send");
		return -1;
        }

	/*recieve the feedback packet from bank*/
        recv_len = recv(sock_bank, buf_recv, TMP_STRING_LENGTH, 0);
#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	fflush(NULL);
	printf("recv RequireSubtractMoneyFromClientAccount packet length= %d \n", recv_len);
	printf("recv RequireSubtractMoneyFromClientAccount packet is %s \n", buf_recv);
	fflush(NULL);	
#endif
    if(recv_len<0)
    {
      	success = 0;
	close(sock_bank);
        perror("error@bank.c:WarrantExcuteAffair:recv");
	return -1;
    }
	
	close(sock_bank);
	return 1;
	
}

/*************************************************************************
 *  \brief
 *    Add the money to client account. 
 *    Detail implementation will be done in the future.
 *
 *  \par Input:
 *     client_serial: the serial NO of the client.
 *     client_len: the string length of client serial.
 *     reversal_money: the money of reveral operation.
 *  \par Output:
 *  \Return:
 *    1: success
 *    0: not success
************************************************************************/
int RequireAddMoneyToClientAccount(char *client_serial, int client_len, unsigned long int charge_money)
{
	int sock_bank = 0, recv_len = 0, success = 0;
	char query_bank_string[TMP_STRING_LENGTH];
	sock_bank = CreateSocket();
	char tmp_string[TMP_STRING_LENGTH];
	char buf_send[TMP_STRING_LENGTH];
	char buf_recv[TMP_STRING_LENGTH];
	/*judge the destination of the packet, read some share varibles*/
  	if(1 == JudgePrimaryDatabaseServer())
   	{
   		/*primary database is alive now*/
   		success = ConnectServerOneTime(global_par.system_par.database_self_ip_address, global_par.system_par.business_virtual_money_port, sock_bank);
   	}
   	else
   	{
   		/*primary database is crush now*/
   		success = ConnectServerOneTime(global_par.system_par.database_brother_ip_address, global_par.system_par.business_virtual_money_port, sock_bank);
	}
	
	if(1 != success)
	{
      		success = 0;
		close(sock_bank);
        	perror("error@bank.c:RequireAddMoneyToClientAccount:connect");
		return -1;
       	}
       		
	bzero(query_bank_string, TMP_STRING_LENGTH);
	bzero(tmp_string, TMP_STRING_LENGTH);
	bzero(buf_send, TMP_STRING_LENGTH);
	bzero(buf_recv, TMP_STRING_LENGTH);
	sprintf(query_bank_string, "02");
	strcat(query_bank_string, client_serial);
	sprintf(tmp_string, "%010lu", charge_money);
	strcat(query_bank_string, tmp_string);
	strcpy(buf_send, query_bank_string);
#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	fflush(NULL);
	printf("send RequireAddMoneyToClientAccount packet length= %d \n", strlen(buf_send));
	printf("send RequireAddMoneyToClientAccount packet is %s \n", buf_send);
	fflush(NULL);	
#endif
    if(send(sock_bank,buf_send,strlen(buf_send),0)==-1)
    {
      	success = 0;
	close(sock_bank);
        perror("error@bank.c:WarrantExcuteAffair:send");
	return -1;
    }

	/*recieve the feedback packet from bank*/
        recv_len = recv(sock_bank, buf_recv, TMP_STRING_LENGTH, 0);
#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	fflush(NULL);
	printf("recv RequireAddMoneyToClientAccount packet length= %d \n", recv_len);
	printf("recv RequireAddMoneyToClientAccount packet is %s \n", buf_recv);
	fflush(NULL);	
#endif
    if(recv_len<0)
    {
      	success = 0;
	close(sock_bank);
        perror("error@bank.c:WarrantExcuteAffair:recv");
	return -1;
    }
	
	close(sock_bank);
	return 1;
}

/*************************************************************************
 *  \brief
 *    Send a packet to a server for add value service (for example, send a message to a mobile phone). 
 *
 *  \par Input:
 *     pkt: the send packet .
 *     pkt_len: the string length of pkt.
 *  \par Output:
 *  \Return:
 *    1: success
 *    0: not success
************************************************************************/
int SendPacketToAddValueServer(char *pkt, int pkt_len)
{
	//原来， 需要向联通服务器的短信接口发包，现在不需要了，
	return 1;
	
	int sock_bank = 0, success = 0;
	sock_bank = CreateSocket();
	char buf_send[MAXPACKETSIZE];
	//char buf_recv[MAXPACKETSIZE];

	/*primary database is alive now*/
	success = ConnectServerOneTime((char*)"10.176.191.242", 8003, sock_bank);
	
	if(1 != success)
	{
      		success = 0;
		close(sock_bank);
        	perror("error@bank.c:SendPacketToAddValueServer:connect");
		return -1;
       }
       		
	bzero(buf_send, MAXPACKETSIZE);
	//bzero(buf_recv, MAXPACKETSIZE);
	memcpy(buf_send, pkt, pkt_len);
	
#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	fflush(NULL);
	printf("send SendPacketToAddValueServer packet length= %d \n", strlen(buf_send));
	printf("send SendPacketToAddValueServer packet is %s \n", buf_send);
	fflush(NULL);	
#endif
        if(send(sock_bank,buf_send,strlen(buf_send),0)==-1)
        {
        	success = 0;
		close(sock_bank);
                perror("error@bank.c:WarrantExcuteAffair:send");
		return -1;
        }

	/*recieve the feedback packet from bank*/
        //recv_len = recv(sock_bank, buf_recv, MAXPACKETSIZE, 0);
#ifdef DEBUG_TRANSMIT_BANK_AFFAIR
	//fflush(NULL);
	//printf("recv SendPacketToAddValueServer packet length= %d \n", recv_len);
	//printf("recv SendPacketToAddValueServer packet is %s \n", buf_recv);
	//fflush(NULL);	
#endif
    //if(recv_len<0)
    //{
    //  	success = 0;
	//close(sock_bank);
   //     perror("error@bank.c:WarrantExcuteAffair:recv");
	//return -1;
   // }
	
	close(sock_bank);
	return 1;
	
}


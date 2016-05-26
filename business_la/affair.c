/*********************************************************
 *project: Line communication charges supermarket
 *filename: affair.c
 *version: 0.4
 *purpose: some function have relationship with affair process 
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-9
 *********************************************************/
 
#include "affair.h"
//#include "assert.h"

/*************************************************************************
 *  \brief
 *    Create a affair process when accept one connection from client 
 *    
 *   fork teo times in order to pervent from create zombie process
 *
 *  \par Input:
 *     buf_retransmit: the buffer contain the retransmited packet.
 *     retran_len: the length of the retransmited packet.
 *     buf_feedback: the buffer contain the feedback packet.
 *     feedback: because buf_feedback is malloc outside,so need the length of the feedback packet (the buffer length).
 *     sockid: the socket.
 *  \par Output:
 *     feedback: the real length of the feedback packet.   
 *  \Return:
 *    1: success
 *    0: error
************************************************************************/
int CreateProcessAcceptClient(int listen_sock, int listen_num)
{
	struct sockaddr_in client_addr; /* information for client which has connected to localhost */
  	unsigned int sin_size;
	int pid;
  	int new_sock_server; /*socket for accept client*/

    /*initial the semphore and share memory for serial number item*/
    InitSemShmForSerialNumber();
    
	if (listen(listen_sock, listen_num) == -1) 
	{
		perror("error@business.c:main\n");
		exit(1);
   	}
   /*begin accept the business from proxy*/
   	sin_size = sizeof(struct sockaddr_in);
   while(1) 
   {
   	fflush(NULL);
	printf("\r\033[01;32mMain process is Alive : random number = %d !!!\033[0m          ", rand());
	fflush(NULL);
   	
	if ((new_sock_server = accept(listen_sock, (struct sockaddr *)&client_addr, &sin_size)) == -1)
   	{
   		perror("error@affair.c:CreateProcessAcceptClient:accept\n");
   		continue;
   	}
   	//printf("server: got connection from %s\n",inet_ntoa(client_addr.sin_addr));
       		
		if ((pid = fork())<0)
      {
      	perror("error@affair.c:CreateProcessAcceptClient:fork_1\n");
		close(new_sock_server);
		continue;
		sleep(2);
      	//exit(1);
      }	
      else if (0==pid) /* first child */ 
      {
      	/*close the listening socket*/
			close(listen_sock);
		
			/*first child process to handle the business from proxy*/
   		if((pid=fork())<0)
   		{
		   	perror("error@affair.c:CreateProcessAcceptClient:fork_2\n");
   			exit(1);
   		}
   		else if (pid>0)
   		{
   			/*parent for second fork == fist child */
   			/*Make it exit normally*/
   			exit(0);
   			/*We're the second child; our parent becomes init as soon
   			as our real parent calls exit() in the statement above.
   			Here's where we'd continue executing, knowing that when
   			we're done, init will reap our status. */
   		}
   			
   		/* this is the second child process! */
   		
			/*Regiest this process PID to data channel lift table*/
			SavePidToArray();

   		/*handle the business from proxy machine, need be implement*/    			
   		HandleBusinessFromProxy(new_sock_server);
   		
   		/*this sock need not exist after handle the business, so close the new sock and exit*/
   		close(new_sock_server);

			/*UnRegiest this process PID from data channel lift table*/
			DeletePidFromArray();

         exit(0);
		}
      	close(new_sock_server); /*parent process need not the new socket*/
		/*wait the child process end and clean the resources occupied by
		child process! the sentense is important!!!*/             
		//waitpid(-1,NULL,WNOHANG);
		if(waitpid(pid, NULL, 0)!=pid)
			perror("error@affair.c:CreateProcessAcceptClient:waitpid\n");	
   }
	return 1;
}

/*************************************************************************
 *  \brief
 *    retransmit the recieved packet to enterprise server with temp link
 *    
 *   1. retransmit the packet with socket.
 *
 *  \par Input:
 *     buf_retransmit: the buffer contain the retransmited packet.
 *     retran_len: the length of the retransmited packet.
 *     buf_feedback: the buffer contain the feedback packet.
 *     feedback: because buf_feedback is malloc outside,so need the length of the feedback packet (the buffer length).
 *     enterprise_code: the company server type (ChinaUnicom or ChinaMobile).
 *  \par Output:
 *     feedback: the real length of the feedback packet.   
 *  \Return:
 *    1: success
 *    0: error
************************************************************************/
int TransmitPacketWithDirectLink(char *buf_retransmit, int retran_len, char *buf_feedback, int *feedback, int enterprise_code, int service_type)
{
	int success;
	int recv_fd_len;
	int sock_sc;

	sock_sc = CreateSocket();
	success = ConnectServerOneTime(global_par.company_par_array[enterprise_code].network_par.ip_address, global_par.company_par_array[enterprise_code].network_par.port, sock_sc);

	/*retransmit the packet to enterprise server*/
#ifdef DEBUG_TRANSMIT_TELECOM_ENTERPRISE_SERVER
	fflush(NULL);
	printf("send to TELECOM ENTERPRISE SERVER packet length = %d \n", retran_len);
	printf("send to TELECOM ENTERPRISE SERVER packet is |%s| \n", buf_retransmit);
	fflush(NULL);
#endif
	if(send(sock_sc,buf_retransmit,retran_len,0)==-1) 
	{
		success = 0;
		perror("error@affair.c:TransmitPacketWithDirectLink:send company");
		exit(1);
    	}

	/*recieve the feedback packet from enterprise server*/
	bzero(buf_feedback, MAXPACKETSIZE);
	recv_fd_len = recv(sock_sc, buf_feedback, MAXPACKETSIZE, 0);

#ifdef DEBUG_TRANSMIT_TELECOM_ENTERPRISE_SERVER
	fflush(NULL);
	printf("recv from TELECOM ENTERPRISE SERVER packet length = %d \n", recv_fd_len);
	printf("recv from TELECOM ENTERPRISE SERVER packet is :%s \n", buf_feedback);
	fflush(NULL);
#endif
	if(recv_fd_len<0)
	{
		success = 0;
		perror("error@affair.c:TransmitPacketWithDirectLink:recv company");
		exit(1);
	}

	*feedback = recv_fd_len;
	close(sock_sc);

	success = 1;
	return success;
}

/*************************************************************************
 *  \brief
 *    Handle the business from proxy
 *    In this version, the affair process connect Unicom server itself
 *
 *  1. recieve the packet
 *  2. return the processed packet to sender
 *
 *  \par Input:
 *    sock_proxy: the socket accept connection from proxy, 
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/ 
int HandleBusinessFromProxy(int sock_proxy)
{
	/*vaiables for recv and send packet by sockt*/
	/*from and to proxy*/
	int recv_from_proxy_len = 0;
	int send_to_proxy_len = 0;
	//char *recv_from_proxy_buf = NULL;
	char  *send_to_proxy_buf = NULL;

	int success = 0;

	/*the buf of common recv buf*/
	char buf_recv[MAXPACKETSIZE];
	
	/* varible for enterprise server*/
	int company_code = -1;
	//char client_id[CLIENT_ID_INDEX_LENGTH_AT_HEADER+1];
	//int len_client_id = 0;
	int service_type_code = -1;
	int internal_packet_flag = -1;
	//unsigned long int charge_money = 0;
	//char la_serial_number[SERIAL_NUMBER_LENGTH + 1] = "00";
	//char packet_header[PACKET_HEADER_LENGTH+1];
	
	/*�����ݿ�Ĳ������Ӿ��*/
	//int sock_db;

	/*recieve the packet from proxy*/
	bzero(buf_recv, MAXPACKETSIZE);
	recv_from_proxy_len = multi_recv(sock_proxy, buf_recv, MAXPACKETSIZE, 0);
#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
	fflush(NULL);
	printf("recv affair packet from proxy length= %d \n", recv_from_proxy_len);
	printf("recv affair packet from proxy is |%s| \n", buf_recv);
	fflush(NULL);
#endif

	/*judge the recv packet, it can be more detail in the future*/
  	if(recv_from_proxy_len<0)
	{
		success = 0;
		perror("error@business.c:HandleBusinessFromProxy:recv_from_proxy_len");
		exit(1);
	}
	/*���ն˺ŵĵ�3λ��Ϊ0�����罫HS188888��ΪHS088888*/
	memset(buf_recv+CLIENT_ID_INDEX_START_POSITION_AT_HEADER+2, '0', 1);
	
	/*�õ��ڲ������*/
	if(-1 == (internal_packet_flag = GetInternalPacketFlagFromHeader(buf_recv, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetInternalPacketFlagFromHeader");
		exit(1);
	}

	/*�õ���ҵ��*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(buf_recv, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*�õ�ҵ��������*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(buf_recv, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}
	if(0 == internal_packet_flag)
	{
		if(1 == global_par.company_par_array[company_code].packet_important_level[service_type_code])
		{
			printf("\n\n|%d|\n\n",company_code);
			//if(1 == company_code)
			//{
				//����ǵ��ŵķ��������ȼ�¼�����ݿ��У�
				//����������������õ��������
				//���ն˷��سɹ�
				//HandleTelecomReveralBusinessPacket(sock_proxy, buf_recv);
				//HandleTelecomReveralBusinessPacketNew(sock_proxy, buf_recv);
				//HandleReveralBusinessPacketNew(sock_proxy, buf_recv);
			//}
			//else
			//{
			//	printf("\n\n|zhixing|\n\n",company_code);
				HandleReveralBusinessPacket(sock_proxy, buf_recv);
			//}
		}
		else if(2 == global_par.company_par_array[company_code].packet_important_level[service_type_code])
		{
			HandleChargeBusinessPacket(sock_proxy, buf_recv);
		}
		else if(10 <= global_par.company_par_array[company_code].packet_important_level[service_type_code])
		{
			HandlePreviousFeeBusinessPacket(sock_proxy, buf_recv);
		}
		else
		{
			//printf("pkt:|%s|\n", recv_from_proxy_buf);
			HandleQueryFeeBusinessPacket(sock_proxy, buf_recv);
		}
	}
	else if(1 == internal_packet_flag)
	{
		HandleInvoiceQueryBusinessPacket(sock_proxy, buf_recv);
	}
	else if(2 == internal_packet_flag)
	{
		HandleInternalBusinessPacket(sock_proxy, buf_recv);
	}
	else if(4 == internal_packet_flag)
	{
		if(1 == global_par.company_par_array[company_code].packet_important_level[service_type_code])
		{
			HandleLoopReveralBusinessPacket(sock_proxy, buf_recv);
		}
		else if(2 == global_par.company_par_array[company_code].packet_important_level[service_type_code])
		{
			HandleLoopChargeBusinessPacket(sock_proxy, buf_recv);
		}
	}
	else
	{
		perror("Internal_packet_flag is not 1, neither 2, and neither 0, must be error (not internal, not external)");
		send_to_proxy_buf = buf_recv;
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "01", "�ڲ�����ǲ�����0��1��2��4�е�һ��!");

#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
	fflush(NULL);
	printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
	printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
	fflush(NULL);
#endif
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send company feedback to proxy (important level = 0)");
			exit(1);
   		}
		
		success = -1;
		return success;
		
	}
	return 1;

}



/*************************************************************************
 *  \brief
 *    ����ɷ������(�ⲿ��)
 *
 *  1. recieve the packet
 *  2. return the processed packet to sender
 *
 *  \par Input:
 *    sock_proxy: the socket accept connection from proxy, 
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/ 
int HandleChargeBusinessPacket(int sock_proxy, char *pkt_from_proxy)
{
	/*vaiables for recv and send packet by sockt*/
	/*from and to proxy*/
	int recv_from_proxy_len = 0;
	int send_to_proxy_len = 0;
	char recv_from_proxy_buf[MAXPACKETSIZE];
	char send_to_proxy_buf[MAXPACKETSIZE];
	/*from and to company*/
	int recv_from_company_len = 0;
	int send_to_company_len = 0;
	char  recv_from_company_buf[MAXPACKETSIZE];
	char  *send_to_company_buf = NULL;
	/*from and to database*/
	int recv_from_db_len = 0;
	int send_to_db_len = 0;
	char  *recv_from_db_buf = NULL;
	char  send_to_db_buf[MAXPACKETSIZE];
	/*from and to database of forward packet*/
	//int recv_from_db_forward_pkt_len = 0;
	//int send_to_db_forward_pkt_len = 0;
	//char  *recv_from_db_forward_pkt_buf = NULL;
	//char  *send_to_db_forward_pkt_buf = NULL;
       /*���������ݿ��ǰ����ͺ����*/
	//int send_db_reserval_forward_pkt_len = 0;
	//int send_db_reserval_backward_pkt_len = 0;
	//char  *send_db_reserval_forward_pkt = NULL;
	//char  *send_db_reserval_backward_pkt = NULL;

	/*the buf store the forward pkt*/
	int forward_pkt_len = 0;
	char  *forward_pkt_buf = NULL;
	char phonenumber[PHONE_NUMBER_LENGTH_AT_HEADER+1];
	int success = 0;

	/*the buf of common recv buf*/
	char buf_recv[MAXPACKETSIZE];
	//int mem_len = 0;
	
	/* varible for enterprise server*/
	int company_code = -1;
	char client_id[CLIENT_ID_INDEX_LENGTH_AT_HEADER+1];
	int len_client_id = 0;
	int service_type_code = -1;
	//int internal_packet_flag = -1;
	long int charge_money = 0;
	char la_serial_number[SERIAL_NUMBER_LENGTH + 1] = "00";
	char packet_header[PACKET_HEADER_LENGTH+1];
	
	/*�����ݿ�Ĳ������Ӿ��*/
	int sock_db;

	/*����绰�����ֶ�*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*�õ���ҵ��*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*�õ�ҵ��������*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*��������佻�������ֶ�*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*������������к��ֶ�*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*�õ��绰�����ֶ�*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*�õ��ͻ��˺�*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*�õ��ɷѽ��*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);
	if(0>=charge_money && 3 != company_code)//�����ƶ�0Ԫ�ɷ�
	{
		//�ɷѽ���Ϊ����
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","�ɷѽ���Ϊ����");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//ֱ�ӷ����������proxy,
	}
	
	/*��������п�ҵ������Ҫ�ж��̻����ʻ�������Ҫ�����ж�*/
	if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
	{
		/*�ж��ն��ʻ�����Ƿ��ܹ���ɱ��νɷ�*/
    	if(0 == (success = QueryWarrantExcuteAffair(client_id, len_client_id, charge_money)))
		//if(0 == (success = WarrantExcuteAffair(client_id, len_client_id, charge_money, company_code+1)))
		{
			/*for this client, no charge right*/
			/*return the ERROR CODE to client*/
			memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
			send_to_proxy_len = recv_from_proxy_len;
			success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","����Ѻ������");
		
			if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
			{
				success = 0;
				perror("error@affair.c:HandleBusinessFromProxy:send");
				exit(1);
    		}
		
			return 0;//ֱ�ӷ����������proxy,
		}
	}

	/*cut off and save packet header */
	memcpy(packet_header, recv_from_proxy_buf, PACKET_HEADER_LENGTH);
	
	send_to_company_buf = recv_from_proxy_buf + PACKET_HEADER_LENGTH;
	send_to_company_len = recv_from_proxy_len -PACKET_HEADER_LENGTH;
	
	/*���ж����ݿ��Ƿ���ã��粻���ã����ܽ������ҵ��*/
	/*Connection to database control server*/
   	sock_db = CreateSocket();
	if(1 == JudgePrimaryDatabaseServer())
   	{
   		/*primary database is alive now*/
   		success = ConnectServerOneTime(global_par.system_par.database_self_ip_address, global_par.system_par.database_data_port, sock_db);
   	}
   	else
   	{
   		/*primary database is crush now*/
   		success = ConnectServerOneTime(global_par.system_par.database_brother_ip_address, global_par.system_par.database_data_port, sock_db);
	}
   	close(sock_db);
	/*������ݿⲻ����*/
	if(-1 == success)
	{
		/*for this client, no charge right*/
		/*return the ERROR CODE to client*/
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","���ݿⲻ�ܹ��������ܽ���");
			
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
			
		return 0;//ֱ�ӷ����������proxy,
	}				
			
	/*Ҫ���͵����ŷ�����֮ǰ��ע�ᵽע�����*/
	success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
				
	  
	/*Ҫ���͵����ŷ�����֮ǰ���ı�����״̬Ϊ1*/
	success = ModifyInfoStateInArray(1);
	/*���͵����ŷ����̣��õ����ذ�*/
	if(0 == global_par.company_par_array[company_code].use_middleware)
	{
		/*��ʹ���м������Ҫ���*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 == TransmitPacketWithDirectLink(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
		{
			perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithShortlinkInAffair");
			exit(1);
		}
		memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
	}
	else
	{
		/*ʹ���м������Ҫ���*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 == TransmitPacketWithMiddleware(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
		{
			perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithMiddleware");
			exit(1);
		}
		memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
	}	
		
	/*У����Ž����̵ķ��ذ��е���Ӧ���Ƿ���ȷ*/
	if(1 == ValidRespondCodeInBackwardPacket(recv_from_company_buf, recv_from_company_len, company_code, service_type_code))
	{
		/*������Ž����̵ķ��ذ��е���Ӧ����ȷ*/
		/*respond_code is correct, need be recorded in database, add the common header to database*/
		memcpy(send_to_db_buf, packet_header, PACKET_HEADER_LENGTH);
  		memcpy(send_to_db_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, recv_from_company_len+1);
		send_to_db_len = recv_from_company_len+PACKET_HEADER_LENGTH;

		/*���ǰ����ͺ����*/
		success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, forward_pkt_buf, forward_pkt_len);
		send_to_db_len = strlen(send_to_db_buf);
				
		/*Ҫ���͵����ݿ��¼֮ǰ���ı�����״̬*/
		success = ModifyInfoStateInArray(2);
				
		/*send to database*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase");
			exit(1);
		}
		recv_from_db_buf = buf_recv;

		/*У�����ݿ�ķ��ذ��е��ڲ��ɹ����*/
		if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
		{
			/*����ɹ�����Ҫ����*/
			/*Ҫ���͵����ʴ����֮ǰ���ı�����״̬*/
			success = ModifyInfoStateInArray(3);
			/*������ֵ������*/
			success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
 
			if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
			{
				/*��������п�ҵ����Ҫ���̻������ϼ���һ��Ǯ*/
				/*���п��ɷѽ��ף����Լ�����*/
				success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
			}
			else
			{
				/*����������п�ҵ����Ҫ���̻������ϼ�һ��Ǯ*/
				/*�ɷѽ��ף����Կ�����*/
				success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);				
			}
			//if(1 != (success = SubtractMoneyFromClientAccount(client_id, len_client_id, charge_money, company_code+1)))
			//{
				/*subtruct the account is error!!!*/
			//	perror("error@affair.c:HandleBusinessFromProxy:SubtractMoneyFromClientAccount");
			//}
		}
			
		/*�������ݿ�ķ��ذ��������*/
		memcpy(send_to_proxy_buf, recv_from_db_buf, strlen(recv_from_db_buf));
		send_to_proxy_len = recv_from_db_len;
#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
		/*Ҫ���͵������֮ǰ���ı�����״̬*/
		success = ModifyInfoStateInArray(4);
			
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send db feedback to proxy");
			exit(1);
   		}

		/*������Ʊ���ݿ��¼*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketInvoiceDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len, 0)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketInvoiceDatabase");
			exit(1);
		}
		success = 1;
		return success;				
	}
	else
	{
		/*������Ž����̵ķ��ذ��е���Ӧ�����ֱ�ӷ��ظ������*/
		/*in this case, the respond code of enterprise feedback
		packet is not "00", return the feedback packet (from Enterprise) to proxy*/
		memcpy(send_to_proxy_buf, packet_header, PACKET_HEADER_LENGTH);
		memcpy(send_to_proxy_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, recv_from_company_len+1);
		send_to_proxy_len = recv_from_company_len+PACKET_HEADER_LENGTH;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "01","���ŷ����̽���ʧ��");

#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send company feedback to proxy");
			exit(1);
		}
		success = -1;
				
		return success;				
	}
		
}


/*************************************************************************
 *  \brief
 *    �����������(�ⲿ��)
 *
 *  1. recieve the packet
 *  2. return the processed packet to sender
 *
 *  \par Input:
 *    sock_proxy: the socket accept connection from proxy, 
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/ 
int HandleReveralBusinessPacket(int sock_proxy, char *pkt_from_proxy)
{
	/*vaiables for recv and send packet by sockt*/
	/*from and to proxy*/
	int recv_from_proxy_len = 0;
	int send_to_proxy_len = 0;
	char recv_from_proxy_buf[MAXPACKETSIZE];
	char  send_to_proxy_buf[MAXPACKETSIZE];
	/*from and to company*/
	int recv_from_company_len = 0;
	int send_to_company_len = 0;
	char  recv_from_company_buf[MAXPACKETSIZE];
	char  *send_to_company_buf = NULL;
	/*from and to database*/
	int recv_from_db_len = 0;
	int send_to_db_len = 0;
	char  *recv_from_db_buf = NULL;
	char  send_to_db_buf[MAXPACKETSIZE];
	/*from and to database of forward packet*/
	int recv_from_db_forward_pkt_len = 0;
	int send_to_db_forward_pkt_len = 0;
	char  *recv_from_db_forward_pkt_buf = NULL;
	char  send_to_db_forward_pkt_buf[MAXPACKETSIZE];
       /*���������ݿ��ǰ����ͺ����*/
	//int send_db_reserval_forward_pkt_len = 0;
	//int send_db_reserval_backward_pkt_len = 0;
	//char  *send_db_reserval_forward_pkt = NULL;
	//char  *send_db_reserval_backward_pkt = NULL;

	/*the buf store the forward pkt*/
	int forward_pkt_len = 0;
	char  *forward_pkt_buf = NULL;
	char phonenumber[PHONE_NUMBER_LENGTH_AT_HEADER+1];
	int success = 0;

	/*the buf of common recv buf*/
	char buf_recv[MAXPACKETSIZE];
	//int mem_len = 0;
	
	/* varible for enterprise server*/
	int company_code = -1;
	char client_id[CLIENT_ID_INDEX_LENGTH_AT_HEADER+1];
	int len_client_id = 0;
	int service_type_code = -1;
	//int internal_packet_flag = -1;
	long int charge_money = 0;
	char la_serial_number[SERIAL_NUMBER_LENGTH + 1] = "00";
	//char packet_header[PACKET_HEADER_LENGTH+1];
	
	/*�����ݿ�Ĳ������Ӿ��*/
	//int sock_db;
	/*����绰�����ֶ�*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	bzero(send_to_db_forward_pkt_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*�õ���ҵ��*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*�õ�ҵ��������*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*��������佻�������ֶ�*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*������������к��ֶ�*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*�õ��绰�����ֶ�*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*�õ��ͻ��˺�*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*�õ��ɷѽ��*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);
	if(0>=charge_money && 3 != company_code)
	{
		//��������Ϊ����
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","��������Ϊ����");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//ֱ�ӷ����������proxy,
	}

	/*������ǰ��������ڲ���Ǹ�Ϊ"03"���������ݿ⣬�����ݿ���鷵���ĺϷ���*/
	/*it is a reversal foward packet, change the internal flag to "03" and send to database*/
	send_to_db_forward_pkt_len = recv_from_proxy_len;
	memcpy(send_to_db_forward_pkt_buf, recv_from_proxy_buf, recv_from_proxy_len+1);
	/*�ı��ڲ����Ϊ"03"*/
	success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "03");

	/*��ǰ������*/
	success = CompactOnePacketInDatabasePacket(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len);
	send_to_db_forward_pkt_len = strlen(send_to_db_forward_pkt_buf);
			
	/*�������ݿ�Ĳ�ѯ�˿�*/
	bzero(buf_recv, MAXPACKETSIZE);			
	//if(0 >= (success = SendPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	if(0 >= (success = SendNonsignificantPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase(Internal Packet)");
		exit(1);
	}
	recv_from_db_forward_pkt_buf = buf_recv;
				
	/*У�����ݿ�ķ��ؿ��Ƿ��ܹ�����*/
	if(1 != (success = ValidInternalSuccessFlagInHeader(recv_from_db_forward_pkt_buf, recv_from_db_forward_pkt_len)))
	{	
		/*��������*/
		/*not permit "reversal" operation, send the feedback packet from database to proxy and return*/
		send_to_proxy_len = recv_from_db_forward_pkt_len;
		memcpy(send_to_proxy_buf, recv_from_db_forward_pkt_buf, strlen(recv_from_db_forward_pkt_buf));
			
#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send databse forward reversal feedback to proxy");
			exit(1);
   		}
			
		return -1;
	}
	else
	{
		/*������*/
		/*Ҫ���͵����ݿ���з���֮ǰ��ע�ᵽע�����*/
		success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
	  
		/*Ҫ���͵����ݿ�֮ǰ���ı�����״̬Ϊ2*/
		success = ModifyInfoStateInArray(2);
		
		/*�����ݿ�ķ��ذ��еõ��ն˺ţ���ʱΪ�������ꡱ���ƵĹ��ܣ����ڲ�ʹ����
		memcpy(recv_from_proxy_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				recv_from_db_forward_pkt_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				CLIENT_ID_INDEX_LENGTH_AT_HEADER);*/

		/*������ǰ�����ϳ�һ�����������ݿ�������¼*/
		bzero(send_to_db_buf, 	MAXPACKETSIZE);
		memcpy(send_to_db_buf, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = recv_from_proxy_len;

		/*compact the database pkt*/
		success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = strlen(send_to_db_buf);
		printf("\n\n\nsend_to_db|%s|\n\n\n", send_to_db_buf);
		/*ֱ�ӽ��������׼�¼�����ݿ��У�֮���ٽ����������͸����ŷ�����*/
				
		/*�������ݿ��¼���ν���*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase");
			exit(1);
		}
		recv_from_db_buf = buf_recv;

		/*У�����ݿ��¼�������Ƿ�ɹ�*/
		if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
		{
			/*����ɹ�����Ϊ�Ƿ�����������Ҫ��������*/
			/*Ҫ���͵���ֵ������֮ǰ���ı�����״̬Ϊ3*/
			success = ModifyInfoStateInArray(3);
			
			/*������ֵ������*/
			success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
			if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
			{
				/*��������п�ҵ����Ҫ���̻������ϼ�һ��Ǯ*/
				/*���п��������ף����Լ�����*/
				success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);
			}
			else
			{
				/*����������п�ҵ����Ҫ���̻������ϼ�һ��Ǯ*/
				/*�������ף����Լ�����*/								
				success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
			}
			/*add money to accounts*/
			//if(1 != (success = AddMoneyToClientAccount(client_id, len_client_id, charge_money, company_code+1)))
			//{
				/*add the account is error!!!*/
			//	perror("error@affair.c:HandleBusinessFromProxy:AddMoneyToClientAccount");
			//}
		}
			
		/*���������󴫸����ŷ�����*/
		/*Ҫ���͵����ŷ�����֮ǰ���ı�����״̬Ϊ1*/
		success = ModifyInfoStateInArray(1);
		send_to_company_buf = recv_from_proxy_buf + PACKET_HEADER_LENGTH;
		send_to_company_len = recv_from_proxy_len -PACKET_HEADER_LENGTH;
				
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 == global_par.company_par_array[company_code].use_middleware)
		{
			/*not use middleware*/
			bzero(buf_recv, MAXPACKETSIZE);
			if(0 == TransmitPacketWithDirectLink(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
			{
				perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithShortlinkInAffair");
				exit(1);
			}
			memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
		}
		else
		{
			bzero(buf_recv, MAXPACKETSIZE);
			if(0 == TransmitPacketWithMiddleware(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
			{
				perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithMiddleware");
				exit(1);
			}
			memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
			
		}	

		/*���͸������*/
		/*Ҫ���͵����������֮ǰ���ı�����״̬Ϊ4*/
		success = ModifyInfoStateInArray(4);
		bzero(send_to_proxy_buf, MAXPACKETSIZE);
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, PACKET_HEADER_LENGTH);
		memcpy(send_to_proxy_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, strlen(recv_from_company_buf));
		send_to_proxy_len = strlen(send_to_proxy_buf);

#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
				
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send db feedback to proxy");
			exit(1);
   		}
		/*������Ʊ���ݿ��¼*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketInvoiceDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len, 0)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketInvoiceDatabase");
			exit(1);
		}
			
		success = 1;
		return success;				
				
	}
	
}

/*************************************************************************
 *  \brief
 *    �����ѯ��������ֻд����ʱ���ݿ�İ�(�ⲿ��)
 *
 *  1. recieve the packet
 *  2. return the processed packet to sender
 *
 *  \par Input:
 *    sock_proxy: the socket accept connection from proxy, 
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/ 
int HandlePreviousFeeBusinessPacket(int sock_proxy, char *pkt_from_proxy)
{
	/*vaiables for recv and send packet by sockt*/
	/*from and to proxy*/
	int recv_from_proxy_len = 0;
	int send_to_proxy_len = 0;
	char recv_from_proxy_buf[MAXPACKETSIZE];
	char  *send_to_proxy_buf = NULL;
	/*from and to company*/
	int recv_from_company_len = 0;
	int send_to_company_len = 0;
	char  recv_from_company_buf[MAXPACKETSIZE];
	char  *send_to_company_buf = NULL;
	/*from and to database*/
	int recv_from_db_len = 0;
	int send_to_db_len = 0;
	char  *recv_from_db_buf = NULL;
	char  send_to_db_buf[MAXPACKETSIZE];

	/*the buf store the forward pkt*/
	int forward_pkt_len = 0;
	char  *forward_pkt_buf = NULL;
	char phonenumber[PHONE_NUMBER_LENGTH_AT_HEADER+1];
	int success = 0;

	/*the buf of common recv buf*/
	char buf_recv[MAXPACKETSIZE];
	//int mem_len = 0;
	
	/* varible for enterprise server*/
	int company_code = -1;
	//char client_id[CLIENT_ID_INDEX_LENGTH_AT_HEADER+1];
	//int len_client_id = 0;
	int service_type_code = -1;
	//int internal_packet_flag = -1;
	//unsigned long int charge_money = 0;
	char la_serial_number[SERIAL_NUMBER_LENGTH + 1] = "00";
	char packet_header[PACKET_HEADER_LENGTH+1];
	
	/*�����ݿ�Ĳ������Ӿ��*/
	//int sock_db;
	/*����绰�����ֶ�*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*�õ���ҵ��*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*�õ�ҵ��������*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*��������佻�������ֶ�*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*������������к��ֶ�*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*�õ��绰�����ֶ�*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);

	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;

	/*���͵����ŷ����̣��õ����ذ�*/
	send_to_company_buf = recv_from_proxy_buf + PACKET_HEADER_LENGTH;
	
	memcpy(packet_header, recv_from_proxy_buf, PACKET_HEADER_LENGTH);

	send_to_company_len = recv_from_proxy_len -PACKET_HEADER_LENGTH;

	if(0 == global_par.company_par_array[company_code].use_middleware)
	{
		/*��ʹ���м������Ҫ���*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 == TransmitPacketWithDirectLink(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
		{
			perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithShortlinkInAffair");
			exit(1);
		}
		memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
	}
	else
	{
		/*ʹ���м������Ҫ���*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 == TransmitPacketWithMiddleware(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
		{
			perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithMiddleware");
			exit(1);
		}
		memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
	}	
	//printf("recv_comp_buf:|%s|\n", recv_from_company_buf);

	/*�����ذ���ǰ����������ݿ⣬ʹ���ݿ������ʱ���ݱ�*/
	bzero(send_to_db_buf, MAXPACKETSIZE);
	memcpy(send_to_db_buf, packet_header, PACKET_HEADER_LENGTH);
	memcpy(send_to_db_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, recv_from_company_len+1);
	send_to_db_len = recv_from_company_len+PACKET_HEADER_LENGTH;
			
	/*compact the database pkt*/
	success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, forward_pkt_buf, forward_pkt_len);
	send_to_db_len = strlen(send_to_db_buf);
	/*send to database*/
	//printf("Send to database query previous!!!!\n");
	//printf("send_pkt_db:|%s|, send_len = %d, recv_len = %d\n", send_to_db_buf, send_to_db_len, recv_from_db_len);
	bzero(buf_recv, MAXPACKETSIZE);
	if(0 >= (success = SendPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase (important level = 11)");
		exit(1);
	}
	recv_from_db_buf =buf_recv;

	/*�����ݿ�ķ��ذ����������*/
	send_to_proxy_buf = recv_from_db_buf;
	send_to_proxy_len = recv_from_db_len;
			
#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
	fflush(NULL);
	printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
	printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
	fflush(NULL);
#endif
	if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
	{
		success = 0;
		perror("error@affair.c:HandleBusinessFromProxy:send company feedback to proxy (important level = 11)");
		exit(1);
   	}
	/*������Ʊ���ݿ��¼*/
	bzero(buf_recv, MAXPACKETSIZE);
	if(0 >= (success = SendPacketInvoiceDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len, 0)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketInvoiceDatabase");
		exit(1);
	}
	success = 1;			
	return success;				
	
}


/*************************************************************************
 *  \brief
 *    �����²�ѯ���಻��¼�����ݿ�İ�(�ⲿ��)
 *
 *  1. recieve the packet
 *  2. return the processed packet to sender
 *
 *  \par Input:
 *    sock_proxy: the socket accept connection from proxy, 
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/ 
int HandleQueryFeeBusinessPacket(int sock_proxy, char *pkt_from_proxy)
{
	/*vaiables for recv and send packet by sockt*/
	/*from and to proxy*/
	int recv_from_proxy_len = 0;
	int send_to_proxy_len = 0;
	char recv_from_proxy_buf[MAXPACKETSIZE];
	char  send_to_proxy_buf[MAXPACKETSIZE];
	/*from and to company*/
	int recv_from_company_len = 0;
	int send_to_company_len = 0;
	char  recv_from_company_buf[MAXPACKETSIZE];
	char  *send_to_company_buf = NULL;

	/*the buf store the forward pkt*/
	int forward_pkt_len = 0;
	char  *forward_pkt_buf = NULL;
	char phonenumber[PHONE_NUMBER_LENGTH_AT_HEADER+1];
	int success = 0;

	/*the buf of common recv buf*/
	char buf_recv[MAXPACKETSIZE];
	//int mem_len = 0;
	
	/* varible for enterprise server*/
	int company_code = -1;
	//char client_id[CLIENT_ID_INDEX_LENGTH_AT_HEADER+1];
	//int len_client_id = 0;
	int service_type_code = -1;
	//int internal_packet_flag = -1;
	//unsigned long int charge_money = 0;
	char la_serial_number[SERIAL_NUMBER_LENGTH + 1] = "00";
	//char packet_header[PACKET_HEADER_LENGTH+1];
	
	/*�����ݿ�Ĳ������Ӿ��*/
	//int sock_db;
	/*����绰�����ֶ�*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);

	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
		
	/*�õ���ҵ��*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*�õ�ҵ��������*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*��������佻�������ֶ�*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*������������к��ֶ�*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*�õ��绰�����ֶ�*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	//printf("pkt_sub_function:|%s|\n",recv_from_proxy_buf);
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;

	/*���͵����ŷ����̣��õ����ذ�*/
	send_to_company_buf = recv_from_proxy_buf + PACKET_HEADER_LENGTH;
	send_to_company_len = recv_from_proxy_len -PACKET_HEADER_LENGTH;

	if(0 == global_par.company_par_array[company_code].use_middleware)
	{
		/*��ʹ���м������Ҫ���*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 == TransmitPacketWithDirectLink(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
		{
			perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithShortlinkInAffair");
			exit(1);
		}
		memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
	}
	else
	{
		/*ʹ���м������Ҫ���*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 == TransmitPacketWithMiddleware(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
		{
			perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithMiddleware");
			exit(1);
		}
		memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
	}	

	/*�����Ź�˾�ķ��ذ����������*/
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	memcpy(send_to_proxy_buf, recv_from_proxy_buf, PACKET_HEADER_LENGTH);
	memcpy(send_to_proxy_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, strlen(recv_from_company_buf));
	send_to_proxy_len = strlen(send_to_proxy_buf);
			
#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
	fflush(NULL);
	printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
	printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
	fflush(NULL);
#endif
	if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
	{
		success = 0;
		perror("error@affair.c:HandleBusinessFromProxy:send company feedback to proxy (important level = 11)");
		exit(1);
   	}
	success = 1;
			
	return success;				
	
}


/*************************************************************************
 *  \brief
 *    �����ڲ������
 *
 *  1. recieve the packet
 *  2. return the processed packet to sender
 *
 *  \par Input:
 *    sock_proxy: the socket accept connection from proxy, 
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/ 
int HandleInternalBusinessPacket(int sock_proxy, char *pkt_from_proxy)
{
	/*vaiables for recv and send packet by sockt*/
	/*from and to proxy*/
	int recv_from_proxy_len = 0;
	int send_to_proxy_len = 0;
	char recv_from_proxy_buf[MAXPACKETSIZE];
	char  *send_to_proxy_buf = NULL;
	/*from and to company*/
	//int recv_from_company_len = 0;
	//int send_to_company_len = 0;
	//char  *recv_from_company_buf = NULL;
	//char  *send_to_company_buf = NULL;
	/*from and to database*/
	int recv_from_db_len = 0;
	int send_to_db_len = 0;
	char  *recv_from_db_buf = NULL;
	char  *send_to_db_buf = NULL;

	/*the buf store the forward pkt*/
	//int forward_pkt_len = 0;
	//char  *forward_pkt_buf = NULL;
	char phonenumber[PHONE_NUMBER_LENGTH_AT_HEADER+1];
	int success = 0;

	/*the buf of common recv buf*/
	char buf_recv[MAXPACKETSIZE];
	//int mem_len = 0;
	
	/* varible for enterprise server*/
	//int company_code = -1;
	//char client_id[CLIENT_ID_INDEX_LENGTH_AT_HEADER+1];
	//int len_client_id = 0;
	//int service_type_code = -1;
	//int internal_packet_flag = -1;
	//unsigned long int charge_money = 0;
	//char la_serial_number[SERIAL_NUMBER_LENGTH + 1] = "00";
	//char packet_header[PACKET_HEADER_LENGTH+1];
	
	/*�����ݿ�Ĳ������Ӿ��*/
	//int sock_db;
	/*����绰�����ֶ�*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*ֱ�ӽ����������ݿ�Ĳ�ѯ�˿�*/
	send_to_db_len = recv_from_proxy_len;
	send_to_db_buf = recv_from_proxy_buf;

	success = CompactOnePacketInDatabasePacket(send_to_db_buf, send_to_db_len);
	send_to_db_len = strlen(send_to_db_buf);

	/*send to database*/
	bzero(buf_recv, MAXPACKETSIZE);
	if(0 >= (success = SendNonsignificantPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase(Internal Packet)");
		exit(1);
	}
	recv_from_db_buf = buf_recv;
	/*�������ݿ�Ĵ������������*/
	send_to_proxy_buf = recv_from_db_buf;
	send_to_proxy_len = recv_from_db_len;
	
#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
	fflush(NULL);
	printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
	printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
	fflush(NULL);
#endif
	if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
	{
		success = 0;
		perror("error@affair.c:HandleBusinessFromProxy:send db feedback to proxy(Internal Packet)");
		exit(1);
	}
	success = 1;
	return success;
	
}


/*************************************************************************
 *  \brief
 *    ����Ʊ��ѯ�����
 *
 *  1. recieve the packet
 *  2. return the processed packet to sender
 *
 *  \par Input:
 *    sock_proxy: the socket accept connection from proxy, 
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/ 
int HandleInvoiceQueryBusinessPacket(int sock_proxy, char *pkt_from_proxy)
{
	/*vaiables for recv and send packet by sockt*/
	/*from and to proxy*/
	int recv_from_proxy_len = 0;
	int send_to_proxy_len = 0;
	char recv_from_proxy_buf[MAXPACKETSIZE];
	char  *send_to_proxy_buf = NULL;
	/*from and to company*/
	//int recv_from_company_len = 0;
	//int send_to_company_len = 0;
	//char  *recv_from_company_buf = NULL;
	//char  *send_to_company_buf = NULL;
	/*from and to database*/
	int recv_from_db_len = 0;
	int send_to_db_len = 0;
	char  *recv_from_db_buf = NULL;
	char  *send_to_db_buf = NULL;

	/*the buf store the forward pkt*/
	//int forward_pkt_len = 0;
	//char  *forward_pkt_buf = NULL;
	char phonenumber[PHONE_NUMBER_LENGTH_AT_HEADER+1];
	int success = 0;

	/*the buf of common recv buf*/
	char buf_recv[MAXPACKETSIZE];
	//int mem_len = 0;
	
	/* varible for enterprise server*/
	//int company_code = -1;
	//char client_id[CLIENT_ID_INDEX_LENGTH_AT_HEADER+1];
	//int len_client_id = 0;
	//int service_type_code = -1;
	//int internal_packet_flag = -1;
	//unsigned long int charge_money = 0;
	//char la_serial_number[SERIAL_NUMBER_LENGTH + 1] = "00";
	//char packet_header[PACKET_HEADER_LENGTH+1];
	
	/*�����ݿ�Ĳ������Ӿ��*/
	//int sock_db;
	/*����绰�����ֶ�*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*ֱ�ӽ����������ݿ�Ĳ�ѯ�˿�*/
	send_to_db_len = recv_from_proxy_len;
	send_to_db_buf = recv_from_proxy_buf;

	success = CompactOnePacketInDatabasePacket(send_to_db_buf, send_to_db_len);
	send_to_db_len = strlen(send_to_db_buf);

	/*������Ʊ���ݿ�ȡ��Ʊ����*/
	bzero(buf_recv, MAXPACKETSIZE);
	if(0 >= (success = SendPacketInvoiceDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len, 1)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketInvoiceDatabase");
		exit(1);
	}
	recv_from_db_buf = buf_recv;
	/*�������ݿ�Ĵ������������*/
	send_to_proxy_buf = recv_from_db_buf;
	send_to_proxy_len = recv_from_db_len;
	
#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
	fflush(NULL);
	printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
	printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
	fflush(NULL);
#endif
	if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
	{
		success = 0;
		perror("error@affair.c:HandleBusinessFromProxy:send db feedback to proxy(Internal Packet)");
		exit(1);
	}
	success = 1;
	return success;
	
}

/*************************************************************************
 *  \brief
 *    �����ڻ��ɷ������������ֻ���˵Ĺ��ܣ����磬����Ʊ��ҵ��
 *
 *  1. recieve the packet
 *  2. return the processed packet to sender
 *
 *  \par Input:
 *    sock_proxy: the socket accept connection from proxy, 
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/ 
int HandleLoopChargeBusinessPacket(int sock_proxy, char *pkt_from_proxy)
{
	/*vaiables for recv and send packet by sockt*/
	/*from and to proxy*/
	int recv_from_proxy_len = 0;
	int send_to_proxy_len = 0;
	char recv_from_proxy_buf[MAXPACKETSIZE];
	char  send_to_proxy_buf[MAXPACKETSIZE];
	/*from and to company*/
	//int recv_from_company_len = 0;
	//int send_to_company_len = 0;
	//char  *recv_from_company_buf = NULL;
	//char  *send_to_company_buf = NULL;
	/*from and to database*/
	int recv_from_db_len = 0;
	int send_to_db_len = 0;
	char  *recv_from_db_buf = NULL;
	char  send_to_db_buf[MAXPACKETSIZE];
	/*from and to database of forward packet*/
	//int recv_from_db_forward_pkt_len = 0;
	//int send_to_db_forward_pkt_len = 0;
	//char  *recv_from_db_forward_pkt_buf = NULL;
	//char  *send_to_db_forward_pkt_buf = NULL;

	/*the buf store the forward pkt*/
	int forward_pkt_len = 0;
	char  *forward_pkt_buf = NULL;
	char phonenumber[PHONE_NUMBER_LENGTH_AT_HEADER+1];
	int success = 0;

	/*the buf of common recv buf*/
	char buf_recv[MAXPACKETSIZE];
	//int mem_len = 0;
	
	/* varible for enterprise server*/
	int company_code = -1;
	char client_id[CLIENT_ID_INDEX_LENGTH_AT_HEADER+1];
	int len_client_id = 0;
	int service_type_code = -1;
	//int internal_packet_flag = -1;
	long int charge_money = 0;
	char la_serial_number[SERIAL_NUMBER_LENGTH + 1] = "00";
	char packet_header[PACKET_HEADER_LENGTH+1];
	
	/*�����ݿ�Ĳ������Ӿ��*/
	//int sock_db;
	/*����绰�����ֶ�*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);

	/*���ڲ�����Ǹ�Ϊ"00"*/	
	//success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "00");
	success = SetInternalPacketFlagInHeader(recv_from_proxy_buf, recv_from_proxy_len, "00");
		
	/*�õ���ҵ��*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*�õ�ҵ��������*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*��������佻�������ֶ�*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*������������к��ֶ�*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*�õ��绰�����ֶ�*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*�õ��ͻ��˺�*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*�õ��ɷѽ��*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);

	if(0>charge_money)
	{
		//�ɷѽ���Ϊ����
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","�ɷѽ���Ϊ����");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//ֱ�ӷ����������proxy,
	}

	/*��������п�ҵ������Ҫ�ж��̻����ʻ�������Ҫ�����ж�*/
	if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
	{
		/*�ж��ն��ʻ�����Ƿ��ܹ���ɱ��νɷ�*/
    	if(0 == (success = QueryWarrantExcuteAffair(client_id, len_client_id, charge_money)))
		//if(0 == (success = WarrantExcuteAffair(client_id, len_client_id, charge_money, company_code+1)))
		{
			/*for this client, no charge right*/
			/*return the ERROR CODE to client*/
			memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
			send_to_proxy_len = recv_from_proxy_len;
			success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","����Ѻ������");
		
			if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
			{
				success = 0;
				perror("error@affair.c:HandleBusinessFromProxy:send");
				exit(1);
    		}
		
			return 0;//ֱ�ӷ����������proxy,
		}
	}
	
	/*cut off and save packet header */
	memcpy(packet_header, recv_from_proxy_buf, PACKET_HEADER_LENGTH);
	
	/*Ҫ���͵����ݿ�֮ǰ��ע�ᵽע�����*/
	success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
				
	  
	/*respond_code is correct, need be recorded in database, add the common header to database*/
	bzero(send_to_db_buf, MAXPACKETSIZE);
	memcpy(send_to_db_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
	send_to_db_len = strlen(send_to_db_buf);
	/*���ǰ����ͺ����,��������ͬ��*/
	success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, forward_pkt_buf, forward_pkt_len);
	send_to_db_len = strlen(send_to_db_buf);
				
	/*Ҫ���͵����ݿ��¼֮ǰ���ı�����״̬*/
	success = ModifyInfoStateInArray(2);
				
	/*send to database*/
	bzero(buf_recv, MAXPACKETSIZE);
	if(0 >= (success = SendPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase");
		exit(1);
	}
	recv_from_db_buf = buf_recv;

	/*У�����ݿ�ķ��ذ��е��ڲ��ɹ����*/
	if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
	{
		/*����ɹ�����Ҫ����*/
		/*Ҫ���͵����ʴ����֮ǰ���ı�����״̬*/
		success = ModifyInfoStateInArray(3);
		/*������ֵ������*/
		success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
 		if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
		{
			/*��������п�ҵ����Ҫ���̻������ϼ���һ��Ǯ*/
			/*���п��ɷѽ��ף����Լ�����*/
			success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
		}
		else
		{
			/*����������п�ҵ����Ҫ���̻������ϼ�һ��Ǯ*/
			/*�ɷѽ��ף����Կ�����*/
			success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);				
		}
		//if(1 != (success = SubtractMoneyFromClientAccount(client_id, len_client_id, charge_money, company_code+1)))
		//{
			/*subtruct the account is error!!!*/
		//	perror("error@affair.c:HandleBusinessFromProxy:SubtractMoneyFromClientAccount");
		//}
	}
			
	/*�������ݿ�ķ��ذ��������*/
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	memcpy(send_to_proxy_buf, recv_from_db_buf, recv_from_db_len);
	//send_to_proxy_buf = recv_from_db_buf;
	send_to_proxy_len = recv_from_db_len;
#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
	fflush(NULL);
	printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
	printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
	fflush(NULL);
#endif
	/*Ҫ���͵������֮ǰ���ı�����״̬*/
	success = ModifyInfoStateInArray(4);
			
	if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
	{
		success = 0;
		perror("error@affair.c:HandleBusinessFromProxy:send db feedback to proxy");
		exit(1);
 	}
	/*������Ʊ���ݿ��¼*/
	bzero(buf_recv, MAXPACKETSIZE);
	if(0 >= (success = SendPacketInvoiceDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len, 0)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketInvoiceDatabase");
		exit(1);
	}

	success = 1;
				
	return success;				
		
}


/*************************************************************************
 *  \brief
 *    �����ڻ����������������ֻ���˵Ĺ��ܣ����磬ȡ������Ʊ��ҵ��
 *
 *  1. recieve the packet
 *  2. return the processed packet to sender
 *
 *  \par Input:
 *    sock_proxy: the socket accept connection from proxy, 
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/ 
int HandleLoopReveralBusinessPacket(int sock_proxy, char *pkt_from_proxy)
{
	/*vaiables for recv and send packet by sockt*/
	/*from and to proxy*/
	int recv_from_proxy_len = 0;
	int send_to_proxy_len = 0;
	char recv_from_proxy_buf[MAXPACKETSIZE];
	char  send_to_proxy_buf[MAXPACKETSIZE];
	/*from and to company*/
	//int recv_from_company_len = 0;
	//int send_to_company_len = 0;
	char  recv_from_company_buf[MAXPACKETSIZE];
	//char  *send_to_company_buf = NULL;
	/*from and to database*/
	int recv_from_db_len = 0;
	int send_to_db_len = 0;
	char  *recv_from_db_buf = NULL;
	char  send_to_db_buf[MAXPACKETSIZE];
	/*from and to database of forward packet*/
	int recv_from_db_forward_pkt_len = 0;
	int send_to_db_forward_pkt_len = 0;
	char  *recv_from_db_forward_pkt_buf = NULL;
	char  send_to_db_forward_pkt_buf[MAXPACKETSIZE];
       /*���������ݿ��ǰ����ͺ����*/
	//int send_db_reserval_forward_pkt_len = 0;
	//int send_db_reserval_backward_pkt_len = 0;
	//char  *send_db_reserval_forward_pkt = NULL;
	//char  *send_db_reserval_backward_pkt = NULL;

	/*the buf store the forward pkt*/
	int forward_pkt_len = 0;
	char  *forward_pkt_buf = NULL;
	char phonenumber[PHONE_NUMBER_LENGTH_AT_HEADER+1];
	int success = 0;

	/*the buf of common recv buf*/
	char buf_recv[MAXPACKETSIZE];
	//int mem_len = 0;
	
	/* varible for enterprise server*/
	int company_code = -1;
	char client_id[CLIENT_ID_INDEX_LENGTH_AT_HEADER+1];
	int len_client_id = 0;
	int service_type_code = -1;
	//int internal_packet_flag = -1;
	long int charge_money = 0;
	char la_serial_number[SERIAL_NUMBER_LENGTH + 1] = "00";
	//char packet_header[PACKET_HEADER_LENGTH+1];
	
	/*�����ݿ�Ĳ������Ӿ��*/
	//int sock_db;
	/*����绰�����ֶ�*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	bzero(send_to_db_forward_pkt_buf, MAXPACKETSIZE);

	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*���ڲ�����Ǹ�Ϊ"00"*/	
	//success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "00");
	success = SetInternalPacketFlagInHeader(recv_from_proxy_buf, recv_from_proxy_len, "00");
		
	/*�õ���ҵ��*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*�õ�ҵ��������*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*��������佻�������ֶ�*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*������������к��ֶ�*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}

	/*�õ��绰�����ֶ�*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
		
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*�õ��ͻ��˺�*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*�õ��ɷѽ��*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);

	if(0>charge_money)
	{
		//��������Ϊ����
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","��������Ϊ����");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//ֱ�ӷ����������proxy,
	}
	
	/*������ǰ��������ڲ���Ǹ�Ϊ"03"���������ݿ⣬�����ݿ���鷵���ĺϷ���*/
	/*it is a reversal foward packet, change the internal flag to "03" and send to database*/
	send_to_db_forward_pkt_len = recv_from_proxy_len;
	memcpy(send_to_db_forward_pkt_buf, recv_from_proxy_buf, recv_from_proxy_len+1);
	/*�ı��ڲ����Ϊ"03"*/
	success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "03");
	
	/*��ǰ������*/
	success = CompactOnePacketInDatabasePacket(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len);
	send_to_db_forward_pkt_len = strlen(send_to_db_forward_pkt_buf);
				
	/*�������ݿ�Ĳ�ѯ�˿�*/
	bzero(buf_recv, MAXPACKETSIZE); 		
	//if(0 >= (success = SendPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	if(0 >= (success = SendNonsignificantPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase(Internal Packet)");
		exit(1);
	}
	recv_from_db_forward_pkt_buf = buf_recv;
					
	/*У�����ݿ�ķ��ؿ��Ƿ��ܹ�����*/
	if(1 != (success = ValidInternalSuccessFlagInHeader(recv_from_db_forward_pkt_buf, recv_from_db_forward_pkt_len)))
	{	
		/*��������*/
		/*not permit "reversal" operation, send the feedback packet from database to proxy and return*/
		send_to_proxy_len = recv_from_db_forward_pkt_len;
		memcpy(send_to_proxy_buf, recv_from_db_forward_pkt_buf, strlen(recv_from_db_forward_pkt_buf));
				
#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send databse forward reversal feedback to proxy");
			exit(1);
		}
				
		return -1;
	}
	else
	{
		/*������*/
		/*Ҫ���͵����ݿ���з���֮ǰ��ע�ᵽע�����*/
		success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
		  
		/*Ҫ���͵����ݿ�֮ǰ���ı�����״̬Ϊ2*/
		success = ModifyInfoStateInArray(2);
			
		/*�����ݿ�ķ��ذ��еõ��ն˺ţ���ʱΪ�������ꡱ���ƵĹ��ܣ����ڲ�ʹ����
		memcpy(recv_from_proxy_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				recv_from_db_forward_pkt_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				CLIENT_ID_INDEX_LENGTH_AT_HEADER);*/
	
		/*������ǰ�����ϳ�һ�����������ݿ�������¼*/
		bzero(send_to_db_buf,	MAXPACKETSIZE);
		memcpy(send_to_db_buf, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = recv_from_proxy_len;
	
		/*compact the database pkt*/
		success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = strlen(send_to_db_buf);
	
		/*ֱ�ӽ��������׼�¼�����ݿ��У�֮���ٽ����������͸����ŷ�����*/
					
		/*�������ݿ��¼���ν���*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase");
			exit(1);
		}
		recv_from_db_buf = buf_recv;
	
		/*У�����ݿ��¼�������Ƿ�ɹ�*/
		if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
		{
			/*����ɹ�����Ϊ�Ƿ�����������Ҫ��������*/
			/*Ҫ���͵���ֵ������֮ǰ���ı�����״̬Ϊ3*/
			success = ModifyInfoStateInArray(3);
				
			/*������ֵ������*/
			success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
			if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
			{
				/*��������п�ҵ����Ҫ���̻������ϼ�һ��Ǯ*/
				/*���п��������ף����Լ�����*/
				success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);
			}
			else
			{
				/*����������п�ҵ����Ҫ���̻������ϼ�һ��Ǯ*/
				/*�������ף����Լ�����*/								
				success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
			}

			/*add money to accounts*/
			//if(1 != (success = AddMoneyToClientAccount(client_id, len_client_id, charge_money, company_code+1)))
			//{
				/*add the account is error!!!*/
			//	perror("error@affair.c:HandleBusinessFromProxy:AddMoneyToClientAccount");
			//}
		}

		/*���͸������*/
		/*Ҫ���͵����������֮ǰ���ı�����״̬Ϊ4*/
		success = ModifyInfoStateInArray(4);
		//bzero(send_to_proxy_buf, MAXPACKETSIZE);
		//memcpy(send_to_proxy_buf, recv_from_proxy_buf, PACKET_HEADER_LENGTH);
		//memcpy(send_to_proxy_buf+PACKET_HEADER_LENGTH, recv_from_db_buf, strlen(recv_from_db_buf));
		//send_to_proxy_len = strlen(send_to_proxy_buf);

		bzero(send_to_proxy_buf, MAXPACKETSIZE);
		memcpy(send_to_proxy_buf, recv_from_db_buf, recv_from_db_len);
		//send_to_proxy_buf = recv_from_db_buf;
		send_to_proxy_len = recv_from_db_len;

#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
				
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send db feedback to proxy");
			exit(1);
   		}
		/*������Ʊ���ݿ��¼*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketInvoiceDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len, 0)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketInvoiceDatabase");
			exit(1);
		}
			
		success = 1;
		return success;	
	}
		
}



/*************************************************************************
 *  \brief
 *    ���������������ķ��������(�ⲿ��)�� Ҫ��������ŷ���
 *    Ȼ���ٽ��ն˵�ǰ������ظ��ͻ���
 *  1. recieve the packet
 *  2. return the processed packet to sender
 *
 *  \par Input:
 *    sock_proxy: the socket accept connection from proxy, 
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/ 
int HandleTelecomReveralBusinessPacket(int sock_proxy, char *pkt_from_proxy)
{
	/*vaiables for recv and send packet by sockt*/
	/*from and to proxy*/
	int recv_from_proxy_len = 0;
	int send_to_proxy_len = 0;
	char recv_from_proxy_buf[MAXPACKETSIZE];
	char  send_to_proxy_buf[MAXPACKETSIZE];
	/*from and to company*/
	int recv_from_company_len = 0;
	int send_to_company_len = 0;
	char  recv_from_company_buf[MAXPACKETSIZE];
	char  *send_to_company_buf = NULL;
	/*from and to database*/
	int recv_from_db_len = 0;
	int send_to_db_len = 0;
	char  *recv_from_db_buf = NULL;
	char  send_to_db_buf[MAXPACKETSIZE];
	/*from and to database of forward packet*/
	int recv_from_db_forward_pkt_len = 0;
	int send_to_db_forward_pkt_len = 0;
	char  *recv_from_db_forward_pkt_buf = NULL;
	char  send_to_db_forward_pkt_buf[MAXPACKETSIZE];
       /*���������ݿ��ǰ����ͺ����*/
	//int send_db_reserval_forward_pkt_len = 0;
	//int send_db_reserval_backward_pkt_len = 0;
	//char  *send_db_reserval_forward_pkt = NULL;
	//char  *send_db_reserval_backward_pkt = NULL;

	/*the buf store the forward pkt*/
	int forward_pkt_len = 0;
	char  *forward_pkt_buf = NULL;
	char phonenumber[PHONE_NUMBER_LENGTH_AT_HEADER+1];
	int success = 0;

	/*the buf of common recv buf*/
	char buf_recv[MAXPACKETSIZE];
	//int mem_len = 0;
	
	/* varible for enterprise server*/
	int company_code = -1;
	char client_id[CLIENT_ID_INDEX_LENGTH_AT_HEADER+1];
	int len_client_id = 0;
	int service_type_code = -1;
	//int internal_packet_flag = -1;
	long int charge_money = 0;
	char la_serial_number[SERIAL_NUMBER_LENGTH + 1] = "00";
	//char packet_header[PACKET_HEADER_LENGTH+1];
	
	/*�����ݿ�Ĳ������Ӿ��*/
	//int sock_db;
	/*����绰�����ֶ�*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	bzero(send_to_db_forward_pkt_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*�õ���ҵ��*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*�õ�ҵ��������*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*��������佻�������ֶ�*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*������������к��ֶ�*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*�õ��绰�����ֶ�*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*�õ��ͻ��˺�*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*�õ��ɷѽ��*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);
	if(0>=charge_money)
	{
		//��������Ϊ����
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","��������Ϊ����");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//ֱ�ӷ����������proxy,
	}

	/*������ǰ��������ڲ���Ǹ�Ϊ"03"���������ݿ⣬�����ݿ���鷵���ĺϷ���*/
	/*it is a reversal foward packet, change the internal flag to "03" and send to database*/
	send_to_db_forward_pkt_len = recv_from_proxy_len;
	memcpy(send_to_db_forward_pkt_buf, recv_from_proxy_buf, recv_from_proxy_len+1);
	/*�ı��ڲ����Ϊ"03"*/
	success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "03");

	/*��ǰ������*/
	success = CompactOnePacketInDatabasePacket(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len);
	send_to_db_forward_pkt_len = strlen(send_to_db_forward_pkt_buf);
			
	/*�������ݿ�Ĳ�ѯ�˿�*/
	bzero(buf_recv, MAXPACKETSIZE);			
	//if(0 >= (success = SendPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	if(0 >= (success = SendNonsignificantPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase(Internal Packet)");
		exit(1);
	}
	recv_from_db_forward_pkt_buf = buf_recv;
				
	/*У�����ݿ�ķ��ؿ��Ƿ��ܹ�����*/
	if(1 != (success = ValidInternalSuccessFlagInHeader(recv_from_db_forward_pkt_buf, recv_from_db_forward_pkt_len)))
	{	
		/*��������*/
		/*not permit "reversal" operation, send the feedback packet from database to proxy and return*/
		send_to_proxy_len = recv_from_db_forward_pkt_len;
		memcpy(send_to_proxy_buf, recv_from_db_forward_pkt_buf, strlen(recv_from_db_forward_pkt_buf));
			
#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send databse forward reversal feedback to proxy");
			exit(1);
   		}
			
		return -1;
	}
	else
	{
		/*������*/
		/*Ҫ���͵����ݿ���з���֮ǰ��ע�ᵽע�����*/
		success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
	  
		/*���������󴫸����ŷ�����*/
		/*Ҫ���͵����ŷ�����֮ǰ���ı�����״̬Ϊ1*/
		success = ModifyInfoStateInArray(1);
		send_to_company_buf = recv_from_proxy_buf + PACKET_HEADER_LENGTH;
		send_to_company_len = recv_from_proxy_len -PACKET_HEADER_LENGTH;
				
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 == global_par.company_par_array[company_code].use_middleware)
		{
			/*not use middleware*/
			bzero(buf_recv, MAXPACKETSIZE);
			if(0 == TransmitPacketWithDirectLink(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
			{
				perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithShortlinkInAffair");
				exit(1);
			}
			memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
		}
		else
		{
			bzero(buf_recv, MAXPACKETSIZE);
			if(0 == TransmitPacketWithMiddleware(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
			{
				perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithMiddleware");
				exit(1);
			}
			memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
			
		}	
		/*У����Ž����̵ķ��ذ��е���Ӧ���Ƿ���ȷ*/
		if(1 != ValidRespondCodeInBackwardPacket(recv_from_company_buf, recv_from_company_len, company_code, service_type_code))
		{
			/*������Ž����̵ķ��ذ��е���Ӧ�벻��ȷ,ֱ�ӷ��ش�����ͻ���*/
			/*���͸������*/
			/*Ҫ���͵����������֮ǰ���ı�����״̬Ϊ4*/
			success = ModifyInfoStateInArray(4);
			bzero(send_to_proxy_buf, MAXPACKETSIZE);
			memcpy(send_to_proxy_buf, recv_from_proxy_buf, PACKET_HEADER_LENGTH);
			memcpy(send_to_proxy_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, strlen(recv_from_company_buf));
			send_to_proxy_len = strlen(send_to_proxy_buf);
			success = WriteInternalSuccessFlag(send_to_proxy_buf, "04","���ŷ����̲��÷���!");

#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
				
			if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
			{
				success = 0;
				perror("error@affair.c:HandleBusinessFromProxy:send db feedback to proxy");
				exit(1);
   			}
			
			/*�����ͻ��˺�ֱ���˳��ù���*/	
			success = -1;
			return success;		
		}
		
		/*Ҫ���͵����ݿ�֮ǰ���ı�����״̬Ϊ2*/
		success = ModifyInfoStateInArray(2);
		
		/*�����ݿ�ķ��ذ��еõ��ն˺ţ���ʱΪ�������ꡱ���ƵĹ��ܣ����ڲ�ʹ����
		memcpy(recv_from_proxy_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				recv_from_db_forward_pkt_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				CLIENT_ID_INDEX_LENGTH_AT_HEADER);*/

		/*������ǰ�����ϳ�һ�����������ݿ�������¼*/
		bzero(send_to_db_buf, 	MAXPACKETSIZE);
		memcpy(send_to_db_buf, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = recv_from_proxy_len;

		/*compact the database pkt*/
		success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = strlen(send_to_db_buf);

		/*ֱ�ӽ��������׼�¼�����ݿ��У�֮���ٽ����������͸����ŷ�����*/
				
		/*�������ݿ��¼���ν���*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase");
			exit(1);
		}
		recv_from_db_buf = buf_recv;

		/*У�����ݿ��¼�������Ƿ�ɹ�*/
		if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
		{
			/*����ɹ�����Ϊ�Ƿ�����������Ҫ��������*/
			/*Ҫ���͵���ֵ������֮ǰ���ı�����״̬Ϊ3*/
			success = ModifyInfoStateInArray(3);
			
			/*������ֵ������*/
			success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
			if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
			{
				/*��������п�ҵ����Ҫ���̻������ϼ�һ��Ǯ*/
				/*���п��������ף����Լ�����*/
				success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);
			}
			else
			{
				/*����������п�ҵ����Ҫ���̻������ϼ�һ��Ǯ*/
				/*�������ף����Լ�����*/								
				success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
			}
			/*add money to accounts*/
			//if(1 != (success = AddMoneyToClientAccount(client_id, len_client_id, charge_money, company_code+1)))
			//{
				/*add the account is error!!!*/
			//	perror("error@affair.c:HandleBusinessFromProxy:AddMoneyToClientAccount");
			//}
		}
			
		/*���͸������*/
		/*Ҫ���͵����������֮ǰ���ı�����״̬Ϊ4*/
		success = ModifyInfoStateInArray(4);
		bzero(send_to_proxy_buf, MAXPACKETSIZE);
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, PACKET_HEADER_LENGTH);
		memcpy(send_to_proxy_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, strlen(recv_from_company_buf));
		send_to_proxy_len = strlen(send_to_proxy_buf);

#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
				
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send db feedback to proxy");
			exit(1);
   		}
			
		success = 1;
		return success;				
				
	}
	
}





/*************************************************************************
 *  \brief
 *    ���������������ķ��������(�ⲿ��)�� Ҫ��������ŷ���
 *    Ҫ�ǵ��ŵķ��ذ�
 *    Ȼ���ٽ��ն˵�ǰ������ظ��ͻ���
 *  1. recieve the packet
 *  2. return the processed packet to sender
 *
 *  \par Input:
 *    sock_proxy: the socket accept connection from proxy, 
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/ 
int HandleTelecomReveralBusinessPacketNew(int sock_proxy, char *pkt_from_proxy)
{
	/*vaiables for recv and send packet by sockt*/
	/*from and to proxy*/
	int recv_from_proxy_len = 0;
	int send_to_proxy_len = 0;
	char recv_from_proxy_buf[MAXPACKETSIZE];
	char  send_to_proxy_buf[MAXPACKETSIZE];
	/*from and to company*/
	int recv_from_company_len = 0;
	int send_to_company_len = 0;
	char  recv_from_company_buf[MAXPACKETSIZE];
	char  *send_to_company_buf = NULL;
	/*from and to database*/
	int recv_from_db_len = 0;
	int send_to_db_len = 0;
	char  *recv_from_db_buf = NULL;
	char  send_to_db_buf[MAXPACKETSIZE];
	/*from and to database of forward packet*/
	int recv_from_db_forward_pkt_len = 0;
	int send_to_db_forward_pkt_len = 0;
	char  *recv_from_db_forward_pkt_buf = NULL;
	char  send_to_db_forward_pkt_buf[MAXPACKETSIZE];
       /*���������ݿ��ǰ����ͺ����*/
	//int send_db_reserval_forward_pkt_len = 0;
	//int send_db_reserval_backward_pkt_len = 0;
	//char  *send_db_reserval_forward_pkt = NULL;
	//char  *send_db_reserval_backward_pkt = NULL;

	/*the buf store the forward pkt*/
	int forward_pkt_len = 0;
	char  *forward_pkt_buf = NULL;
	char phonenumber[PHONE_NUMBER_LENGTH_AT_HEADER+1];
	int success = 0;

	/*the buf of common recv buf*/
	char buf_recv[MAXPACKETSIZE];
	//int mem_len = 0;
	
	/* varible for enterprise server*/
	int company_code = -1;
	char client_id[CLIENT_ID_INDEX_LENGTH_AT_HEADER+1];
	int len_client_id = 0;
	int service_type_code = -1;
	//int internal_packet_flag = -1;
	long int charge_money = 0;
	char la_serial_number[SERIAL_NUMBER_LENGTH + 1] = "00";
	//char packet_header[PACKET_HEADER_LENGTH+1];
	
	/*�����ݿ�Ĳ������Ӿ��*/
	//int sock_db;
	/*����绰�����ֶ�*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	bzero(send_to_db_forward_pkt_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*�õ���ҵ��*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*�õ�ҵ��������*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*��������佻�������ֶ�*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*������������к��ֶ�*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*�õ��绰�����ֶ�*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*�õ��ͻ��˺�*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*�õ��ɷѽ��*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);
	if(0>=charge_money)
	{
		//��������Ϊ����
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","��������Ϊ����");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//ֱ�ӷ����������proxy,
	}

	/*������ǰ��������ڲ���Ǹ�Ϊ"03"���������ݿ⣬�����ݿ���鷵���ĺϷ���*/
	/*it is a reversal foward packet, change the internal flag to "03" and send to database*/
	send_to_db_forward_pkt_len = recv_from_proxy_len;
	memcpy(send_to_db_forward_pkt_buf, recv_from_proxy_buf, recv_from_proxy_len+1);
	/*�ı��ڲ����Ϊ"03"*/
	success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "03");

	/*��ǰ������*/
	success = CompactOnePacketInDatabasePacket(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len);
	send_to_db_forward_pkt_len = strlen(send_to_db_forward_pkt_buf);
			
	/*�������ݿ�Ĳ�ѯ�˿�*/
	bzero(buf_recv, MAXPACKETSIZE);			
	//if(0 >= (success = SendPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	if(0 >= (success = SendNonsignificantPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase(Internal Packet)");
		exit(1);
	}
	recv_from_db_forward_pkt_buf = buf_recv;
				
	/*У�����ݿ�ķ��ؿ��Ƿ��ܹ�����*/
	if(1 != (success = ValidInternalSuccessFlagInHeader(recv_from_db_forward_pkt_buf, recv_from_db_forward_pkt_len)))
	{	
		/*��������*/
		/*not permit "reversal" operation, send the feedback packet from database to proxy and return*/
		send_to_proxy_len = recv_from_db_forward_pkt_len;
		memcpy(send_to_proxy_buf, recv_from_db_forward_pkt_buf, strlen(recv_from_db_forward_pkt_buf));
			
#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send databse forward reversal feedback to proxy");
			exit(1);
   		}
			
		return -1;
	}
	else
	{
		/*������*/
		/*Ҫ���͵����ݿ���з���֮ǰ��ע�ᵽע�����*/
		success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
	  
		/*���������󴫸����ŷ�����*/
		/*Ҫ���͵����ŷ�����֮ǰ���ı�����״̬Ϊ1*/
		success = ModifyInfoStateInArray(1);
		send_to_company_buf = recv_from_proxy_buf + PACKET_HEADER_LENGTH;
		send_to_company_len = recv_from_proxy_len -PACKET_HEADER_LENGTH;
				
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 == global_par.company_par_array[company_code].use_middleware)
		{
			/*not use middleware*/
			bzero(buf_recv, MAXPACKETSIZE);
			if(0 == TransmitPacketWithDirectLink(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
			{
				perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithShortlinkInAffair");
				exit(1);
			}
			memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
		}
		else
		{
			bzero(buf_recv, MAXPACKETSIZE);
			if(0 == TransmitPacketWithMiddleware(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
			{
				perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithMiddleware");
				exit(1);
			}
			memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
			
		}	
		/*У����Ž����̵ķ��ذ��е���Ӧ���Ƿ���ȷ*/
		if(1 != ValidRespondCodeInBackwardPacket(recv_from_company_buf, recv_from_company_len, company_code, service_type_code))
		{
			/*������Ž����̵ķ��ذ��е���Ӧ�벻��ȷ,ֱ�ӷ��ش�����ͻ���*/
			/*���͸������*/
			/*Ҫ���͵����������֮ǰ���ı�����״̬Ϊ4*/
			success = ModifyInfoStateInArray(4);
			bzero(send_to_proxy_buf, MAXPACKETSIZE);
			memcpy(send_to_proxy_buf, recv_from_proxy_buf, PACKET_HEADER_LENGTH);
			memcpy(send_to_proxy_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, strlen(recv_from_company_buf));
			send_to_proxy_len = strlen(send_to_proxy_buf);
			success = WriteInternalSuccessFlag(send_to_proxy_buf, "04","���ŷ����̲��÷���!");

#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
				
			if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
			{
				success = 0;
				perror("error@affair.c:HandleBusinessFromProxy:send db feedback to proxy");
				exit(1);
   			}
			
			/*�����ͻ��˺�ֱ���˳��ù���*/	
			success = -1;
			return success;		
		}
		
		/*Ҫ���͵����ݿ�֮ǰ���ı�����״̬Ϊ2*/
		success = ModifyInfoStateInArray(2);
		
		/*�����ݿ�ķ��ذ��еõ��ն˺ţ���ʱΪ�������ꡱ���ƵĹ��ܣ����ڲ�ʹ����
		memcpy(recv_from_proxy_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				recv_from_db_forward_pkt_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				CLIENT_ID_INDEX_LENGTH_AT_HEADER);*/

		/*respond_code is correct, need be recorded in database, add the common header to database*/
		bzero(send_to_db_buf, MAXPACKETSIZE);
		//printf("forward_pkt_buf:|%s|\n", forward_pkt_buf);
		memcpy(send_to_db_buf, forward_pkt_buf, PACKET_HEADER_LENGTH);		
		//printf("recv_from_company_buf:|%s|\n", recv_from_company_buf);
  		memcpy(send_to_db_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, recv_from_company_len+1);
		//printf("send_to_db_buf:|%s|\n", send_to_db_buf);
		send_to_db_len = recv_from_company_len+PACKET_HEADER_LENGTH;

		/*���ǰ����ͺ����*/
		success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, forward_pkt_buf, forward_pkt_len);
		send_to_db_len = strlen(send_to_db_buf);

		///*������ǰ�����ϳ�һ�����������ݿ�������¼*/
		//bzero(send_to_db_buf, 	MAXPACKETSIZE);
		//memcpy(send_to_db_buf, recv_from_proxy_buf, recv_from_proxy_len);
		//send_to_db_len = recv_from_proxy_len;

		/*compact the database pkt*/
		//success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, recv_from_proxy_buf, recv_from_proxy_len);
		//send_to_db_len = strlen(send_to_db_buf);

		/*ֱ�ӽ��������׼�¼�����ݿ��У�֮���ٽ����������͸����ŷ�����*/
				
		/*�������ݿ��¼���ν���*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase");
			exit(1);
		}
		recv_from_db_buf = buf_recv;

		/*У�����ݿ��¼�������Ƿ�ɹ�*/
		if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
		{
			/*����ɹ�����Ϊ�Ƿ�����������Ҫ��������*/
			/*Ҫ���͵���ֵ������֮ǰ���ı�����״̬Ϊ3*/
			success = ModifyInfoStateInArray(3);
			
			/*������ֵ������*/
			success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
			if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
			{
				/*��������п�ҵ����Ҫ���̻������ϼ�һ��Ǯ*/
				/*���п��������ף����Լ�����*/
				success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);
			}
			else
			{
				/*����������п�ҵ����Ҫ���̻������ϼ�һ��Ǯ*/
				/*�������ף����Լ�����*/								
				success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
			}
			/*add money to accounts*/
			//if(1 != (success = AddMoneyToClientAccount(client_id, len_client_id, charge_money, company_code+1)))
			//{
				/*add the account is error!!!*/
			//	perror("error@affair.c:HandleBusinessFromProxy:AddMoneyToClientAccount");
			//}
		}
			
		/*���͸������*/
		/*Ҫ���͵����������֮ǰ���ı�����״̬Ϊ4*/
		success = ModifyInfoStateInArray(4);
		bzero(send_to_proxy_buf, MAXPACKETSIZE);
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, PACKET_HEADER_LENGTH);
		memcpy(send_to_proxy_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, strlen(recv_from_company_buf));
		send_to_proxy_len = strlen(send_to_proxy_buf);

#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
				
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send db feedback to proxy");
			exit(1);
   		}
			
		success = 1;
		return success;				
				
	}
	
}



/*************************************************************************
 *  \brief
 *    �����������(�ⲿ��)
 *
 *  1. recieve the packet
 *  2. return the processed packet to sender
 *
 *  \par Input:
 *    sock_proxy: the socket accept connection from proxy, 
 *  \par Output:
 *
 *  \Return: 
 *    1: success
 *    0: fail 
************************************************************************/ 
int HandleReveralBusinessPacketNew(int sock_proxy, char *pkt_from_proxy)
{
	/*vaiables for recv and send packet by sockt*/
	/*from and to proxy*/
	int recv_from_proxy_len = 0;
	int send_to_proxy_len = 0;
	char recv_from_proxy_buf[MAXPACKETSIZE];
	char  send_to_proxy_buf[MAXPACKETSIZE];
	/*from and to company*/
	int recv_from_company_len = 0;
	int send_to_company_len = 0;
	char  recv_from_company_buf[MAXPACKETSIZE];
	char  *send_to_company_buf = NULL;
	/*from and to database*/
	int recv_from_db_len = 0;
	int send_to_db_len = 0;
	char  *recv_from_db_buf = NULL;
	char  send_to_db_buf[MAXPACKETSIZE];
	/*from and to database of forward packet*/
	int recv_from_db_forward_pkt_len = 0;
	int send_to_db_forward_pkt_len = 0;
	char  *recv_from_db_forward_pkt_buf = NULL;
	char  send_to_db_forward_pkt_buf[MAXPACKETSIZE];
       /*���������ݿ��ǰ����ͺ����*/
	//int send_db_reserval_forward_pkt_len = 0;
	//int send_db_reserval_backward_pkt_len = 0;
	//char  *send_db_reserval_forward_pkt = NULL;
	//char  *send_db_reserval_backward_pkt = NULL;

	/*the buf store the forward pkt*/
	int forward_pkt_len = 0;
	char  *forward_pkt_buf = NULL;
	char phonenumber[PHONE_NUMBER_LENGTH_AT_HEADER+1];
	int success = 0;

	/*the buf of common recv buf*/
	char buf_recv[MAXPACKETSIZE];
	//int mem_len = 0;
	
	/* varible for enterprise server*/
	int company_code = -1;
	char client_id[CLIENT_ID_INDEX_LENGTH_AT_HEADER+1];
	int len_client_id = 0;
	int service_type_code = -1;
	//int internal_packet_flag = -1;
	long int charge_money = 0;
	char la_serial_number[SERIAL_NUMBER_LENGTH + 1] = "00";
	//char packet_header[PACKET_HEADER_LENGTH+1];
	
	/*�����ݿ�Ĳ������Ӿ��*/
	//int sock_db;
	/*����绰�����ֶ�*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	bzero(send_to_db_forward_pkt_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*�õ���ҵ��*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*�õ�ҵ��������*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*��������佻�������ֶ�*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*������������к��ֶ�*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*�õ��绰�����ֶ�*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*�õ��ͻ��˺�*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*�õ��ɷѽ��*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);
	if(0>=charge_money && 3 != company_code)
	{
		//��������Ϊ����
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","��������Ϊ����");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//ֱ�ӷ����������proxy,
	}

	/*������ǰ��������ڲ���Ǹ�Ϊ"03"���������ݿ⣬�����ݿ���鷵���ĺϷ���*/
	/*it is a reversal foward packet, change the internal flag to "03" and send to database*/
	send_to_db_forward_pkt_len = recv_from_proxy_len;
	memcpy(send_to_db_forward_pkt_buf, recv_from_proxy_buf, recv_from_proxy_len+1);
	/*�ı��ڲ����Ϊ"03"*/
	success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "03");

	/*��ǰ������*/
	success = CompactOnePacketInDatabasePacket(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len);
	send_to_db_forward_pkt_len = strlen(send_to_db_forward_pkt_buf);
			
	/*�������ݿ�Ĳ�ѯ�˿�*/
	bzero(buf_recv, MAXPACKETSIZE);			
	//if(0 >= (success = SendPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	if(0 >= (success = SendNonsignificantPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase(Internal Packet)");
		exit(1);
	}
	recv_from_db_forward_pkt_buf = buf_recv;
				
	/*У�����ݿ�ķ��ؿ��Ƿ��ܹ�����*/
	if(1 != (success = ValidInternalSuccessFlagInHeader(recv_from_db_forward_pkt_buf, recv_from_db_forward_pkt_len)))
	{	
		/*��������*/
		/*not permit "reversal" operation, send the feedback packet from database to proxy and return*/
		send_to_proxy_len = recv_from_db_forward_pkt_len;
		memcpy(send_to_proxy_buf, recv_from_db_forward_pkt_buf, strlen(recv_from_db_forward_pkt_buf));
			
#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send databse forward reversal feedback to proxy");
			exit(1);
   		}
			
		return -1;
	}
	else
	{
		/*������*/
		/*Ҫ���͵����ݿ���з���֮ǰ��ע�ᵽע�����*/
		success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
	  
		/*Ҫ���͵����ݿ�֮ǰ���ı�����״̬Ϊ2*/
		success = ModifyInfoStateInArray(2);
		
		/*�����ݿ�ķ��ذ��еõ��ն˺ţ���ʱΪ�������ꡱ���ƵĹ��ܣ����ڲ�ʹ����
		memcpy(recv_from_proxy_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				recv_from_db_forward_pkt_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				CLIENT_ID_INDEX_LENGTH_AT_HEADER);*/

		/*������ǰ�����ϳ�һ�����������ݿ�������¼*/
		bzero(send_to_db_buf, 	MAXPACKETSIZE);
		memcpy(send_to_db_buf, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = recv_from_proxy_len;

		/*compact the database pkt*/
		success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = strlen(send_to_db_buf);
		printf("\n\n\nsend_to_db|%s|\n\n\n", send_to_db_buf);
		/*ֱ�ӽ��������׼�¼�����ݿ��У�֮���ٽ����������͸����ŷ�����*/
				
		/*�������ݿ��¼���ν���*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase");
			exit(1);
		}
		recv_from_db_buf = buf_recv;

		/*У�����ݿ��¼�������Ƿ�ɹ�*/
		if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
		{
			/*����ɹ�����Ϊ�Ƿ�����������Ҫ��������*/
			/*Ҫ���͵���ֵ������֮ǰ���ı�����״̬Ϊ3*/
			success = ModifyInfoStateInArray(3);
			
			/*������ֵ������*/
			success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
			if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
			{
				/*��������п�ҵ����Ҫ���̻������ϼ�һ��Ǯ*/
				/*���п��������ף����Լ�����*/
				success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);
			}
			else
			{
				/*����������п�ҵ����Ҫ���̻������ϼ�һ��Ǯ*/
				/*�������ף����Լ�����*/								
				success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
			}
			/*add money to accounts*/
			//if(1 != (success = AddMoneyToClientAccount(client_id, len_client_id, charge_money, company_code+1)))
			//{
				/*add the account is error!!!*/
			//	perror("error@affair.c:HandleBusinessFromProxy:AddMoneyToClientAccount");
			//}
		}

		/*���͸������*/
		/*Ҫ���͵����������֮ǰ���ı�����״̬Ϊ4*/
		success = ModifyInfoStateInArray(4);
		bzero(send_to_proxy_buf, MAXPACKETSIZE);
		memcpy(send_to_proxy_buf, recv_from_db_buf, strlen(recv_from_db_buf));
		send_to_proxy_len = strlen(send_to_proxy_buf);

#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
				
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send db feedback to proxy");
			exit(1);
   		}
			
		/*���������󴫸����ŷ�����*/
		/*Ҫ���͵����ŷ�����֮ǰ���ı�����״̬Ϊ1*/
		success = ModifyInfoStateInArray(1);
		send_to_company_buf = recv_from_proxy_buf + PACKET_HEADER_LENGTH;
		send_to_company_len = recv_from_proxy_len -PACKET_HEADER_LENGTH;
				
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 == global_par.company_par_array[company_code].use_middleware)
		{
			/*not use middleware*/
			bzero(buf_recv, MAXPACKETSIZE);
			if(0 == TransmitPacketWithDirectLink(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
			{
				perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithShortlinkInAffair");
				exit(1);
			}
			memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
		}
		else
		{
			bzero(buf_recv, MAXPACKETSIZE);
			if(0 == TransmitPacketWithMiddleware(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
			{
				perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithMiddleware");
				exit(1);
			}
			memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
			
		}	

			
		success = 1;
		return success;				
				
	}
	
}



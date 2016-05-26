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
	
	/*与数据库的测试连接句柄*/
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
	/*将终端号的第3位置为0，例如将HS188888置为HS088888*/
	memset(buf_recv+CLIENT_ID_INDEX_START_POSITION_AT_HEADER+2, '0', 1);
	
	/*得到内部包标记*/
	if(-1 == (internal_packet_flag = GetInternalPacketFlagFromHeader(buf_recv, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetInternalPacketFlagFromHeader");
		exit(1);
	}

	/*得到企业码*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(buf_recv, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*得到业务类型码*/
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
				//如果是电信的返销请求，先记录到数据库中，
				//再向电信请求返销，得到结果后再
				//向终端返回成功
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
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "01", "内部包标记不等于0，1，2，4中的一个!");

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
 *    处理缴费事务包(外部的)
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
       /*返销给数据库的前向包和后向包*/
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
	
	/*与数据库的测试连接句柄*/
	int sock_db;

	/*清零电话号码字段*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*得到企业码*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*得到业务类型码*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*按配置填充交易日期字段*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*按配置填充序列号字段*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*得到电话号码字段*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*得到客户端号*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*得到缴费金额*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);
	if(0>=charge_money && 3 != company_code)//允许移动0元缴费
	{
		//缴费金额不能为负数
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","缴费金额不能为负数");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//直接返回请求包给proxy,
	}
	
	/*如果是银行卡业务，则不需要判定商户的帐户余额，否则要进行判断*/
	if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
	{
		/*判定终端帐户余额是否能够完成本次缴费*/
    	if(0 == (success = QueryWarrantExcuteAffair(client_id, len_client_id, charge_money)))
		//if(0 == (success = WarrantExcuteAffair(client_id, len_client_id, charge_money, company_code+1)))
		{
			/*for this client, no charge right*/
			/*return the ERROR CODE to client*/
			memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
			send_to_proxy_len = recv_from_proxy_len;
			success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","交款押金余额不足");
		
			if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
			{
				success = 0;
				perror("error@affair.c:HandleBusinessFromProxy:send");
				exit(1);
    		}
		
			return 0;//直接返回请求包给proxy,
		}
	}

	/*cut off and save packet header */
	memcpy(packet_header, recv_from_proxy_buf, PACKET_HEADER_LENGTH);
	
	send_to_company_buf = recv_from_proxy_buf + PACKET_HEADER_LENGTH;
	send_to_company_len = recv_from_proxy_len -PACKET_HEADER_LENGTH;
	
	/*先判断数据库是否可用，如不可用，不能进行这笔业务*/
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
	/*如果数据库不可用*/
	if(-1 == success)
	{
		/*for this client, no charge right*/
		/*return the ERROR CODE to client*/
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","数据库不能工作，不能交易");
			
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
			
		return 0;//直接返回请求包给proxy,
	}				
			
	/*要发送到电信服务商之前，注册到注册表中*/
	success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
				
	  
	/*要发送到电信服务商之前，改变该项的状态为1*/
	success = ModifyInfoStateInArray(1);
	/*发送到电信服务商，得到返回包*/
	if(0 == global_par.company_par_array[company_code].use_middleware)
	{
		/*不使用中间件或不需要编程*/
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
		/*使用中间件或需要编程*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 == TransmitPacketWithMiddleware(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
		{
			perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithMiddleware");
			exit(1);
		}
		memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
	}	
		
	/*校验电信交易商的返回包中的响应码是否正确*/
	if(1 == ValidRespondCodeInBackwardPacket(recv_from_company_buf, recv_from_company_len, company_code, service_type_code))
	{
		/*如果电信交易商的返回包中的响应码正确*/
		/*respond_code is correct, need be recorded in database, add the common header to database*/
		memcpy(send_to_db_buf, packet_header, PACKET_HEADER_LENGTH);
  		memcpy(send_to_db_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, recv_from_company_len+1);
		send_to_db_len = recv_from_company_len+PACKET_HEADER_LENGTH;

		/*打包前向包和后向包*/
		success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, forward_pkt_buf, forward_pkt_len);
		send_to_db_len = strlen(send_to_db_buf);
				
		/*要发送到数据库记录之前，改变该项的状态*/
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

		/*校验数据库的返回包中的内部成功标记*/
		if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
		{
			/*如果成功，需要扣帐*/
			/*要发送到虚帐处理机之前，改变该项的状态*/
			success = ModifyInfoStateInArray(3);
			/*发给增值服务器*/
			success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
 
			if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
			{
				/*如果是银行卡业务，需要给商户的账上加上一笔钱*/
				/*银行卡缴费交易，所以加虚帐*/
				success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
			}
			else
			{
				/*如果不是银行卡业务，需要给商户的账上减一笔钱*/
				/*缴费交易，所以扣虚帐*/
				success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);				
			}
			//if(1 != (success = SubtractMoneyFromClientAccount(client_id, len_client_id, charge_money, company_code+1)))
			//{
				/*subtruct the account is error!!!*/
			//	perror("error@affair.c:HandleBusinessFromProxy:SubtractMoneyFromClientAccount");
			//}
		}
			
		/*返回数据库的返回包给代理机*/
		memcpy(send_to_proxy_buf, recv_from_db_buf, strlen(recv_from_db_buf));
		send_to_proxy_len = recv_from_db_len;
#ifdef DEBUG_TRANSMIT_PROXY_AFFAIR
		fflush(NULL);
		printf("send affair packet to proxy length= %d \n", send_to_proxy_len);
		printf("send affair packet to proxy is |%s| \n", send_to_proxy_buf);
		fflush(NULL);
#endif
		/*要发送到代理机之前，改变该项的状态*/
		success = ModifyInfoStateInArray(4);
			
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send db feedback to proxy");
			exit(1);
   		}

		/*发给发票数据库记录*/
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
		/*如果电信交易商的返回包中的响应码错误，直接返回给代理机*/
		/*in this case, the respond code of enterprise feedback
		packet is not "00", return the feedback packet (from Enterprise) to proxy*/
		memcpy(send_to_proxy_buf, packet_header, PACKET_HEADER_LENGTH);
		memcpy(send_to_proxy_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, recv_from_company_len+1);
		send_to_proxy_len = recv_from_company_len+PACKET_HEADER_LENGTH;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "01","电信服务商交易失败");

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
 *    处理返销事务包(外部的)
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
       /*返销给数据库的前向包和后向包*/
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
	
	/*与数据库的测试连接句柄*/
	//int sock_db;
	/*清零电话号码字段*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	bzero(send_to_db_forward_pkt_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*得到企业码*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*得到业务类型码*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*按配置填充交易日期字段*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*按配置填充序列号字段*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*得到电话号码字段*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*得到客户端号*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*得到缴费金额*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);
	if(0>=charge_money && 3 != company_code)
	{
		//返销金额不能为负数
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","返销金额不能为负数");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//直接返回请求包给proxy,
	}

	/*返销的前向包，将内部标记改为"03"，发给数据库，让数据库检验返销的合法性*/
	/*it is a reversal foward packet, change the internal flag to "03" and send to database*/
	send_to_db_forward_pkt_len = recv_from_proxy_len;
	memcpy(send_to_db_forward_pkt_buf, recv_from_proxy_buf, recv_from_proxy_len+1);
	/*改变内部标记为"03"*/
	success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "03");

	/*将前向包打包*/
	success = CompactOnePacketInDatabasePacket(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len);
	send_to_db_forward_pkt_len = strlen(send_to_db_forward_pkt_buf);
			
	/*发给数据库的查询端口*/
	bzero(buf_recv, MAXPACKETSIZE);			
	//if(0 >= (success = SendPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	if(0 >= (success = SendNonsignificantPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase(Internal Packet)");
		exit(1);
	}
	recv_from_db_forward_pkt_buf = buf_recv;
				
	/*校验数据库的返回看是否能够返销*/
	if(1 != (success = ValidInternalSuccessFlagInHeader(recv_from_db_forward_pkt_buf, recv_from_db_forward_pkt_len)))
	{	
		/*不允许返销*/
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
		/*允许返销*/
		/*要发送到数据库进行返销之前，注册到注册表中*/
		success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
	  
		/*要发送到数据库之前，改变该项的状态为2*/
		success = ModifyInfoStateInArray(2);
		
		/*从数据库的返回包中得到终端号，当时为”深蓝店”定制的功能，现在不使用了
		memcpy(recv_from_proxy_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				recv_from_db_forward_pkt_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				CLIENT_ID_INDEX_LENGTH_AT_HEADER);*/

		/*将两个前向包组合成一个包发给数据库让它记录*/
		bzero(send_to_db_buf, 	MAXPACKETSIZE);
		memcpy(send_to_db_buf, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = recv_from_proxy_len;

		/*compact the database pkt*/
		success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = strlen(send_to_db_buf);
		printf("\n\n\nsend_to_db|%s|\n\n\n", send_to_db_buf);
		/*直接将返销交易记录到数据库中，之后再将返销包发送给电信服务商*/
				
		/*先让数据库记录本次交易*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase");
			exit(1);
		}
		recv_from_db_buf = buf_recv;

		/*校验数据库记录返销包是否成功*/
		if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
		{
			/*如果成功，因为是返销包，所以要增加虚帐*/
			/*要发送到增值服务器之前，改变该项的状态为3*/
			success = ModifyInfoStateInArray(3);
			
			/*发给增值服务器*/
			success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
			if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
			{
				/*如果是银行卡业务，需要给商户的账上减一笔钱*/
				/*银行卡返销交易，所以减虚帐*/
				success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);
			}
			else
			{
				/*如果不是银行卡业务，需要给商户的账上加一笔钱*/
				/*返销交易，所以加虚帐*/								
				success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
			}
			/*add money to accounts*/
			//if(1 != (success = AddMoneyToClientAccount(client_id, len_client_id, charge_money, company_code+1)))
			//{
				/*add the account is error!!!*/
			//	perror("error@affair.c:HandleBusinessFromProxy:AddMoneyToClientAccount");
			//}
		}
			
		/*将返销请求传给电信服务器*/
		/*要发送到电信服务商之前，改变该项的状态为1*/
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

		/*发送给代理机*/
		/*要发送到代理服务器之前，改变该项的状态为4*/
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
		/*发给发票数据库记录*/
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
 *    处理查询往月这类只写入临时数据库的包(外部的)
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
	
	/*与数据库的测试连接句柄*/
	//int sock_db;
	/*清零电话号码字段*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*得到企业码*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*得到业务类型码*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*按配置填充交易日期字段*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*按配置填充序列号字段*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*得到电话号码字段*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);

	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;

	/*发送到电信服务商，得到返回包*/
	send_to_company_buf = recv_from_proxy_buf + PACKET_HEADER_LENGTH;
	
	memcpy(packet_header, recv_from_proxy_buf, PACKET_HEADER_LENGTH);

	send_to_company_len = recv_from_proxy_len -PACKET_HEADER_LENGTH;

	if(0 == global_par.company_par_array[company_code].use_middleware)
	{
		/*不使用中间件或不需要编程*/
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
		/*使用中间件或需要编程*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 == TransmitPacketWithMiddleware(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
		{
			perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithMiddleware");
			exit(1);
		}
		memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
	}	
	//printf("recv_comp_buf:|%s|\n", recv_from_company_buf);

	/*将返回包和前向包发给数据库，使数据库记入临时数据表*/
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

	/*将数据库的返回包发给代理机*/
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
	/*发给发票数据库记录*/
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
 *    处理当月查询这类不记录到数据库的包(外部的)
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
	
	/*与数据库的测试连接句柄*/
	//int sock_db;
	/*清零电话号码字段*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);

	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
		
	/*得到企业码*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*得到业务类型码*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*按配置填充交易日期字段*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*按配置填充序列号字段*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*得到电话号码字段*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	//printf("pkt_sub_function:|%s|\n",recv_from_proxy_buf);
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;

	/*发送到电信服务商，得到返回包*/
	send_to_company_buf = recv_from_proxy_buf + PACKET_HEADER_LENGTH;
	send_to_company_len = recv_from_proxy_len -PACKET_HEADER_LENGTH;

	if(0 == global_par.company_par_array[company_code].use_middleware)
	{
		/*不使用中间件或不需要编程*/
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
		/*使用中间件或需要编程*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 == TransmitPacketWithMiddleware(send_to_company_buf, send_to_company_len, buf_recv, &recv_from_company_len, company_code, service_type_code))
		{
			perror("error@affair.c:HandleBusinessFromProxy:TransmitPacketWithMiddleware");
			exit(1);
		}
		memcpy(recv_from_company_buf, buf_recv, recv_from_company_len+1);
	}	

	/*将电信公司的返回包发给代理机*/
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
 *    处理内部事务包
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
	
	/*与数据库的测试连接句柄*/
	//int sock_db;
	/*清零电话号码字段*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*直接将它发给数据库的查询端口*/
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
	/*返回数据库的处理结果给代理机*/
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
 *    处理发票查询事务包
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
	
	/*与数据库的测试连接句柄*/
	//int sock_db;
	/*清零电话号码字段*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*直接将它发给数据库的查询端口*/
	send_to_db_len = recv_from_proxy_len;
	send_to_db_buf = recv_from_proxy_buf;

	success = CompactOnePacketInDatabasePacket(send_to_db_buf, send_to_db_len);
	send_to_db_len = strlen(send_to_db_buf);

	/*发给发票数据库取发票数据*/
	bzero(buf_recv, MAXPACKETSIZE);
	if(0 >= (success = SendPacketInvoiceDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len, 1)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketInvoiceDatabase");
		exit(1);
	}
	recv_from_db_buf = buf_recv;
	/*返回数据库的处理结果给代理机*/
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
 *    处理内环缴费事务包，用于只记账的功能（例如，定机票等业务）
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
	
	/*与数据库的测试连接句柄*/
	//int sock_db;
	/*清零电话号码字段*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);

	/*将内部包标记改为"00"*/	
	//success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "00");
	success = SetInternalPacketFlagInHeader(recv_from_proxy_buf, recv_from_proxy_len, "00");
		
	/*得到企业码*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*得到业务类型码*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*按配置填充交易日期字段*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*按配置填充序列号字段*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*得到电话号码字段*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*得到客户端号*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*得到缴费金额*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);

	if(0>charge_money)
	{
		//缴费金额不能为负数
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","缴费金额不能为负数");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//直接返回请求包给proxy,
	}

	/*如果是银行卡业务，则不需要判定商户的帐户余额，否则要进行判断*/
	if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
	{
		/*判定终端帐户余额是否能够完成本次缴费*/
    	if(0 == (success = QueryWarrantExcuteAffair(client_id, len_client_id, charge_money)))
		//if(0 == (success = WarrantExcuteAffair(client_id, len_client_id, charge_money, company_code+1)))
		{
			/*for this client, no charge right*/
			/*return the ERROR CODE to client*/
			memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
			send_to_proxy_len = recv_from_proxy_len;
			success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","交款押金余额不足");
		
			if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
			{
				success = 0;
				perror("error@affair.c:HandleBusinessFromProxy:send");
				exit(1);
    		}
		
			return 0;//直接返回请求包给proxy,
		}
	}
	
	/*cut off and save packet header */
	memcpy(packet_header, recv_from_proxy_buf, PACKET_HEADER_LENGTH);
	
	/*要发送到数据库之前，注册到注册表中*/
	success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
				
	  
	/*respond_code is correct, need be recorded in database, add the common header to database*/
	bzero(send_to_db_buf, MAXPACKETSIZE);
	memcpy(send_to_db_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
	send_to_db_len = strlen(send_to_db_buf);
	/*打包前向包和后向包,他们是相同的*/
	success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, forward_pkt_buf, forward_pkt_len);
	send_to_db_len = strlen(send_to_db_buf);
				
	/*要发送到数据库记录之前，改变该项的状态*/
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

	/*校验数据库的返回包中的内部成功标记*/
	if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
	{
		/*如果成功，需要扣帐*/
		/*要发送到虚帐处理机之前，改变该项的状态*/
		success = ModifyInfoStateInArray(3);
		/*发给增值服务器*/
		success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
 		if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
		{
			/*如果是银行卡业务，需要给商户的账上加上一笔钱*/
			/*银行卡缴费交易，所以加虚帐*/
			success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
		}
		else
		{
			/*如果不是银行卡业务，需要给商户的账上减一笔钱*/
			/*缴费交易，所以扣虚帐*/
			success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);				
		}
		//if(1 != (success = SubtractMoneyFromClientAccount(client_id, len_client_id, charge_money, company_code+1)))
		//{
			/*subtruct the account is error!!!*/
		//	perror("error@affair.c:HandleBusinessFromProxy:SubtractMoneyFromClientAccount");
		//}
	}
			
	/*返回数据库的返回包给代理机*/
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
	/*要发送到代理机之前，改变该项的状态*/
	success = ModifyInfoStateInArray(4);
			
	if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
	{
		success = 0;
		perror("error@affair.c:HandleBusinessFromProxy:send db feedback to proxy");
		exit(1);
 	}
	/*发给发票数据库记录*/
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
 *    处理内环返销事务包，用于只记账的功能（例如，取消定机票等业务）
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
       /*返销给数据库的前向包和后向包*/
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
	
	/*与数据库的测试连接句柄*/
	//int sock_db;
	/*清零电话号码字段*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	bzero(send_to_db_forward_pkt_buf, MAXPACKETSIZE);

	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*将内部包标记改为"00"*/	
	//success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "00");
	success = SetInternalPacketFlagInHeader(recv_from_proxy_buf, recv_from_proxy_len, "00");
		
	/*得到企业码*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*得到业务类型码*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*按配置填充交易日期字段*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*按配置填充序列号字段*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}

	/*得到电话号码字段*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
		
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*得到客户端号*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*得到缴费金额*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);

	if(0>charge_money)
	{
		//返销金额不能为负数
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","返销金额不能为负数");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//直接返回请求包给proxy,
	}
	
	/*返销的前向包，将内部标记改为"03"，发给数据库，让数据库检验返销的合法性*/
	/*it is a reversal foward packet, change the internal flag to "03" and send to database*/
	send_to_db_forward_pkt_len = recv_from_proxy_len;
	memcpy(send_to_db_forward_pkt_buf, recv_from_proxy_buf, recv_from_proxy_len+1);
	/*改变内部标记为"03"*/
	success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "03");
	
	/*将前向包打包*/
	success = CompactOnePacketInDatabasePacket(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len);
	send_to_db_forward_pkt_len = strlen(send_to_db_forward_pkt_buf);
				
	/*发给数据库的查询端口*/
	bzero(buf_recv, MAXPACKETSIZE); 		
	//if(0 >= (success = SendPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	if(0 >= (success = SendNonsignificantPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase(Internal Packet)");
		exit(1);
	}
	recv_from_db_forward_pkt_buf = buf_recv;
					
	/*校验数据库的返回看是否能够返销*/
	if(1 != (success = ValidInternalSuccessFlagInHeader(recv_from_db_forward_pkt_buf, recv_from_db_forward_pkt_len)))
	{	
		/*不允许返销*/
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
		/*允许返销*/
		/*要发送到数据库进行返销之前，注册到注册表中*/
		success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
		  
		/*要发送到数据库之前，改变该项的状态为2*/
		success = ModifyInfoStateInArray(2);
			
		/*从数据库的返回包中得到终端号，当时为”深蓝店”定制的功能，现在不使用了
		memcpy(recv_from_proxy_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				recv_from_db_forward_pkt_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				CLIENT_ID_INDEX_LENGTH_AT_HEADER);*/
	
		/*将两个前向包组合成一个包发给数据库让它记录*/
		bzero(send_to_db_buf,	MAXPACKETSIZE);
		memcpy(send_to_db_buf, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = recv_from_proxy_len;
	
		/*compact the database pkt*/
		success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = strlen(send_to_db_buf);
	
		/*直接将返销交易记录到数据库中，之后再将返销包发送给电信服务商*/
					
		/*先让数据库记录本次交易*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase");
			exit(1);
		}
		recv_from_db_buf = buf_recv;
	
		/*校验数据库记录返销包是否成功*/
		if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
		{
			/*如果成功，因为是返销包，所以要增加虚帐*/
			/*要发送到增值服务器之前，改变该项的状态为3*/
			success = ModifyInfoStateInArray(3);
				
			/*发给增值服务器*/
			success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
			if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
			{
				/*如果是银行卡业务，需要给商户的账上减一笔钱*/
				/*银行卡返销交易，所以减虚帐*/
				success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);
			}
			else
			{
				/*如果不是银行卡业务，需要给商户的账上加一笔钱*/
				/*返销交易，所以加虚帐*/								
				success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
			}

			/*add money to accounts*/
			//if(1 != (success = AddMoneyToClientAccount(client_id, len_client_id, charge_money, company_code+1)))
			//{
				/*add the account is error!!!*/
			//	perror("error@affair.c:HandleBusinessFromProxy:AddMoneyToClientAccount");
			//}
		}

		/*发送给代理机*/
		/*要发送到代理服务器之前，改变该项的状态为4*/
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
		/*发给发票数据库记录*/
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
 *    处理电信这种特殊的返销事务包(外部的)， 要求先向电信发包
 *    然后再将终端的前向包返回给客户端
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
       /*返销给数据库的前向包和后向包*/
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
	
	/*与数据库的测试连接句柄*/
	//int sock_db;
	/*清零电话号码字段*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	bzero(send_to_db_forward_pkt_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*得到企业码*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*得到业务类型码*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*按配置填充交易日期字段*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*按配置填充序列号字段*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*得到电话号码字段*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*得到客户端号*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*得到缴费金额*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);
	if(0>=charge_money)
	{
		//返销金额不能为负数
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","返销金额不能为负数");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//直接返回请求包给proxy,
	}

	/*返销的前向包，将内部标记改为"03"，发给数据库，让数据库检验返销的合法性*/
	/*it is a reversal foward packet, change the internal flag to "03" and send to database*/
	send_to_db_forward_pkt_len = recv_from_proxy_len;
	memcpy(send_to_db_forward_pkt_buf, recv_from_proxy_buf, recv_from_proxy_len+1);
	/*改变内部标记为"03"*/
	success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "03");

	/*将前向包打包*/
	success = CompactOnePacketInDatabasePacket(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len);
	send_to_db_forward_pkt_len = strlen(send_to_db_forward_pkt_buf);
			
	/*发给数据库的查询端口*/
	bzero(buf_recv, MAXPACKETSIZE);			
	//if(0 >= (success = SendPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	if(0 >= (success = SendNonsignificantPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase(Internal Packet)");
		exit(1);
	}
	recv_from_db_forward_pkt_buf = buf_recv;
				
	/*校验数据库的返回看是否能够返销*/
	if(1 != (success = ValidInternalSuccessFlagInHeader(recv_from_db_forward_pkt_buf, recv_from_db_forward_pkt_len)))
	{	
		/*不允许返销*/
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
		/*允许返销*/
		/*要发送到数据库进行返销之前，注册到注册表中*/
		success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
	  
		/*将返销请求传给电信服务器*/
		/*要发送到电信服务商之前，改变该项的状态为1*/
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
		/*校验电信交易商的返回包中的响应码是否正确*/
		if(1 != ValidRespondCodeInBackwardPacket(recv_from_company_buf, recv_from_company_len, company_code, service_type_code))
		{
			/*如果电信交易商的返回包中的响应码不正确,直接返回错误给客户端*/
			/*发送给代理机*/
			/*要发送到代理服务器之前，改变该项的状态为4*/
			success = ModifyInfoStateInArray(4);
			bzero(send_to_proxy_buf, MAXPACKETSIZE);
			memcpy(send_to_proxy_buf, recv_from_proxy_buf, PACKET_HEADER_LENGTH);
			memcpy(send_to_proxy_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, strlen(recv_from_company_buf));
			send_to_proxy_len = strlen(send_to_proxy_buf);
			success = WriteInternalSuccessFlag(send_to_proxy_buf, "04","电信服务商不让返销!");

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
			
			/*发给客户端后直接退出该过程*/	
			success = -1;
			return success;		
		}
		
		/*要发送到数据库之前，改变该项的状态为2*/
		success = ModifyInfoStateInArray(2);
		
		/*从数据库的返回包中得到终端号，当时为”深蓝店”定制的功能，现在不使用了
		memcpy(recv_from_proxy_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				recv_from_db_forward_pkt_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				CLIENT_ID_INDEX_LENGTH_AT_HEADER);*/

		/*将两个前向包组合成一个包发给数据库让它记录*/
		bzero(send_to_db_buf, 	MAXPACKETSIZE);
		memcpy(send_to_db_buf, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = recv_from_proxy_len;

		/*compact the database pkt*/
		success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = strlen(send_to_db_buf);

		/*直接将返销交易记录到数据库中，之后再将返销包发送给电信服务商*/
				
		/*先让数据库记录本次交易*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase");
			exit(1);
		}
		recv_from_db_buf = buf_recv;

		/*校验数据库记录返销包是否成功*/
		if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
		{
			/*如果成功，因为是返销包，所以要增加虚帐*/
			/*要发送到增值服务器之前，改变该项的状态为3*/
			success = ModifyInfoStateInArray(3);
			
			/*发给增值服务器*/
			success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
			if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
			{
				/*如果是银行卡业务，需要给商户的账上减一笔钱*/
				/*银行卡返销交易，所以减虚帐*/
				success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);
			}
			else
			{
				/*如果不是银行卡业务，需要给商户的账上加一笔钱*/
				/*返销交易，所以加虚帐*/								
				success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
			}
			/*add money to accounts*/
			//if(1 != (success = AddMoneyToClientAccount(client_id, len_client_id, charge_money, company_code+1)))
			//{
				/*add the account is error!!!*/
			//	perror("error@affair.c:HandleBusinessFromProxy:AddMoneyToClientAccount");
			//}
		}
			
		/*发送给代理机*/
		/*要发送到代理服务器之前，改变该项的状态为4*/
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
 *    处理电信这种特殊的返销事务包(外部的)， 要求先向电信发包
 *    要记电信的返回包
 *    然后再将终端的前向包返回给客户端
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
       /*返销给数据库的前向包和后向包*/
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
	
	/*与数据库的测试连接句柄*/
	//int sock_db;
	/*清零电话号码字段*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	bzero(send_to_db_forward_pkt_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*得到企业码*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*得到业务类型码*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*按配置填充交易日期字段*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*按配置填充序列号字段*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*得到电话号码字段*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*得到客户端号*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*得到缴费金额*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);
	if(0>=charge_money)
	{
		//返销金额不能为负数
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","返销金额不能为负数");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//直接返回请求包给proxy,
	}

	/*返销的前向包，将内部标记改为"03"，发给数据库，让数据库检验返销的合法性*/
	/*it is a reversal foward packet, change the internal flag to "03" and send to database*/
	send_to_db_forward_pkt_len = recv_from_proxy_len;
	memcpy(send_to_db_forward_pkt_buf, recv_from_proxy_buf, recv_from_proxy_len+1);
	/*改变内部标记为"03"*/
	success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "03");

	/*将前向包打包*/
	success = CompactOnePacketInDatabasePacket(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len);
	send_to_db_forward_pkt_len = strlen(send_to_db_forward_pkt_buf);
			
	/*发给数据库的查询端口*/
	bzero(buf_recv, MAXPACKETSIZE);			
	//if(0 >= (success = SendPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	if(0 >= (success = SendNonsignificantPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase(Internal Packet)");
		exit(1);
	}
	recv_from_db_forward_pkt_buf = buf_recv;
				
	/*校验数据库的返回看是否能够返销*/
	if(1 != (success = ValidInternalSuccessFlagInHeader(recv_from_db_forward_pkt_buf, recv_from_db_forward_pkt_len)))
	{	
		/*不允许返销*/
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
		/*允许返销*/
		/*要发送到数据库进行返销之前，注册到注册表中*/
		success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
	  
		/*将返销请求传给电信服务器*/
		/*要发送到电信服务商之前，改变该项的状态为1*/
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
		/*校验电信交易商的返回包中的响应码是否正确*/
		if(1 != ValidRespondCodeInBackwardPacket(recv_from_company_buf, recv_from_company_len, company_code, service_type_code))
		{
			/*如果电信交易商的返回包中的响应码不正确,直接返回错误给客户端*/
			/*发送给代理机*/
			/*要发送到代理服务器之前，改变该项的状态为4*/
			success = ModifyInfoStateInArray(4);
			bzero(send_to_proxy_buf, MAXPACKETSIZE);
			memcpy(send_to_proxy_buf, recv_from_proxy_buf, PACKET_HEADER_LENGTH);
			memcpy(send_to_proxy_buf+PACKET_HEADER_LENGTH, recv_from_company_buf, strlen(recv_from_company_buf));
			send_to_proxy_len = strlen(send_to_proxy_buf);
			success = WriteInternalSuccessFlag(send_to_proxy_buf, "04","电信服务商不让返销!");

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
			
			/*发给客户端后直接退出该过程*/	
			success = -1;
			return success;		
		}
		
		/*要发送到数据库之前，改变该项的状态为2*/
		success = ModifyInfoStateInArray(2);
		
		/*从数据库的返回包中得到终端号，当时为”深蓝店”定制的功能，现在不使用了
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

		/*打包前向包和后向包*/
		success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, forward_pkt_buf, forward_pkt_len);
		send_to_db_len = strlen(send_to_db_buf);

		///*将两个前向包组合成一个包发给数据库让它记录*/
		//bzero(send_to_db_buf, 	MAXPACKETSIZE);
		//memcpy(send_to_db_buf, recv_from_proxy_buf, recv_from_proxy_len);
		//send_to_db_len = recv_from_proxy_len;

		/*compact the database pkt*/
		//success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, recv_from_proxy_buf, recv_from_proxy_len);
		//send_to_db_len = strlen(send_to_db_buf);

		/*直接将返销交易记录到数据库中，之后再将返销包发送给电信服务商*/
				
		/*先让数据库记录本次交易*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase");
			exit(1);
		}
		recv_from_db_buf = buf_recv;

		/*校验数据库记录返销包是否成功*/
		if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
		{
			/*如果成功，因为是返销包，所以要增加虚帐*/
			/*要发送到增值服务器之前，改变该项的状态为3*/
			success = ModifyInfoStateInArray(3);
			
			/*发给增值服务器*/
			success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
			if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
			{
				/*如果是银行卡业务，需要给商户的账上减一笔钱*/
				/*银行卡返销交易，所以减虚帐*/
				success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);
			}
			else
			{
				/*如果不是银行卡业务，需要给商户的账上加一笔钱*/
				/*返销交易，所以加虚帐*/								
				success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
			}
			/*add money to accounts*/
			//if(1 != (success = AddMoneyToClientAccount(client_id, len_client_id, charge_money, company_code+1)))
			//{
				/*add the account is error!!!*/
			//	perror("error@affair.c:HandleBusinessFromProxy:AddMoneyToClientAccount");
			//}
		}
			
		/*发送给代理机*/
		/*要发送到代理服务器之前，改变该项的状态为4*/
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
 *    处理返销事务包(外部的)
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
       /*返销给数据库的前向包和后向包*/
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
	
	/*与数据库的测试连接句柄*/
	//int sock_db;
	/*清零电话号码字段*/
	bzero(phonenumber, PHONE_NUMBER_LENGTH_AT_HEADER+1);
	
	bzero(buf_recv, MAXPACKETSIZE);
	bzero(recv_from_proxy_buf, MAXPACKETSIZE);
	bzero(send_to_proxy_buf, MAXPACKETSIZE);
	bzero(recv_from_company_buf, MAXPACKETSIZE);
	bzero(send_to_db_buf, MAXPACKETSIZE);
	bzero(send_to_db_forward_pkt_buf, MAXPACKETSIZE);
	
	recv_from_proxy_len = strlen(pkt_from_proxy);
	memcpy(recv_from_proxy_buf, pkt_from_proxy, recv_from_proxy_len+1);
		
	/*得到企业码*/
	if(-1 == (company_code = GetEnterpriseCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetEnterpriseCodeFromHeader");
		exit(1);
	}

	/*得到业务类型码*/
	if(-1 == (service_type_code = GetServiceTypeCodeFromHeader(recv_from_proxy_buf, recv_from_proxy_len)))
	{
		/*error when parse pkt*/
		perror("error:affair.c:HandleBusinessFromProxy:GetServiceTypeCodeFromHeader");
		exit(1);
	}

	/*按配置填充交易日期字段*/
	if(-1 == (success = FillRecordDateWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillRecordDateWithinForwardPacket");
		exit(1);
	}

	/*按配置填充序列号字段*/
	bzero(la_serial_number, SERIAL_NUMBER_LENGTH);
	if(-1 == (success = FillSerialNumberWithinForwardPacket(recv_from_proxy_buf+PACKET_HEADER_LENGTH, company_code, service_type_code, la_serial_number)))
	{
		/*error when fill serial_number*/
		perror("error:affair.c:HandleBusinessFromProxy:FillSerialNumberWithinForwardPacket");
		exit(1);
	}
	/*得到电话号码字段*/
	GetPhoneNumberFromHeader(recv_from_proxy_buf, strlen(recv_from_proxy_buf), phonenumber);
	
	/*store the forward pkt again, this time has serial number of li'an*/
	forward_pkt_len = recv_from_proxy_len;
	forward_pkt_buf = recv_from_proxy_buf;
	/*得到客户端号*/
	success = GetClientSerialFromHeader(recv_from_proxy_buf, recv_from_proxy_len, client_id, &len_client_id);
	/*得到缴费金额*/
	charge_money = GetChargeMoneyFromHeader(recv_from_proxy_buf, recv_from_proxy_len);
	if(0>=charge_money && 3 != company_code)
	{
		//返销金额不能为负数
		memcpy(send_to_proxy_buf, recv_from_proxy_buf, strlen(recv_from_proxy_buf));
		send_to_proxy_len = recv_from_proxy_len;
		success = WriteInternalSuccessFlag(send_to_proxy_buf, "03","返销金额不能为负数");
		
		if(multi_send(sock_proxy, send_to_proxy_buf, send_to_proxy_len, 0)==-1) 
		{
			success = 0;
			perror("error@affair.c:HandleBusinessFromProxy:send");
			exit(1);
    		}
		
		return 0;//直接返回请求包给proxy,
	}

	/*返销的前向包，将内部标记改为"03"，发给数据库，让数据库检验返销的合法性*/
	/*it is a reversal foward packet, change the internal flag to "03" and send to database*/
	send_to_db_forward_pkt_len = recv_from_proxy_len;
	memcpy(send_to_db_forward_pkt_buf, recv_from_proxy_buf, recv_from_proxy_len+1);
	/*改变内部标记为"03"*/
	success = SetInternalPacketFlagInHeader(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, "03");

	/*将前向包打包*/
	success = CompactOnePacketInDatabasePacket(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len);
	send_to_db_forward_pkt_len = strlen(send_to_db_forward_pkt_buf);
			
	/*发给数据库的查询端口*/
	bzero(buf_recv, MAXPACKETSIZE);			
	//if(0 >= (success = SendPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	if(0 >= (success = SendNonsignificantPacketDatabase(send_to_db_forward_pkt_buf, send_to_db_forward_pkt_len, buf_recv, &recv_from_db_forward_pkt_len)))
	{
		/*error!!*/
		perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase(Internal Packet)");
		exit(1);
	}
	recv_from_db_forward_pkt_buf = buf_recv;
				
	/*校验数据库的返回看是否能够返销*/
	if(1 != (success = ValidInternalSuccessFlagInHeader(recv_from_db_forward_pkt_buf, recv_from_db_forward_pkt_len)))
	{	
		/*不允许返销*/
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
		/*允许返销*/
		/*要发送到数据库进行返销之前，注册到注册表中*/
		success = RegisterInfoToArrayEx(company_code, service_type_code, charge_money, la_serial_number, client_id, phonenumber);
	  
		/*要发送到数据库之前，改变该项的状态为2*/
		success = ModifyInfoStateInArray(2);
		
		/*从数据库的返回包中得到终端号，当时为”深蓝店”定制的功能，现在不使用了
		memcpy(recv_from_proxy_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				recv_from_db_forward_pkt_buf+CLIENT_ID_INDEX_START_POSITION_AT_HEADER, \
				CLIENT_ID_INDEX_LENGTH_AT_HEADER);*/

		/*将两个前向包组合成一个包发给数据库让它记录*/
		bzero(send_to_db_buf, 	MAXPACKETSIZE);
		memcpy(send_to_db_buf, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = recv_from_proxy_len;

		/*compact the database pkt*/
		success = CompactTwoPacketInDatabasePacket(send_to_db_buf, send_to_db_len, recv_from_proxy_buf, recv_from_proxy_len);
		send_to_db_len = strlen(send_to_db_buf);
		printf("\n\n\nsend_to_db|%s|\n\n\n", send_to_db_buf);
		/*直接将返销交易记录到数据库中，之后再将返销包发送给电信服务商*/
				
		/*先让数据库记录本次交易*/
		bzero(buf_recv, MAXPACKETSIZE);
		if(0 >= (success = SendPacketDatabase(send_to_db_buf, send_to_db_len, buf_recv, &recv_from_db_len)))
		{
			/*error!!*/
			perror("error:affair.c:HandleBusinessFromProxy:SendPacketDatabase");
			exit(1);
		}
		recv_from_db_buf = buf_recv;

		/*校验数据库记录返销包是否成功*/
		if(1 == ValidInternalSuccessFlagInHeader(recv_from_db_buf, recv_from_db_len))
		{
			/*如果成功，因为是返销包，所以要增加虚帐*/
			/*要发送到增值服务器之前，改变该项的状态为3*/
			success = ModifyInfoStateInArray(3);
			
			/*发给增值服务器*/
			success = SendPacketToAddValueServer(recv_from_db_buf, recv_from_db_len);
			if(0 != strncmp("bank_card_", global_par.company_par_array[company_code].company_name,10))
			{
				/*如果是银行卡业务，需要给商户的账上减一笔钱*/
				/*银行卡返销交易，所以减虚帐*/
				success = RequireSubtractMoneyFromClientAccount(client_id, len_client_id, charge_money);
			}
			else
			{
				/*如果不是银行卡业务，需要给商户的账上加一笔钱*/
				/*返销交易，所以加虚帐*/								
				success = RequireAddMoneyToClientAccount(client_id, len_client_id, charge_money);
			}
			/*add money to accounts*/
			//if(1 != (success = AddMoneyToClientAccount(client_id, len_client_id, charge_money, company_code+1)))
			//{
				/*add the account is error!!!*/
			//	perror("error@affair.c:HandleBusinessFromProxy:AddMoneyToClientAccount");
			//}
		}

		/*发送给代理机*/
		/*要发送到代理服务器之前，改变该项的状态为4*/
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
			
		/*将返销请求传给电信服务器*/
		/*要发送到电信服务商之前，改变该项的状态为1*/
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



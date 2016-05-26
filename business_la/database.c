/*********************************************************
 *project: Line communication charges supermarket
 *filename: database.c
 *version: 0.5
 *purpose: functions about send the pkt to database
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-13
 *********************************************************/
 
#include "database.h"

/*************************************************************************
 *  \brief
 *    judge the primary database server is alive accoding the share varible 
 *
 *  \par Input:

 *  \par Output:
 *  \Return:
 *     1: primary database server is alive
 *    <=0: primary database server is crush
************************************************************************/
int JudgePrimaryDatabaseServer()
{
	int success;

	/*varibles for check data channels*/
	struct DB_status *shm_db_status;
	int sem_db_status;
	
	/*malloc the share memory for check data channels at the first time*/
	sem_db_status = GetExistedSemphoreExt(SEM_DB_STATUS_FTOK_ID);
	shm_db_status = (struct DB_status *)MappingShareMemOwnSpaceExt(SHM_DB_STATUS_FTOK_ID);

   	success = AcquireAccessRight(sem_db_status);
	if(PRIMARY == shm_db_status->status)
	{
		/*Unmapping the share memory of data channels status*/
		success = UnmappingShareMem((void*)shm_db_status);
  		success = ReleaseAccessRight(sem_db_status);

		return 1;
	}
	else
	{
		/*Unmapping the share memory of data channels status*/
		success = UnmappingShareMem((void*)shm_db_status);
  		success = ReleaseAccessRight(sem_db_status);

		return 0;
	}
}

/*************************************************************************
 *  \brief
 *    Send the packet to database 
 *
 *  \par Input:
 *     buf_send: the buffer pointer to the packet which is to be sent.
 *     send_len: the length of this packet.
 *     buf_recv_db: the recieve buffer from DB.
 *     recv_db_len: the length of feedback packet from DB.
 *  \par Output:
 *  \Return:
 *     1: success
 *    <=0: the function has some error
************************************************************************/
int SendPacketDatabase(char *buf_send, int send_len, char *buf_recv_db, int *recv_db_len)
{
#ifdef DEBUG_SEND_PACKET_DATABASE
	int success = 0;

	/*varibles for connect database*/
	int sock_db;
	 
	char buf_recv[MAXPACKETSIZE];
	int recv_len;
	
	bzero(buf_recv, MAXPACKETSIZE);

	/*Connection to database control server*/
   sock_db = CreateSocket();
   	
   /*judge the destination of the packet, read some share varibles*/

   do{
    	/*judge the destination of the packet, read some share varibles*/
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
   }while(0>success);


	/*retransmit the packet to database*/
	if(multi_send(sock_db,buf_send,send_len,0)==-1) 
	{
		success = 0;
		perror("error@database.c:SendPacketDatabase:send\n");
		exit(1);
    }
#ifdef DEBUG_TRANSMIT_DATABASE_AFFAIR
	fflush(NULL);
	printf("send to database AFFAIR packet length= |%d| \n", send_len);
	printf("send pkt to database affair packet is |%s| \n", buf_send);
	fflush(NULL);
#endif
	/*recieve the feedback packet from database*/
	recv_len = multi_recv(sock_db, buf_recv, MAXPACKETSIZE, 0);
	if(recv_len<0)
	{
		success = 0;
		perror("error@database.c:SendPacketDatabase:recv\n");
		exit(1);
	}
	*recv_db_len = recv_len;
	memcpy(buf_recv_db, buf_recv, recv_len); 
#ifdef DEBUG_TRANSMIT_DATABASE_AFFAIR
	fflush(NULL);
	printf("recv from database AFFAIR packet length = |%d| \n", recv_len);
	printf("recv from database AFFAIR packet is |%s| \n", buf_recv);
	fflush(NULL);
#endif

   	/*close this socket*/
   	close(sock_db);
#else
	int success = 1;
	memcpy(buf_recv_db, buf_send, send_len);
	*recv_db_len = send_len;
	//FILE *fp;
	//fp= fopen("1.txt", "wb");
	//fwrite(buf_send, 1, strlen(buf_send), fp);
	//fclose(fp);
	//printf("recv from business is : |%d|,            |%s|\n", strlen(buf_send), buf_send);
	//printf("not send to database!!!!\n");
#endif

	return success;	
}

/*************************************************************************
 *  \brief
 *    Send the packet to Invoice database 
 *
 *  \par Input:
 *     buf_send: the buffer pointer to the packet which is to be sent.
 *     send_len: the length of this packet.
 *     buf_recv_db: the recieve buffer from DB.
 *     recv_db_len: the length of feedback packet from DB.
 *  \par Output:
 *  \Return:
 *     1: success
 *    <=0: the function has some error
************************************************************************/
int SendPacketInvoiceDatabase(char *buf_send, int send_len, char *buf_recv_db, int *recv_db_len, int use_query_port)
{
#ifdef DEBUG_SEND_PACKET_DATABASE
	int success = 0;

	/*varibles for connect database*/
	int sock_db;
	 
	char buf_recv[MAXPACKETSIZE];
	int recv_len;
	
	bzero(buf_recv, MAXPACKETSIZE);

	/*Connection to database control server*/
   	sock_db = CreateSocket();
   	
	if(1==use_query_port)
  	{
  		success = ConnectServerOneTime(global_par.system_par.database_invoice_ip_address, global_par.system_par.database_query_port, sock_db);
	}
	else
	{
	  	success = ConnectServerOneTime(global_par.system_par.database_invoice_ip_address, global_par.system_par.database_data_port, sock_db);
	}

	if (0>success) return -1; 

	/*retransmit the packet to database*/
	if(multi_send(sock_db,buf_send,send_len,0)==-1) 
	{
		success = 0;
		perror("error@database.c:SendPacketDatabase:send\n");
		exit(1);
    }
#ifdef DEBUG_TRANSMIT_DATABASE_AFFAIR
	fflush(NULL);
	printf("send to database AFFAIR packet length= |%d| \n", send_len);
	printf("send pkt to database affair packet is |%s| \n", buf_send);
	fflush(NULL);
#endif
	/*recieve the feedback packet from database*/
	recv_len = multi_recv(sock_db, buf_recv, MAXPACKETSIZE, 0);
	if(recv_len<0)
	{
		success = 0;
		perror("error@database.c:SendPacketDatabase:recv\n");
		exit(1);
	}
	*recv_db_len = recv_len;
	memcpy(buf_recv_db, buf_recv, recv_len); 
#ifdef DEBUG_TRANSMIT_DATABASE_AFFAIR
	fflush(NULL);
	printf("recv from database AFFAIR packet length = |%d| \n", recv_len);
	printf("recv from database AFFAIR packet is |%s| \n", buf_recv);
	fflush(NULL);
#endif

   	/*close this socket*/
   	close(sock_db);
#else
	int success = 1;
	memcpy(buf_recv_db, buf_send, send_len);
	*recv_db_len = send_len;
	//FILE *fp;
	//fp= fopen("1.txt", "wb");
	//fwrite(buf_send, 1, strlen(buf_send), fp);
	//fclose(fp);
	//printf("recv from business is : |%d|,            |%s|\n", strlen(buf_send), buf_send);
	//printf("not send to database!!!!\n");
#endif

	return success;	
}

/*************************************************************************
 *  \brief
 *    Send the packet to database 
 *
 *  \par Input:
 *     buf_send: the buffer pointer to the packet which is to be sent.
 *     send_len: the length of this packet.
 *     buf_recv_db: the recieve buffer from DB.
 *     recv_db_len: the length of feedback packet from DB.
 *  \par Output:
 *  \Return:
 *     1: success
 *    <=0: the function has some error
************************************************************************/
int SendNonsignificantPacketDatabase(char *buf_send, int send_len, char *buf_recv_db, int *recv_db_len)
{
#ifdef DEBUG_SEND_PACKET_DATABASE
	int success = 0;

	/*varibles for connect database*/
	int sock_db;
	 
	char buf_recv[MAXPACKETSIZE];
	int recv_len;
	
	bzero(buf_recv, MAXPACKETSIZE);

	/*Connection to database control server*/
   sock_db = CreateSocket();
   	
   /*judge the destination of the packet, read some share varibles*/

   do{
    	/*judge the destination of the packet, read some share varibles*/
  		if(1 == JudgePrimaryDatabaseServer())
   		{
   			/*primary database is alive now*/
   			success = ConnectServerOneTime(global_par.system_par.database_self_ip_address, global_par.system_par.database_query_port, sock_db);
   		}
   		else
   		{
   			/*primary database is crush now*/
   			success = ConnectServerOneTime(global_par.system_par.database_brother_ip_address, global_par.system_par.database_query_port, sock_db);
		}
   }while(0>success);

	/*retransmit the packet to database*/
	if(multi_send(sock_db,buf_send,send_len,0)==-1) 
	{
		success = 0;
		perror("error@database.c:SendPacketDatabase:send\n");
		exit(1);
    }
#ifdef DEBUG_TRANSMIT_DATABASE_QUERY
	fflush(NULL);
	printf("send to database QUERY packet length= |%d| \n", send_len);
	printf("send to database QUERY packet is |%s| \n", buf_send);
	fflush(NULL);
#endif
	/*recieve the feedback packet from database*/
	recv_len = multi_recv(sock_db, buf_recv, MAXPACKETSIZE, 0);
	if(recv_len<0)
	{
		success = 0;
		perror("error@database.c:SendPacketDatabase:recv\n");
		exit(1);
	}
	*recv_db_len = recv_len;
	memcpy(buf_recv_db, buf_recv, recv_len); 
#ifdef DEBUG_TRANSMIT_DATABASE_QUERY
	fflush(NULL);
	printf("recv from database QUERY packet length = |%d| \n", recv_len);
	printf("recv from database QUERY packet is |%s| \n", buf_recv);
	fflush(NULL);
#endif

   	/*close this socket*/
   	close(sock_db);
#else
	int success = 1;
	memcpy(buf_recv_db, buf_send, send_len);
	*recv_db_len = send_len;
	//FILE *fp;
	//fp= fopen("1.txt", "wb");
	//fwrite(buf_send, 1, strlen(buf_send), fp);
	//fclose(fp);
	//printf("recv from business is : |%d|,            |%s|\n", strlen(buf_send), buf_send);
	//printf("not send to database!!!!\n");
#endif

	return success;	
}

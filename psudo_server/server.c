/*********************************************************
 *project: Line communication charges supermarket
 *filename: server.c
 *version: 1.0
 *purpose: Simulate company server to test connection from business server
 *developer: gexiaodan, Xi'an Jiaotong University
 *data: 2006-1-6
 *********************************************************/

#include "server.h"

/* *************************************************
 * Function Name: 
 * 		int Connect_Server_Retry(int sockfd, const struct sockaddr *addr, socklen_t alen)
 *
 * Input:
 *		int sockfd: the socket for setup the control channel with proxy
 *		const struct sockaddr *addr: the structure contain the destination IP and port
 *		socklen_t alen: the length of the structure sockaddr 
 *
 * Output:
 *		none
 *
 * Return:
 * 		1 ---> success
 * 		0 ---> error
 * *************************************************/
//int connect_server_retry(int sockfd, const struct sockaddr *addr, socklen_t alen)
//{
//	int nsec;

//	for(nsec = 1; nsec <= MAXSLEEP; nsec <<=1){
//		if(connect(sockfd,addr,alen)==0){
//			return 0;
//		}
//		if(nsec<=MAXSLEEP/2)
//			sleep(nsec);
//	}
//	return -1;
//}

/* ************************************************* 
 * Function Name: 
 * 		int Init_data_comm_client(int client_data_port, struct sockaddr_in &sa)
 * Input: 
 * 		int client_data_port ---> Open the port for receiving client connection
 * Output: 
 * 		socket description ---> success
 * 		-1 ---> error
 * *************************************************/
int Init_data_comm_server(int client_data_port,struct sockaddr_in *sa)
{
	int sd_to_clients = 0;

	/* Check input parameters */
	if(client_data_port<1024){
		perror("error:proxy_init.c:Init_data_comm_client()\n");
		return -1;
	}

	/* Prepare the socket */
	if((sd_to_clients = socket(AF_INET,SOCK_STREAM,0))<0){
		perror("error:proxy_init.c:Init_data_com_client():sd_to_clients\n");
		return -1;
	}
	bzero(sa,sizeof(struct sockaddr_in));
	sa->sin_family = AF_INET;
	sa->sin_port = htons(client_data_port);
	if(INADDR_ANY)
		sa->sin_addr.s_addr = htonl(INADDR_ANY);
	
	/*!	permit any IP address to send request */
	while (bind(sd_to_clients,(struct sockaddr *) sa, sizeof(struct sockaddr))<0){
		perror("error:proxy_int.c:Init_data_com_client():bind");
		sleep(5);
	}

	/* Listen the port for clients */
	listen(sd_to_clients, BACKLOG_DATA_CHANNEL);

	return sd_to_clients;
}

 /* ************************************************* 
 * Function Name: 
 * 		int Init_data_comm_busi_server(char *business_ip_address,int business_data_port, struct sockaddr_in *sa)
 * Input: 
 * 		char *business_ip_address ---> IP address of business 
 * 		int business_data_port ---> data port of business
 * Output:
 * 		struct sockaddr_in *sa ---> the initialized data
 * 		1 ---> success
 * 		-1 ---> error
 * *************************************************/
//int Init_data_comm_client(char *ip_address,int data_port, struct sockaddr_in *sa)
//{
//	/* Initialize the structure of socket */
//	bzero(sa,sizeof(struct sockaddr_in));
//	sa->sin_family = AF_INET;
//	sa->sin_port = htons(data_port);

//	sa->sin_addr.s_addr = inet_addr(ip_address);
//	return 1;
//}

/*************************************************************************
 *  \brief
 *    The main function
 *	  1. This is a company server testing program
 *    2. create a socket server for business
 *    3. fork a new process handle query of business
 *
 *  \par Input:
 *
 *  \par Output:
 *
 *  \Return:
 *    0: success
 *    1: failure
************************************************************************/   	
int main()
{
	/*variables for accountant server*/
	int sock, new_sock;
    	struct sockaddr_in local_addr;
	pid_t pid;
	
	/*variables for china bank server*/
	//int sd_to_bank;
	//struct sockaddr_in sa_bank;
	
	int sin_size = sizeof(struct sockaddr_in);
	
	/*variable for packet to send and receive*/
	char buf_pkt[MAXPACKETSIZE]; /*buffer for response packet to send*/
	int count = 0;
	
	/*initial server for financial server*/
	sock = Init_data_comm_server(9999, &local_addr);
	
	/*initial client for bank query*/
	//Init_data_comm_client("9.88.32.158", 7990, &sa_bank);
 	
   	/*begin accept the query from business server*/
   	while(1) 
   	{
      		if ((new_sock = accept(sock, (struct sockaddr *)&local_addr, &sin_size)) < 0)
   		{
			perror("accepting query from business server");
   			continue;
   		}
       		
   		/*fork a child process*/
		if((pid = fork()) < 0)
		{
			perror("error:proxy_init.c:Dameon_foreground():fork");
			close(new_sock);
			sleep(1);
			continue;
		}
		else if (0 == pid)
		{
			/* In the first child process*/
			/* Close the listening socket description */
			close(sock);
			if((pid = fork()) < 0)
			{
				perror("proxy_init.c:Daemon_foreground():fork()");
				exit(1);
			}
			else if (pid > 0)
			{
				exit(0);
			}
			
			/*second child*/
			/*receive packet from client*/
			bzero(buf_pkt, MAXPACKETSIZE);
   			if ((count = recv(new_sock, buf_pkt, MAXPACKETSIZE, 0)) <= 0)
			{
				perror("recv data: financial client");
				exit (1);
			}
			//判断收到的包
			char trade_code[20]="";
			memcpy(trade_code, buf_pkt+17, 4);
			if(0==memcmp(trade_code, "1100", 4)){
				//query pkt
				bzero(buf_pkt, MAXPACKETSIZE);
				strcpy(buf_pkt, "00144AD001A7100491100000913212345678                   测试员              999999999999000000000000000105510000005430000000000000            正常返回");
				memset(buf_pkt+strlen("00144AD001A7100491100000913212345678                   测试员              999999999999000000000000000105510000005430000000000000            正常返回"), 0x0a, 1);
				printf("query pkt:1100\n");
			}else if(0==memcmp(trade_code, "2100", 4)){
				//charge pkt
				bzero(buf_pkt, MAXPACKETSIZE);
				strcpy(buf_pkt, "00297AD001A7100652100000913212345678                   201301270013012739477825    测试员              AAAAAA-AAAAAA  9               000000000000000105510000000100000001065103下期已产生话费0000005430本期可透支额度0000000000积分          00000008110000000000000000000000000000543000            正常返回");
				memset(buf_pkt+strlen("00297AD001A7100652100000913212345678                   201301270013012739477825    测试员              AAAAAA-AAAAAA  9               000000000000000105510000000100000001065103下期已产生话费0000005430本期可透支额度0000000000积分          00000008110000000000000000000000000000543000            正常返回"), 0x0a, 1);
				printf("charge pkt:2100\n");
			}else if(0==memcmp(trade_code, "2300", 4)){
				//reversal pkt
				bzero(buf_pkt, MAXPACKETSIZE);
				strcpy(buf_pkt, "00130AD001A7100652300000913212345678                   20130127测试员              00000001000013012739477825    00            正常返回");
				memset(buf_pkt+strlen("00130AD001A7100652300000913212345678                   20130127测试员              00000001000013012739477825    00            正常返回"), 0x0a, 1);
				printf("reversal pkt:2300\n");
			}
			
			/*send packet to finincial client*/
			if ((count = send(new_sock, buf_pkt, strlen(buf_pkt), 0)) < 0)
			{
				perror("send data: financial client");
				exit (1);
			}

        		close(new_sock);
        		//close(sd_to_bank);
        		exit(0);

		}

		/* In the parent process */
		close(new_sock);
			
		if(waitpid(pid,NULL,0)!=pid)
		{
			perror("server.c:waitpid");
		}	
	}
	
	return 0;
}


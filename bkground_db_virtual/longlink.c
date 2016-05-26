/*********************************************************
 *project: Line communication charges supermarket
 *filename: longlink.c
 *version: 0.4
 *purpose: some function use for LC deamon process 
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-9
 *********************************************************/
#include "longlink.h"

/* *************************************************
 * Function Name: 
 * 		PGconn *Connect_db_server(char *user_name, char *password,char *db_name,char *ip_addr)
 * Input: 
 * 		char *ip_addr;
 * 		char *user_name;
 * 		char *password;
 * 		char *db_name;
 * Output:
 * 		PGconn *conn ---> success
 *		NULL ---> failure
 * *************************************************/
PGconn *Connect_db_server(char *user_name, char *password,char *db_name,char *ip_addr)
{
	PGconn     *conn;
	char conn_string[100];

	/* Check input parameters */
	if(NULL==ip_addr||NULL==user_name||NULL==password||NULL==db_name) {
		perror("error@longlink.c:Connect_db_server:NULL==ip_addr");
		return NULL;
	}

	bzero(conn_string,100);
	sprintf(conn_string,"user=%s password=%s dbname=%s hostaddr=%s",user_name,password,db_name,ip_addr);

	/* Connect the database */
	conn = PQconnectdb(conn_string);

	if (PQstatus(conn) == CONNECTION_BAD) {
		perror("error@longlink.c:Connect_db_server()");
		return NULL;
	}

	return conn;
}

int connect_server_retry(int sockfd, const struct sockaddr *addr, socklen_t alen)
{
	int nsec;

	for(nsec = 1; nsec <= MAXSLEEP; nsec <<=1){
		if(connect(sockfd,addr,alen)==0){
			return 0;
		}
		if(nsec<=MAXSLEEP/2)
			sleep(nsec);
	}
	return -1;
}

int Init_comm_server(char *ip_address,int data_port, struct sockaddr_in *sa)
{
	/* Initialize the structure of socket */
	bzero(sa,sizeof(struct sockaddr_in));
	sa->sin_family = AF_INET;
	sa->sin_port = htons(data_port);

	sa->sin_addr.s_addr = inet_addr(ip_address);
	return 1;
}


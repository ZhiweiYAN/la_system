/*********************************************************
 *project: Line communication charges supermarket
 *filename: recvsend.c
 *version: 1.0
 *purpose: receive and send data
 *developer: gexiaodan, Xi'an Jiaotong University(Drum Team)
 *data: 2007-4-23
 *********************************************************/
#include "multi_recvsend.h"

/* *************************************************
 *  \brief
 *    receive all of the data(< MAXPACKETSIZE)
 *  
 * Function Name: 
 * 		int multi_recv(int socket_fd, char *buf_recv, size_t recv_len, int flags)  
 *
 * Input: 
 *		int socket_fd ---> socket
 *		size_t recv_len ---> length of received data(MAXPACKETSIZE)
 *		int flags ---> flags parameter
 *
 * Output:
 *      char *buf_recv ---> buffer of received data
 *
 * Return: 
 *		>=0 ---> real receive packet length
 *		-1 ---> failure
 *		
 * *************************************************/
int multi_recv(int socket_fd, char *buf_recv, size_t recv_len, int flags)  
{
	int count = 0;
	int total_count = 0;
	char buf_temp[11] = "00";
	size_t recv_packet_len = 0;

	bzero(buf_temp, 11);
	/*recv the packet length*/
	if ((count = recv(socket_fd, (void*)(buf_temp), 10, flags)) < 0)
	{
		/*error occur*/
		perror("multi_recvsend.c:multi_recv():recv1()");
		return -1;
	}
	recv_packet_len = strtoul(buf_temp, NULL, 10);
	
	/*send ok when recv packet length*/
	if ((count = send(socket_fd, "OK", 2, flags)) < 0)
	{
		/*error occur*/
		perror("multi_recvsend.c:multi_recv():Send()");
		return -1;
	}	
		
	/*judge whether buf_rev has enough space to save, when larger than MAXPACKETSIZE, recv MAXPACKETSIZE-1*/
	if (recv_packet_len > (MAXPACKETSIZE - 1))
	{
		printf("\033[01;31merror@multi_recvsend.c:multi_recv():The buffer of data channel is too small, please increase MAXPACKETSIZE!\033[0m\n");
		recv_packet_len = MAXPACKETSIZE - 1;
	}

	/*recv packet that needed*/
	bzero(buf_recv, MAXPACKETSIZE);
	while (recv_packet_len > (unsigned int )total_count)  
	{
		/*recv packet one time*/
        if ((count = recv(socket_fd, (void*)(buf_recv + total_count), recv_len, flags)) < 0)
        {
			perror("error@multi_recvsend.c:multi_recv():recv2()");
			return -1;
        }			
		total_count += count;
	}
	
	return total_count;
}

/* *************************************************
 *  \brief
 *    send all of the data(< MAXPACKETSIZE)
 *  
 * Function Name: 
 * 		int multi_send(int socket_fd, const char *buf_send, size_t send_len, int flags)  
 *
 * Input: 
 *		int socket_fd ---> socket
 *		const char *buf_send ---> buffer of send data
 *		size_t send_len ---> length of send data(equal to unsigned int)
 *		int flags ---> flags parameter
 *
 * Output:
 *      none
 *
 * Return: 
 *		>=0 ---> real send packet length
 *		-1 ---> failure
 * *************************************************/
int multi_send(int socket_fd, const char *buf_send, size_t send_len, int flags)  
{
	char buf_temp[11] = "00";
	int count = 0;

	/*Add by yanzw BEGIN*/
	/*if buf_send = NULL, it is a triger packet*/
	if(strncmp("NULL",buf_send,4)==0) {
		count = send(socket_fd,"NULL",4,0);
		return count;
	}
	/*Add by yanzw END */
	
	/*judge whether send_len is larger than MAXPACKETSIZE*/
	if (send_len > (MAXPACKETSIZE - 1))
	{
		printf("\033[01;31merror@multi_recvsend.c:multi_send():The buffer of data channel is too small, please increase MAXPACKETSIZE!\033[0m\n");
		send_len = MAXPACKETSIZE - 1;
	}

	/*send the packet length*/
	bzero(buf_temp, 11);
	sprintf(buf_temp, "%u", send_len);
	if ((count = send(socket_fd, (void*)(buf_temp), 10, flags)) < 0)
	{
		/*error occur*/
		perror("multi_recvsend.c.c:multi_send():Send1()");
		return -1;
	}

	/*recv response(OK)*/
	bzero(buf_temp, 11);
	if ((count = recv(socket_fd, (void*)(buf_temp), 10, flags)) < 0)
	{
		/*error occur*/
		perror("multi_recvsend.c.c:multi_send():recv()");
		return -1;
	}

	/*send packet that needed*/
	if ((count = send(socket_fd, (void*)(buf_send), send_len, flags)) < 0)
	{
		/*error occur*/
		perror("multi_recvsend.c:multi_send():Send2()");
		return -1;
	}
		
	return count;
}


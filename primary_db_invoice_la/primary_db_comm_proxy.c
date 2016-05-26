/* *************************************************
 * File name:
 * 		primary_db_comm_proxy.cc
 * Description:
 * 		The program is run at the primary database server.
 * Author:
 * 		Zhiwei Yan, jerod.yan@gmail.com
 * Date:
 * 		2012-12-14
 * *************************************************/
#include "primary_db_comm_proxy.h"
/* *************************************************
* Function Name:
* 		int Send_message_to_proxy(char *msg, int msg_length)
* Input:
* 		NONE;
* Ouput:
* 		1 ---> success
* 		-1 ---> failure
* *************************************************/
int Send_message_to_proxy(char *msg, int msg_length)
{
    /*variable for proxy*/
    int i = 0;
    int proxy_sd;/*socket for proxy*/
    struct sockaddr_in proxy_sa;/* information for proxy*/

    char proxy_address_array[MAX_PROXY_NUMBER][16];
    int proxy_control_port = 0;
    int success = 0;

    /* Get proxy address and communication with proxy */
    success = Read_proxy_parameters((char*)(proxy_address_array),&proxy_control_port);
    for (i=0; i<global_par.system_par.proxy_number; i++) {
        if ((proxy_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("error:@primary_db_comm_proxy.cc:Send_message_to_proxy()01");
            LOG(ERROR)<<"socket initialization in Send_message_to_proxy, failed.";
            return -1;
        }

        /*set information for proxy*/
        bzero(&proxy_sa, sizeof(proxy_sa));
        proxy_sa.sin_family = AF_INET;
        proxy_sa.sin_port = htons(proxy_control_port);
        //	proxy_sa.sin_addr.s_addr = inet_addr(proxy_address_array[i]);
        proxy_sa.sin_addr.s_addr = inet_addr(global_par.system_par.proxy_ip_addr_array[i]);
        /*connect to proxy*/
        if (connect(proxy_sd, (struct sockaddr*)&proxy_sa, sizeof(proxy_sa)) < 0) {
            perror("error:@primary_db_comm_proxy.cc:Send_message_to_proxy()02");
            LOG(ERROR)<<"failed to connect proxy server "<< global_par.system_par.proxy_ip_addr_array[i]
                      <<":"<<proxy_control_port;
            close(proxy_sd);
            continue;
        }

        LOG(INFO)<<"successfully connect to proxy server" << global_par.system_par.proxy_ip_addr_array[i]
                 <<":" <<proxy_control_port;
        /* Send the message */
        if (send(proxy_sd, msg, msg_length, 0) < 0) {
            perror("error:@primary_db_comm_proxy.cc:Send_message_to_proxy()03");
            LOG(ERROR)<<"failed to send msg to proxy server: "<< msg;
            close(proxy_sd);
            continue;
        }
        /* Recv a message */
        /*if (recv(proxy_sd, msg, msg_length, 0) < 0) {
        	perror("error:@primary_db_comm_proxy.cc:Send_message_to_proxy()04");
        	close(proxy_sd);
        	return -1;
        }*/

        close(proxy_sd);
        LOG(INFO)<<"successfully send msg to proxy server:" << msg;
    }
    return 1;
}

/* *************************************************
* Function Name:
* 		int Read_proxy_parameters(char *proxy_addr,int *proxy_control_port)
* Input:
* 		NONE;
* Ouput:
* 		1 ---> success
* 		-1 ---> failure
*		char *proxy_addr
*       int *proxy_control_port
* *************************************************/
int Read_proxy_parameters(char *proxy_addr_array,int *proxy_control_port)
{
    *proxy_control_port = global_par.system_par.proxy_control_port;
    memcpy(proxy_addr_array,global_par.system_par.proxy_ip_addr_array,MAX_PROXY_NUMBER*16);
    return 1;
}

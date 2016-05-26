/* *************************************************
 * File name:
 * 		primary_db_comm_database.cc
 * Description:
 * 		The program is run at the primary database server.
 * 		It receives the data request packet from Slave database.
 * Author:
 * 		Zhiwei Yan, jerod.yan@gmail.com
 * Date:
 * 		2013-01-18
 * *************************************************/
#include "primary_db_comm_database.h"
/* *************************************************
* Function Name:
* 		int Send_message_to_database_record_port(char *msg, int msg_length)
* Input:
* 		NONE;
* Ouput:
* 		1 ---> success
* 		-1 ---> failure
* *************************************************/
int Send_message_to_database_record_port(char *msg, int msg_length)
{
    /*variable for database*/
    int database_sd;/*socket for database*/
    struct sockaddr_in database_sa;/* information for database*/

    if(NULL==msg) {
        LOG(ERROR)<<"The pointer of char *msg for record of input parameters is NULL.";
        return -1;
    }

    /* Get database address and communication with database */
    if ((database_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOG(ERROR)<<"Socket initialization, error, Send_message_to_database_record_port()";
        perror("error@primary_db_comm_database.cc:Send_message_to_database()01");
        return -1;
    }

    /*set information for database*/
    bzero(&database_sa, sizeof(database_sa));
    database_sa.sin_family = AF_INET;
    database_sa.sin_port = htons(global_par.system_par.database_data_port);
    database_sa.sin_addr.s_addr = inet_addr(global_par.system_par.database_self_ip_address);

    /*connect to database*/
    if (connect(database_sd, (struct sockaddr*)&database_sa, sizeof(database_sa)) < 0) {
        perror("error@primary_db_comm_database.cc:Send_message_to_database()02");
        LOG(ERROR)<<"failed to connect to database:"<<global_par.system_par.database_self_ip_address
                  << ":" << global_par.system_par.database_data_port;
        close(database_sd);
        return -1;
    }
    LOG(INFO)<<"successfuly to connect to database:"
             <<global_par.system_par.database_self_ip_address
             << ":" << global_par.system_par.database_data_port;

    /* Send the message */
    if (multi_send(database_sd, msg, msg_length, 0) < 0) {
        perror("error@primary_db_comm_database.cc:Send_message_to_database()03");
        LOG(ERROR)<<"failed multi_send to database record message: "<<msg;
        close(database_sd);
        return -1;
    }
    close(database_sd);
    LOG(INFO)<<"successfully multi_send to database record message: "<<msg;
    return 1;
}

/* *************************************************
* Function Name:
* 		int Send_message_to_database_view_port(char *msg, int msg_length)
* Input:
* 		NONE;
* Ouput:
* 		1 ---> success
* 		-1 ---> failure
* *************************************************/
int Send_message_to_database_view_port(char *msg, int msg_length)
{
    /*variable for database*/
    int database_sd;/*socket for database*/
    struct sockaddr_in database_sa;/* information for database*/

    if(NULL==msg) {
        LOG(ERROR)<<"The pointer of char *msg for view port of input parameters is NULL.";
        return -1;
    }

    /* Get database address and communication with database */
    if ((database_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOG(ERROR)<<"Socket initialization, error, Send_message_to_database_view_port()";
        perror("error@primary_db_comm_database.cc:Send_message_to_database_view_port()01");
        return -1;
    }

    /*set information for database*/
    bzero(&database_sa, sizeof(database_sa));
    database_sa.sin_family = AF_INET;
    database_sa.sin_port = htons(global_par.system_par.database_query_port);
    database_sa.sin_addr.s_addr = inet_addr(global_par.system_par.database_self_ip_address);

    /*connect to database*/
    if (connect(database_sd, (struct sockaddr*)&database_sa, sizeof(database_sa)) < 0) {
        LOG(ERROR)<<"failed to connect to database:"<<global_par.system_par.database_self_ip_address
                  << ":" << global_par.system_par.database_query_port;
        perror("error@primary_db_comm_database.cc:Send_message_to_database_view_port()02");
        close(database_sd);
        return -1;
    }
    LOG(INFO)<<"successfuly to connect to database:"
             <<global_par.system_par.database_self_ip_address
             << ":" << global_par.system_par.database_query_port;

    /* Send the message */
    if (multi_send(database_sd, msg, msg_length, 0) < 0) {
        perror("error@primary_db_comm_database.cc:Send_message_to_database_view_port()03");
        LOG(ERROR)<<"failed multi_send to database message: "<<msg;
        close(database_sd);
        return -1;
    }
    close(database_sd);
    LOG(INFO)<<"successfully multi_send to database query message: "<<msg;
    return 1;
}
/* *************************************************
* Function Name:
* 		int Send_message_to_database_cas_port(char *msg, int msg_length)
* Input:
* 		NONE;
* Ouput:
* 		1 ---> success
* 		-1 ---> failure
* *************************************************/
int Send_message_to_database_cas_port(char *msg, int msg_length)
{
    /*variable for database*/
    int database_sd;/*socket for database*/
    struct sockaddr_in database_sa;/* information for database*/

    if(NULL==msg) {
        LOG(ERROR)<<"The pointer of char *msg for cas port of input parameters is NULL.";
        return -1;
    }
    /* Get database address and communication with database */
    if ((database_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOG(ERROR)<<"Socket initialization, error, Send_message_to_database_cas_port()";
        perror("error@primary_db_comm_database.cc:Send_message_to_database_cas_port()01");
        return -1;
    }

    /*set information for database*/
    bzero(&database_sa, sizeof(database_sa));
    database_sa.sin_family = AF_INET;
    database_sa.sin_port = htons(global_par.system_par.business_virtual_money_port);
    database_sa.sin_addr.s_addr = inet_addr(global_par.system_par.database_self_ip_address);

    /*connect to database*/
    if (connect(database_sd, (struct sockaddr*)&database_sa, sizeof(database_sa)) < 0) {
        perror("error@primary_db_comm_database.cc:Send_message_to_database_cas_port()02");
        LOG(ERROR)<<"failed to connect to database cas port:"<<global_par.system_par.database_self_ip_address
                  << ":" << global_par.system_par.business_virtual_money_port;
        close(database_sd);
        return -1;
    }

    LOG(INFO)<<"successfully to connect to database cas port:"<<global_par.system_par.database_self_ip_address
             << ":" << global_par.system_par.business_virtual_money_port;
    /* Send the message */
    //if (multi_send(database_sd, msg, msg_length, 0) < 0) {
    if (send(database_sd, msg, msg_length, 0) < 0) {
        perror("error@primary_db_comm_database.cc:Send_message_to_database_cas_port()03");
        LOG(ERROR)<<"failed to send to database message: "<<msg;
        close(database_sd);
        return -1;
    }
    close(database_sd);
    LOG(INFO)<<"successfully send to database cas message: "<<msg;
    return 1;
}

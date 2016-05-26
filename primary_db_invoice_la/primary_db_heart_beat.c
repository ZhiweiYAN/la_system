/*
 * =====================================================================================
 *
 *       Filename:  primary_db_heart_beat.c
 *
 *    Description:  Recv or send heart_beat packets
 *
 *        Version:  1.0
 *        Created:  6/12/2010 8:54:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Zhi-wei YAN (Jerod YAN), jerod.yan@gmail.com
 *        Company:  DrumTm
 *
 * =====================================================================================
 */

#include "primary_db_heart_beat.h"

/* *************************************************
 * Function Name:
 * 		int Send_heart_beat_packet(void)
 * Input:
 * 		NONE;
 * Output:
 * 		1 ---> success;
 * 		-1 ---> success;
 * *************************************************/
int Send_heart_beat_packet(void)
{
    enum ServerMode srv_mode = READY;
    char msg[HEART_BEAT_PACKET_SIZE] = "00";
    long int rnd_number = 0;
    char tmp_str[TEMP_LENGTH];
    int success = 0;

    /*variable for brother database*/
    int brother_db_sd;/*socket for brother database*/
    struct sockaddr_in brother_db_sa;/* information for brother database*/

    char brother_db_address[TEMP_LENGTH] = "0.0.0.0";
    int brother_db_control_port = 0;

    /* Get database brother address and communication with brother database */
    bzero(brother_db_address,TEMP_LENGTH);
    success = Read_brother_parameters_network(brother_db_address, &brother_db_control_port);
    LOG(INFO)<<"Brother_DB IP and heartbeat port:"<<brother_db_address
             <<":"<<brother_db_control_port;

    /*set information for brother database*/
    bzero(&brother_db_sa, sizeof(brother_db_sa));
    brother_db_sa.sin_family = AF_INET;
    brother_db_sa.sin_port = htons(brother_db_control_port);
    brother_db_sa.sin_addr.s_addr = inet_addr(brother_db_address);

    /*connect to brother database*/
    while (1) {

        if ((brother_db_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            OUTPUT_ERROR;
            close(brother_db_sd);
            sleep(DELAY_MIN_TIME);
            continue;
        }

        if (connect(brother_db_sd, (struct sockaddr*)&brother_db_sa, sizeof(brother_db_sa))<0) {
            //OUTPUT_ERROR;
            close(brother_db_sd);
            perror("Failed: Connect to the Brother Database Server:");
            LOG(ERROR)<<"Failed to connect to brother database with control port -> "
                      <<brother_db_address<<":"<<brother_db_control_port;
            sleep(DELAY_MIN_TIME);
            continue;

        }
        bzero(msg, HEART_BEAT_PACKET_SIZE);
        bzero(tmp_str,TEMP_LENGTH);

        /* Get the server mode */
        srv_mode=Get_ownself_server_mode();
        switch (srv_mode) {
        case TWINS:
            strcpy(msg,"TWINS");
            break;
        case ALONE:
            strcpy(msg,"ALONE");
            break;
        case ERROR:
            strcpy(msg,"ERROR");
            break;
        case PRISYNC:
        case SYNC:
            strcpy(msg,"SYNC");
            break;
        case CHECK:
            strcpy(msg,"CHECK");
            break;
        default:
            strcpy(msg,"READY");
            break;
        };

        /* Produce a rand number 0001 ~ 9999*/
        rnd_number = Produce_rand_number();
        sprintf(tmp_str,"_%04ld",rnd_number);
        strcat(msg,tmp_str);

        if (send(brother_db_sd, msg, strlen(msg), 0) > 0) {
            success = Set_process_life_time(getpid(),0);
            DBG("HB_SEND:|%s|\n",msg);

        }

        close(brother_db_sd);

        sleep(1);
        /* Loop to send the hear beat packet */
    }
    return 1;

}


/* *************************************************
 * Function Name:
 *  	int Send_heart_beat_packet_defuncted(char *msg, int msg_length,char *database_ip, int database_port )
 * Input:
 * 		char *msg ---> the content of the heart beat
 * 		int msg_length ---> the length of the message
 * 		char *database_ip ---> IP address
 * 		int database_port ---> port
 * Output:
 * 		1 ---> success;
 * 		-1 ---> failed;
 * *************************************************/
int Send_heart_beat_packet_defuncted(char *msg, int msg_length,char *database_ip, int database_port )
{
    /*variable for database*/
    int database_sd;/*socket for database*/
    struct sockaddr_in database_sa;/* information for database*/

    char database_address[TEMP_LENGTH] = "0.0.0.0";
    int database_control_port = 0;

    /* Get database address and communication with database */
    strcpy(database_address, database_ip);
    database_control_port = database_port;


    if ((database_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        OUTPUT_ERROR;
        return -1;
    }

    /*set information for database*/
    bzero(&database_sa, sizeof(database_sa));
    database_sa.sin_family = AF_INET;
    database_sa.sin_port = htons(database_control_port);
    database_sa.sin_addr.s_addr = inet_addr(database_address);

    /*connect to database*/
    if (connect(database_sd, (struct sockaddr*)&database_sa, sizeof(database_sa)) >0) {
        if (send(database_sd, msg, msg_length, 0) < 0)
            OUTPUT_ERROR;

    }
    close(database_sd);
    return 1;
}
/* *************************************************
 * Function Name:
 * 		int Receive_heart_beat_packet(void)
 * Input:
 * 		NONE
 * Output:
 * 		1 ---> success;
 * 		-1 ---> success;
 * *************************************************/
int Receive_heart_beat_packet(void)
{

    int success = 0;
    int welcome_sd = 0;
    int connection_sd = 0;
    struct sockaddr_in sa;
    int port = 0;
    socklen_t len = 0;
    char buf_recv[HEART_BEAT_BUFFER_SIZE];
    int count = 0;
    int reuse =1;

    port = global_par.system_par.database_heartbeat_port;
    /* Create a socket */
    if ((welcome_sd=socket(AF_INET,SOCK_STREAM,0))<0) {
        perror("error@pri_db_heart_beat.cc:Receive_heart_beat():socket");
        LOG(ERROR)<<"socket initialization, failed.";
        return -1;
    }
    /* Bind the socket to the port, and allow all IP connect with it */
    bzero(&sa,sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    if (INADDR_ANY){
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    if (setsockopt(welcome_sd,SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof(int))<0) {
        perror("error:pri_db_heart_beat.c:Receive_heart_beat_packet():setsockopt()");
        LOG(ERROR)<<"error:pri_db_heart_beat.c:Receive_heart_beat_packet():setsockopt()";
    }
    while (1) {
        if (bind(welcome_sd,(struct sockaddr *)&sa,sizeof(sa))<0) {
            perror("error@pri_db_heart_beat.cc:Receive_heart_beat():bind");
            sleep(5);
        } else {
            break;
        }
        printf("\rContinuing to bind the port for receiving heart beart packet.\n");
    }
    listen(welcome_sd,HEART_BEAT_BACKLOG);
    len = sizeof (sa);
    fflush(NULL);

    /* Receving only one message from Slave Database */
    LOG(INFO)<<"Ready to accept the heartbeat pkt form brother server at port: "<<port;
    google::FlushLogFiles(google::INFO);
    while (1) {

        if (( connection_sd = accept(welcome_sd,(struct sockaddr*)&sa,&len))<0) {
            perror("error:pri_db_heart_beat.cc:Receive_heart_beat():accept");
            LOG(ERROR)<<"failed to accept a connection from outside.";
            sleep(DELAY_MIN_TIME);

        }

        bzero(buf_recv,HEART_BEAT_BUFFER_SIZE);
        count = recv(connection_sd,buf_recv,HEART_BEAT_BUFFER_SIZE,0);
        if (count>0) {
            success = Set_process_life_time(getpid(),0);
            LOG(INFO)<<"Heart beat pkt from brother DB server:" << buf_recv;
        }
        close(connection_sd);

        google::FlushLogFiles(google::ERROR);
        google::FlushLogFiles(google::INFO);

    }

    return 1;
}


/* *************************************************
 * Function Name:
 * 		long int Produce_rand_number(void)
 * Input:
 * 		NONE
 * Output:
 * 		1 ---> success;
 * 		-1 ---> success;
 * *************************************************/
long int Produce_rand_number(void)
{
    long int n = 0;
    time_t t = 0;
    t = time(NULL);
    srand((unsigned int) t);
    n = 1 + (int) (9999.0 * rand() / (RAND_MAX + 1.0));
    return n;
}

/* *************************************************
 * Function Name:
 * 		success = Read_parameters_network(ip_addr, port);
 * Input:
 * 		IP address
 * 		port
 * Output:
 * 		1 ---> success;
 * 		-1 ---> success;
 * *************************************************/
int Read_brother_parameters_network(char *ip_addr, int *port)
{
    if (MAIN_DB==Get_ownself_server_type()) {
        strcpy(ip_addr,global_par.system_par.database_brother_ip_address);
    }
    if (SLAVE_DB==Get_ownself_server_type()) {
        strcpy(ip_addr,global_par.system_par.database_self_ip_address);
    }

    *port = global_par.system_par.database_heartbeat_port;
    return 1;

}

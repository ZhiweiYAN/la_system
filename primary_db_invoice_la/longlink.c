/*********************************************************
 *project: Line communication charges supermarket
 *filename: longlink.c
 *version: 0.4
 *purpose: some function use for LC deamon process
 *developer: ssurui, Xi'an Jiaotong University (Drum Team)
 *data: 2007-1-9
 *********************************************************/
#include "longlink.h"

/*************************************************************************
 *  \brief
 *    use socket which is created in parent process
 *    connect to a enterprise server in children process
 *  \par Input:
 *    server_ip_address: server IP
 *    server_port: server port
 *    sockid: socket which is created in parent process.
 *  \par Output:
 *
 *  \Return:
 *    >0: success, the value is the socket of long link
 *    -1: fail
************************************************************************/
int ConnectServer(char *server_ip_address, int server_port, int sockid)
{
    /*varible for connect unicom server*/
    struct hostent *server_name;
    struct sockaddr_in server_addr;

    bzero(&server_addr,sizeof server_addr);

    /*get the unicome hostname or ip addr*/
    if ((server_name = gethostbyname(server_ip_address)) == NULL) {
        perror("error@business.c:ConnectServer:gethostbyname\n");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = ((struct in_addr *)(server_name->h_addr))->s_addr;

    /*use the random backward agorithm to retry seting up the control channel with unicom
         until the server accept my connection*/
    while (0 > Connect_Server_Retry(sockid, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) {
        printf("\rCan't connect to server (%s, %d), PID: %d will try again!\n", server_ip_address, server_port, getpid());
    }

    return 1;
}

/*************************************************************************
 *  \brief
 *    before setup a server, first bind server port
 *  \par Input:
 *    server_port: server port
 *    sockid: socket which is created in parent process.
 *  \par Output:
 *
 *  \Return:
 *    >0: success, the value is the socket of long link
 *    -1: fail
************************************************************************/
int BindServerPort(int server_port, int sockid)
{
    int success = -1;
    int reuse =1;
    struct sockaddr_in local_addr; /* information for localhost */

    bzero(&local_addr,sizeof local_addr);
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(server_port);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    //if ((success = bind(sockid, (struct sockaddr *)&local_addr, sizeof(struct sockaddr))) == -1)
    if (setsockopt(sockid,SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof(int))<0) {
        perror("error:longlink.c:BindServerPort():setsockopt()");
    }
    while (-1 == (success = bind(sockid, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)))) {
        perror("error@longlink.c:BindServerPort\n");
        sleep(MAXSLEEP/2);
        //exit(1);
    }

    return 1;
}

/*************************************************************************
 *  \brief
 *    create a socket by socket() function
 *  \par Input:
 *  \par Output:
 *
 *  \Return:
 *    >0: the socket created.
 *    <=0: fail
************************************************************************/
int CreateSocket(void)
{
    int sockid = -1;
    if ((sockid = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("error@longlink.c:CreateSocket\n");
        exit(1);
    }

    return sockid;
}

/* *************************************************
 *  \brief
 *     use the random backward agorithm to retry seting up the control channel with proxy
 *
 * Input:
 *    sockfd: the socket for setup the control channel with proxy
 *    addr: the structure contain the destination IP and port
 *    alen: the length of the structure sockaddr
 *
 * Ouput:
 *    0: success
 *    -1: fail
 * *************************************************/

int Connect_Server_Retry(int sockfd, const struct sockaddr *addr, socklen_t alen)
{
    int nsec;

    for (nsec = 1; nsec <= MAXSLEEP; nsec <<=1) {
        if (connect(sockfd,addr,alen)==0) {
            return 0;
        }
        if (nsec<=MAXSLEEP/2)
            sleep(nsec);
    }
    return -1;
}

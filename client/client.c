//不上传
#include "head.h"



int server_port = 0;

char server_ip[20] = { 0 };





char* conf = "./football.conf";

int sockfd = -1;

void logout(int sigunm) {
    struct ChatMsg msg;
    msg.type = CHAT_FIN;
    send(sockfd, (void*)&msg, sizeof(msg), 0);
    close(sockfd);
    DBG(RED"Bye!\n"NONE);
    exit(0);
}

int main(int argc, char** argv) {

    int opt;
    struct LogRequest request;
    struct LogResponse response;
    bzero(&request, sizeof(request));
    bzero(&response, sizeof(response));
    while ((opt = getopt(argc, argv, "h:p:t:m:n:")) != -1) {

        switch (opt) {

        case 't':

            request.team = atoi(optarg);

            break;

        case 'h':

            strcpy(server_ip, optarg);

            break;

        case 'p':

            server_port = atoi(optarg);

            break;

        case 'm':

            strcpy(request.msg, optarg);

            break;

        case 'n':

            strcpy(request.name, optarg);

            break;

        default:

            fprintf(stderr, "Usage : %s [-hptmn]!\n", argv[0]);

            exit(1);

        }

    }





    if (!server_port) server_port = atoi(get_conf_value(conf, "SERVERPORT"));

    if (!request.team) request.team = atoi(get_conf_value(conf, "TEAM"));

    if (!strlen(server_ip)) strcpy(server_ip, get_conf_value(conf, "SERVERIP"));

    if (!strlen(request.name)) strcpy(request.name, get_conf_value(conf, "NAME"));

    if (!strlen(request.msg)) strcpy(request.msg, get_conf_value(conf, "LOGMSG"));





    DBG("<"GREEN"Conf Show"NONE"> : server_ip = %s, port = %d, team = %s, name = %s\n%s", \

        server_ip, server_port, request.team ? "BLUE" : "RED", request.name, request.msg);



    struct sockaddr_in server;

    server.sin_family = AF_INET;

    server.sin_port = htons(server_port);

    server.sin_addr.s_addr = inet_addr(server_ip);



    socklen_t len = sizeof(server);



    if ((sockfd = socket_udp()) < 0) {

        perror("socket_udp()");

        exit(1);

    }



    sendto(sockfd, &request.msg, sizeof(request), 0, (struct sockaddr*)&server, len);

    fd_set set;
    FD_ZERO(&set);
    FD_SET(sockfd, &set);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    int retval = select(sockfd + 1, &set, NULL, NULL, &tv);
    if (retval < 0) {
        perror("select()");
        exit(1);
    }
    else if (retval) {
        int ret = recvfrom(sockfd, (void*)&response, sizeof(response), 0, (struct sockaddr*)&server, &len);
        if (ret != sizeof(response) || response.type) {
            DBG(RED"Error"NONE"The Game Server refused your login.\n\tThis May be helpful:%s\n", response.msg);
            exit(1);
        }
    }
    else {
        DBG(RED"Error"NONE"The Game Server is out of service!\n");
            exit(1);
    }
    DBG(GREEN"Server"NONE" : %s\n", response.msg);
    
    connect(sockfd, (struct sockaddr*)&server, len);
   
    pthread_t recv_t;
    pthread_create(&recv_t, NULL, do_recv, NULL);
   
    signal(SIGINT, logout);
    struct ChatMsg msg;
    while (1) {
        bzero(&msg, sizeof(msg));
        msg.type = CHAT_WALL;
        printf(RED"Please Input: \n"NONE);
        scanf("%s[^\n]", msg.msg);
        getchar();
        if (strlen(msg.msg)) {
            if (msg.msg[0] == '@') msg.type = CHAT_MSG;
            if (msg.msg[0] == '#') msg.type = CHAT_FUNC;
            send(sockfd, (void*)&msg, sizeof(msg), 0);
        }
     
    }
    return 0;

}

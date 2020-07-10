
#include "head.h"
extern pthread_mutex_t rmutex;
extern pthread_mutex_t bmutex;
extern int port;
extern int repollfd, bepollfd;
struct User* rteam, * bteam;

void add_event_ptr(int epollfd, int fd, int events, struct User* user) {
    struct epoll_event ev;
    ev.data.ptr = (void*)user;
    ev.events = events;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void del_event(int epollfd, int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
}

int udp_connect(struct sockaddr_in* client) {

    int sockfd;

    if ((sockfd = socket_create_udp(port)) < 0) {

        perror("socket_create_udp");

        return -1;

    }

    if (connect(sockfd, (struct sockaddr*)client, sizeof(struct sockaddr)) < 0) {

        return -1;

    }

    return sockfd;

}

int check_online(struct LogRequest* request) {
    for (int i = 0; i < MAX; i++) {
        if (rteam[i].online && !strcmp(request->name, rteam[i].name))return 1;
        if (bteam[i].online && !strcmp(request->name, bteam[i].name))return 1;
    }
    return 0;           
}



int udp_accept(int fd, struct User* user) {

    int new_fd, ret;

    struct sockaddr_in client;

    struct LogRequest request;

    struct LogResponse response;

    bzero(&request, sizeof(request));

    bzero(&response, sizeof(response));

    socklen_t len = sizeof(client);



    ret = recvfrom(fd, (void*)&request, sizeof(request), 0, (struct sockaddr*)&client, &len);



    if (ret != sizeof(request)) {

        response.type = 1;

        strcpy(response.msg, "Login failed with Data errors!");

        sendto(fd, (void*)&response, sizeof(response), 0, (struct sockaddr*)&client, len);

        return -1;

    }



     if (check_online(&request)) {

         response.type = 1;

         strcpy(response.msg, "You are Already Login!");

         sendto(fd, (void *)&response, sizeof(response), 0, (struct sockaddr *)&client, len);

         return -1;

     }



    response.type = 0;

    sprintf(response.msg, "Login Success. Enjoy yourself!\nUsername = %s",request.name);

    sendto(fd, (void*)&response, sizeof(response), 0, (struct sockaddr*)&client, len);



    if (request.team) {

        DBG(GREEN"注意"NONE" : "BLUE"%s 登录成功 %s:%d  <%s>\n", request.name, inet_ntoa(client.sin_addr), ntohs(client.sin_port), request.msg);

    }
    else {

        DBG(GREEN"注意"NONE" : "RED"%s 登录成功 %s:%d   <%s>\n", request.name, inet_ntoa(client.sin_addr), ntohs(client.sin_port), request.msg);

    }

 //   strcpy(user->ip,inet_ntoa(client.sin.addr));

    strcpy(user->name, request.name);

    user->team = request.team;
    
    new_fd = udp_connect(&client);
    struct ChatMsg r_msg;

    bzero(&r_msg,sizeof(r_msg));

        sprintf(r_msg.msg,"User %s 加入聊天!\n  %s",user->name,request.msg);
        printf("User"RED" %s"NONE"加入聊天! ",user->name);
        strcpy(r_msg.name,user->name);
        r_msg.type = CHAT_SYS;
        send_all(&r_msg);    
 
    user->fd = new_fd;
 /*   response.type = 0;
    sprintf(response.msg,"Dear %s Login Success. Enjoy yourself\n",request.name);
*/
    return new_fd;

}

int find_sub(struct User* team) {
    for (int i = 0; i < MAX; i++) {
        if (!team[i].online)return i;
    }
    return -1;
}

void add_to_sub_reactor(struct User* user) {
    struct User* team = (user->team ? bteam : rteam);
    if(user->team) {
      pthread_mutex_lock(&bmutex);
    } else {
     pthread_mutex_lock(&rmutex);
    }
    //????????
    int sub = find_sub(team);
    if (sub < 0) {
        fprintf(stderr, "Full Team!\n");
        return;
    }
    team[sub] =  *user;
    team[sub].online = 1;
    team[sub].flag = 10;
     if(user->team) {
      pthread_mutex_unlock(&bmutex);
    } else {
     pthread_mutex_unlock(&rmutex);
    }

    DBG(""RED"sub = %d, name =%s\n", sub, team[sub].name);
    if (user->team)
        add_event_ptr(bepollfd, team[sub].fd, EPOLLIN | EPOLLET, &team[sub]);
    else
        add_event_ptr(repollfd, team[sub].fd, EPOLLIN | EPOLLET, &team[sub]);
}


#include "head.h"
extern pthread_mutex_t rmutex;
extern pthread_mutex_t bmutex;
extern int repollfd, bepollfd;
extern struct User*rteam,*bteam;
void send_to(char *to,struct ChatMsg *msg, int fd){

     int flag = 0;
     for(int i=0;i<MAX;i++) {
        if(rteam[i].online &&(!strcmp(to,rteam[i].name))){
             send(rteam[i].fd,msg,sizeof(struct ChatMsg),0);
	     flag =1;
             break;

        }  
	 if(bteam[i].online &&(!strcmp(to,bteam[i].name))){
             send(bteam[i].fd,msg,sizeof(struct ChatMsg),0);
             flag =1;
	     break;
        }
     }
	if(!flag){
	    memset(msg->msg,0,sizeof(struct ChatMsg));
	    sprintf(msg->msg,"用户"RED" %s "NONE"现不在线，或用户名错误！",to);
	    msg->type =CHAT_SYS;
	    send(fd, msg,sizeof(struct ChatMsg),0);
	}

}

void send_all(struct ChatMsg *msg){
        for(int i =0;i<MAX;i++){
        if(bteam[i].online) send(bteam[i].fd,(void*)msg,sizeof(struct ChatMsg),0);
        if(rteam[i].online) send(rteam[i].fd,(void*)msg,sizeof(struct ChatMsg),0);
        }
}


void do_work(struct User* user) {

    struct ChatMsg msg;

    struct ChatMsg r_msg;

    bzero(&msg,sizeof(msg));
    bzero(&r_msg,sizeof(r_msg));
    recv(user->fd, (void*)&msg, sizeof(msg), 0);

    if (msg.type & CHAT_WALL) {
     //   if(!user->test[4]){

        printf("<%s> ∼ %s \n", user->name, msg.msg);
       strcpy(msg.name,user->name);
        send_all(&r_msg);
        //?????????????????//
    }

    else if (msg.type & CHAT_MSG) {
        char to[20] = {0};
        int i = 1;
        for( ;i<=21;i++) {
           if(msg.msg[i] ==' ') break;
        }
        if(msg.msg[i] != ' '|| msg.msg[0] != '@'){
           memset(&r_msg , 0,sizeof(r_msg));
           r_msg.type =CHAT_SYS;
           sprintf(r_msg.msg,RED"私聊的格式错误，服务端无法处理信息"NONE);
           send(user->fd,(void*)&r_msg,sizeof(r_msg),0);
        } else {
           msg.type = CHAT_MSG;
           strcpy(msg.name,user->name);
           strncpy(to,msg.msg+1,i-1);//????????
           send_to(to,&msg,user->fd);
        }
        printf("<%s> $ %s \n", user->name, msg.msg);

    }

    else if (msg. type & CHAT_FIN) {

	bzero(msg.msg,sizeof(msg.msg));
	msg.type = CHAT_SYS;
	sprintf(msg.msg,"用户"RED" %s "NONE"即将下线！\n", user->name);
	strcpy(msg.name,user->name);
	send_all(&msg);
	if(user->team)
		pthread_mutex_lock(&bmutex);
	else 
		pthread_mutex_lock(&rmutex);
        user->online = 0;
//	user->score = 0;
        int epollfd = user->team ? bepollfd : repollfd;
        del_event(epollfd, user->fd);
	if(user->team)
                pthread_mutex_unlock(&bmutex);
        else 
                pthread_mutex_unlock(&rmutex);

        printf(GREEN"系统"NONE" :%s 下线!\n", user->name);

        close(user->fd);

    }



}



void task_queue_init(struct task_queue* taskQueue, int sum, int epollfd) {



    taskQueue->sum = sum;



    taskQueue->epollfd = epollfd;



    taskQueue->team = calloc(sum, sizeof(void*));



    taskQueue->head = taskQueue->tail = 0;



    pthread_mutex_init(&taskQueue->mutex, NULL);



    pthread_cond_init(&taskQueue->cond, NULL);



}







void task_queue_push(struct task_queue* taskQueue, struct User* user) {



    pthread_mutex_lock(&taskQueue->mutex);



    taskQueue->team[taskQueue->tail] = user;



    DBG(L_GREEN"Thread Pool"NONE" : Task push %s\n", user->name);



    if (++taskQueue->tail == taskQueue->sum) {



        DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");



        taskQueue->tail = 0;



    }



    pthread_cond_signal(&taskQueue->cond);



    pthread_mutex_unlock(&taskQueue->mutex);



}











struct User* task_queue_pop(struct task_queue* taskQueue) {



    pthread_mutex_lock(&taskQueue->mutex);



    while (taskQueue->tail == taskQueue->head) {



        DBG(L_GREEN"Thread Pool"NONE" : Task Queue Empty, Waiting For Task\n");



        pthread_cond_wait(&taskQueue->cond, &taskQueue->mutex);



    }



    struct User* user = taskQueue->team[taskQueue->head];



    DBG(L_GREEN"Thread Pool"NONE" : Task Pop %s\n", user->name);



    if (++taskQueue->head == taskQueue->sum) {



        DBG(L_GREEN"Thread Pool"NONE" : Task Queue End\n");



        taskQueue->head = 0;



    }



    pthread_mutex_unlock(&taskQueue->mutex);



    return user;



}







void* thread_run(void* arg) {



    pthread_detach(pthread_self());



    struct task_queue* taskQueue = (struct task_queue*)arg;



    while (1) {



        struct User* user = task_queue_pop(taskQueue);



        do_work(user);



    }



}




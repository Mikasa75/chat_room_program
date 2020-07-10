#include "head.h"
extern int repollfd, bepollfd;

void do_work(struct User* user) {
	struct ChatMsg msg;
	recv(user->fd,(void *)&msg, sizeof(msg), 0);
	if (msg.type & CHAT_WALL) {
		printf("<%s> ∼ %s \n", user->name, msg.msg);
	}
	else if(msg.type & CHAT_MSG){
		printf("<%s> $ %s \n", user->name, msg.msg);
	}
	else if (msg, type & CHAT_FIN) {
		user->online = 0;
		int epollfd = user->team ? bepollfd : repollfd;
		del_event(epollfd, user->fd);
		printf(GREEN"Server Info"NONE" :%s logout!\n", user->name);
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



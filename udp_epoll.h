
#ifndef _UDP_EPOLL_H

#define _UDP_EPOLL_H
int udp_accept(int fd, struct User* user);
void add_to_sub_reactor(struct User* user);
void del_event(int epollfd, int fd);
#endif


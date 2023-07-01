#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>

#define BUFSIZE 100
#define EPOLLSIZE 50
 
void error_handling(char *buf);

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t sz_serv_addr;
    int str_len;
    char buf[BUFSIZE];

    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_count;
    if(argc != 2) {
        printf("Usage : %d <ip> <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("bind() error");
    }
    listen(serv_sock, 5);
    epfd = epoll_create(EPOLLSIZE);
    ep_events = malloc(sizeof(struct epoll_event)*EPOLLSIZE);
    
    event.events = EPOLLIN;
    event.data.fd = serv_sock;

    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    while(1) {
        event_count = epoll_wait(epfd, ep_events, EPOLLSIZE, -1);
        if(event_count == -1) {
            error_handling("epoll() error");
        }
        for(int i = 0; i < event_count; i++) {
            if(ep_events[i].data.fd == serv_sock) {
                sz_serv_addr = sizeof(clnt_addr);
                clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &sz_serv_addr);
                event.events = EPOLLIN;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                printf("connected client : %d\n", clnt_sock);
            } else {
                str_len = read(ep_events[i].data.fd, buf, BUFSIZE);
                if(str_len == 0) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                    close(ep_events[i].data.fd);
                    printf("closed client : %d\n", ep_events[i].data.fd);
                }
                write(ep_events[i].data.fd, buf, str_len);
            }
        }
    }
    close(serv_sock);
    close(epfd);
    return 0;
}

void error_handling(char *buf) {
    fputs(buf, stderr);
    fputs("\n", stderr);
    exit(1);
}

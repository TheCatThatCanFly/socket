#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#define BUFSIZE 50

void error_handling(char *message);
void read_childproc(int sig);


int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    pid_t pid;
    int state, str_len;
    socklen_t sz_clnt_addr;
    char buf[BUFSIZE], msgbuf[BUFSIZE];
    struct sigaction act;
    int fds[2];

    if(argc != 2) {
        printf("few arguments");
        exit(1);
    }

    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags  =0;
    state = sigaction(SIGCHLD, &act, 0);

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1) {
        error_handling("invalid socket");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));


    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("bind error");
    }

    if(listen(serv_sock, 5) == -1) {
        error_handling("listen error");
    }

    sz_clnt_addr = sizeof(clnt_addr);
    pipe(fds);
    pid = fork();
    if(pid == 0) {
        //写文件进程
        FILE *fp = fopen("test.txt", "wt");
        if(fp == NULL) {
            error_handling("invalid write proccess");
        }
        int strlen = 0;
        for(int i = 0; i < 10; i++) {
            strlen = read(fds[0], msgbuf, BUFSIZE);
            fwrite(fp, 1, strlen, fp);
            fflush(fp);
            fsync(fileno(fp));
        }
        fclose(fp);
        return 0;
    }
    while(1) {
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &sz_clnt_addr);
        if(clnt_sock == -1) {
            continue;
        } else {
            puts("new client connect...");
        }
        pid = fork();
        if(pid < 0) {
            close(clnt_sock);
            continue;
        }
        if(pid == 0) {
            close(serv_sock);
            while((str_len = read(clnt_sock, buf, BUFSIZE)) != 0) {
                if(str_len <= 0) {
                    break;
                }
                buf[str_len] = '\0';
                puts(buf);
                write(clnt_sock, buf, str_len);
                write(fds[1], msgbuf, str_len);
            }
            close(clnt_sock);
            return 0;
        } else {
            close(clnt_sock);
        }
    } 
    close(serv_sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputs("\n", stderr);
    exit(1);
}

void read_childproc(int sig) {
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    printf("removed id is : %d", pid);
}
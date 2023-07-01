#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#define BUFSIZE 50

void error_handling(char *message);
void read_proc(int sock, char *buf);
void write_proc(int sock, char *buf);

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in addr;
    pid_t pid;
    char buf[BUFSIZE];

    if(argc != 3) {
        printf("few arguments\n");
        exit(1);
    }
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        error_handling("invalid socket");
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        error_handling("connect error");
    }

    pid = fork();
    if(pid == 0) {
        write_proc(sock, buf);
        return 0;
    } else {
        read_proc(sock, buf);
    }
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputs("\n", stderr);
    exit(1);
}

void read_proc(int sock, char *buf) {
    int str_len;
    while(1) {
        str_len = read(sock, buf, BUFSIZE);
        if(str_len <= 0) {
            break;
        }
        buf[str_len] = '\0';
        puts(buf);
    }
    close(sock);
}

void write_proc(int sock, char *buf) {
    while(1) {
        fgets(buf, BUFSIZE, stdin);
        buf[strlen(buf) - 1] = '\0';
        write(sock, buf, strlen(buf));
    }
    close(sock);
}
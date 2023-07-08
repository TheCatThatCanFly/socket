#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/uio.h>
#define BUFFER_SIZE 1024
#include <libgen.h>

const char *status_line[2] = {"200 OK", "200 Internal server error"};

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t sz_clnt_addr;
    int ret = 0;

    if(argc <= 3) {
        printf("Usage: %s ip_address port filename\n", basename(argv[0]));
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    const char *file_name = argv[3];
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = PF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(serv_sock, 5);
    sz_clnt_addr = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &sz_clnt_addr);

    char head_buf[BUFFER_SIZE];
    memset(head_buf, '\0', BUFFER_SIZE);
    char *file_buf;
    struct stat file_stat;
    int valid = 1;
    int len = 0;

    if(stat(file_name, &file_stat) < 0) {
        valid = 0;
    } else {
        if(S_ISDIR(file_stat.st_mode)) {
            valid = 0;
        } else if(file_stat.st_mode&S_IROTH) {
            int fd = open(file_name, O_RDONLY);
            file_buf = new char[file_stat.st_size + 1];
            memset(file_buf, '\0', BUFFER_SIZE);
            if(read(fd, file_buf, file_stat.st_size) < 0) {
                valid = false;
            }
        } else {
            valid = false;
        }
    } 
    if(valid) {
        ret = snprintf(head_buf, BUFFER_SIZE - 1, "%s%s\r\n", "HTTP/1.1", status_line[0]);
        len += ret;
        ret = snprintf(head_buf + len, BUFFER_SIZE - 1 - len, "Content-Length: %d\r\n", file_stat.st_size);
        len += ret;
        ret = snprintf(head_buf + len, BUFFER_SIZE - 1 - len, "%s", "\r\n");

        struct iovec iv[2];
        iv[0].iov_base = head_buf;
        iv[0].iov_len = strlen(head_buf);
        iv[1].iov_base = file_buf;
        iv[1].iov_len = file_stat.st_size;
        ret = writev(clnt_sock, iv, 2);
    } else {
        ret = snprintf(head_buf, BUFFER_SIZE - 1, "%s%s\r\n", "HTTP/1.1", status_line[1]);;
        len += ret;
        ret = snprintf(head_buf, BUFFER_SIZE - 1 - len, "%s", "\r\n");
        write(clnt_sock, head_buf, strlen(head_buf));
    }
    close(clnt_sock);
    close(serv_sock);
    delete[] file_buf;
    return 0;
}



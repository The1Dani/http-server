#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PORT 8080
#define DEFAULT_TEXT "Hello From some client!\n"
#define MAX 256

typedef struct {
    int sockfd;
    const char *text;
    struct sockaddr_in addr;
} Context;

void add_newline(char *buff, const char *txt) { sprintf(buff, "%s\n", txt); }

void say_hello(Context ctx) {
    char buff[MAX] = {0};
    add_newline(buff, ctx.text);
    write(ctx.sockfd, buff, strlen(buff));
}

void clean_and_exit(Context ctx, int status) {
    close(ctx.sockfd);
    exit(status);
}

int main(int argc, char **argv) {

    Context ctx = {.addr = {0}};

    if ((ctx.sockfd = socket(AF_INET, SOCK_STREAM, 6)) == -1) {
        printf("Could not bind to socket!\n");
        exit(1);
    }

    printf("Socket Created Successfully!\n");

    ctx.addr = (struct sockaddr_in){
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(IP),
        .sin_port = htons(PORT),
    };

    if (connect(ctx.sockfd, (struct sockaddr *)&ctx.addr, sizeof(ctx.addr)) !=
        0) {
        printf("connection has failed!\n");
        clean_and_exit(ctx, 1);
    }

    if (argc <= 1) {
        printf("Sending default text message\n" DEFAULT_TEXT);
        ctx.text = DEFAULT_TEXT;
    } else {
        printf("Sending...\n%s\n", argv[1]);
        ctx.text = argv[1];
    }

    say_hello(ctx);

    clean_and_exit(ctx, 0);
}
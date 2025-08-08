#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "simple_lexer.h"
#include "da.h"
#include "parse_http.h"
#include "req_handle.c"

typedef struct sockaddr sockaddr;

#define IP "127.0.0.1"
#define PORT 8080
#define QUE_LEN 5
#define BUFF_SIZE 8

void clean_and_exit(int sockfd, int status) {
    close(sockfd);
    exit(status);
}

int get_soc_str(char **buff, int connfd) {

    if (*buff != NULL) {
        free(*buff);
    }
    int n_chunk = 1;
    int n;
    char t_buff[BUFF_SIZE];
    
    while((n = read(connfd, t_buff, sizeof(t_buff))) == BUFF_SIZE) {
        *buff = realloc(*buff, sizeof(char) * BUFF_SIZE * n_chunk);
        memcpy(*buff + BUFF_SIZE * (n_chunk - 1), t_buff, n);
        n_chunk++;
    }

    int str_lenght = (n_chunk - 1) * BUFF_SIZE + n;       

    if ( n == 0 ) {
        *buff = realloc(*buff, (str_lenght + 1) * sizeof(char));
    } else if ( n != -1 ){
        *buff = realloc(*buff, (str_lenght + 1) * sizeof(char));
        memcpy(*buff + BUFF_SIZE * (n_chunk - 1), t_buff, n);
    } else {
        exit(1);
    }
    (*buff)[str_lenght] = '\0';
    return str_lenght;

}

void http_send_resp_ok(int connfd) {
    write(connfd, RESP_OK, strlen(RESP_OK));
    close(connfd);
}

void echo_message(int connfd) {
    
    char *buff = NULL;
    get_soc_str(&buff, connfd); 
    Lexer lexer = {0};
    Lexer *lex = &lexer;
    lex->src = buff;
    Da_str da = da_str_new();

    int at = 0;
    for(;;){    
        lex_get_line(lex);
        if (lex->status != LEXER_SUCCSESS) break;    
        char *line = lex->str;
        da_str_push(&da, strdup(line)); //!Memory leak (free strings and array)
        printf(ESC GREEN"Message:"ESC_CLOSE" %s\n", line);
    }
    if (buff == NULL) exit(1);
    // printf(ESC RED"Message_Buf:"ESC_CLOSE" %s", buff);

    Req * req = http_parse_req(da.list, da.size);

    if (req == NULL) {
        exit(3);
    }

    req_handler(req); //Still going

    http_send_resp_ok(connfd);

    lex_destroy(lex);
    free(buff);
}


int main() {

    int sockfd, connfd;
    struct sockaddr_in servaddr = {0};

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        printf("Socket could not be created!\n");
        exit(1);
    }
    printf("Socket Created Successfully!\n");

    /*To reuse address*/
    /**
     * TODO Read about setsockopt  
     */
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    servaddr = (struct sockaddr_in){
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = inet_addr(IP),
    };

    if ((bind(sockfd, (sockaddr*)&servaddr, sizeof(servaddr))) != 0 ) {
        printf("Socket failed to bind probably there is another bind on the same port!\n");
        clean_and_exit(sockfd, 1);        
    }
    printf("Socket bind successfull!\n");

    if ((listen(sockfd, QUE_LEN)) != 0) {
        printf("Server listen failed!\n");
    }

    
    for (;;) {
        
        connfd = accept(sockfd, NULL, NULL);
        if (connfd < 0) {
            printf("Connection to the client failed!\n");
            continue;
        }

        echo_message(connfd);

    }

    return 0;

}
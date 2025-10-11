#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "mime.h"
#include "parse_http.h"
#include "req_handle.h"
#include "simple_lexer.h"
#include <bits/getopt_core.h>

#define ARENA_IMPLEMENTATION
#include "arena.h"

typedef struct sockaddr sockaddr;

#define IP "0.0.0.0"
#define PORT 8080
#define QUE_LEN 25
#define BUFF_SIZE 8

// #define DEBUG_MODE

void clean_and_exit(int sockfd, int status) {
    close(sockfd);
    exit(status);
}

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return;

    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int get_soc_str(char **buff, int connfd) {

    if (*buff != NULL) {
        free(*buff);
    }

    /* Poll the sock connection */
    fd_set reading;
    struct timeval timeout;
    FD_ZERO(&reading);
    FD_SET(connfd, &reading);
    int maxsock = FD_SETSIZE;

    memset(&timeout, 0, sizeof(timeout));
    timeout.tv_sec = 1;
    int rc = select(maxsock, &reading, NULL, NULL, &timeout);

    if (rc < 0) {
        printf("Error in select, %s\n", strerror(errno));
    }
#ifdef DEBUG_MODE
    else if (rc == 0) {
        /* none of the sockets were ready in our little poll */
        printf("nobody is home.\n");
    } else {
        /* at least one of the sockets is ready */
        printf("conndf is %s for reading\n",
               FD_ISSET(connfd, &reading) ? "READY" : "NOT READY");
    }
#endif

    int n_chunk = 1;
    int n;
    char t_buff[BUFF_SIZE];

    while ((n = recv(connfd, t_buff, sizeof(t_buff), MSG_WAITALL)) ==
           BUFF_SIZE) {
        *buff = realloc(*buff, sizeof(char) * BUFF_SIZE * n_chunk);
        memcpy(*buff + BUFF_SIZE * (n_chunk - 1), t_buff, n);
        n_chunk++;
    }

    if (n == -1) {
#ifdef DEBUG_MODE
        printf("Error, %s\n", strerror(errno));
#endif
    }

    int str_lenght = (n_chunk - 1) * BUFF_SIZE + n;

    if (n == 0) {
        *buff = realloc(*buff, (str_lenght + 1) * sizeof(char));
    } else if (n != -1) {
        *buff = realloc(*buff, (str_lenght + 1) * sizeof(char));
        memcpy(*buff + BUFF_SIZE * (n_chunk - 1), t_buff, n);
    } else {
        if (n == -1) {

            if (errno == EAGAIN || errno == EWOULDBLOCK) {
#ifdef DEBUG_MODE
                fprintf(
                    stderr,
                    "No data available right now, skipping...\n Error: %s\n",
                    strerror(errno));
#endif
            } else {
                exit(1);
            }
        } else {
            exit(2);
        }
    }
    (*buff)[str_lenght] = '\0';
    return str_lenght;
}

void write_to_socket(int connfd, const char *buf, size_t size) {
    int written = 0;
    while ((size_t)written < size) {
        int w = write(connfd, buf + written, size - written);
        if (w == -1) {
            // printf("Error writing to socket, %s\n", strerror(errno));
            struct timespec req = {0};
            req.tv_nsec = 1000;
            nanosleep(&req, NULL);
            continue;
        }
        written += w;
    }
    // printf("Written %d bytes to socket, size %zu\n", written, size);
    assert((size_t)written == (size_t)size);
}

void http_send_resp_ok(int connfd, Resp r) {

    Arena *arena = arena_new(KILOBYTE / 8);

    char *buf;
    size_t size = construct_response(&r, (void *)(&buf));

    write_to_socket(connfd, buf, size);

    close(connfd);

#ifdef DEBUG_MODE
    TRANSFORM_BUF_TO_C_STR(buf, size);
    printf("\n%s %s\n", str_to_arena_ptr(arena, paint_str("[DEBUG]", GREEN)),
           buf);

// end:
#endif

    arena_free(arena);
    free(buf);
}

void return_with_bad_request(int connfd) {

    Arena *arena = arena_new(KILOBYTE / 8);
    Resp r = {0};
    r.protocol = "HTTP/1.1";
    SET_FIELDS_NEW(r);
    set_status_code(&r, 400, "Bad Request");

    char *buf;

    int size = construct_response(&r, (void *)(&buf));
    write_to_socket(connfd, buf, size);

    TRANSFORM_BUF_TO_C_STR(buf, size);
    printf("\n%s %s\n", str_to_arena_ptr(arena, paint_str("[DEBUG]", RED)),
           buf);

    close(connfd);
    free(buf);
    exit(10);
}

void echo_message(int connfd) {

    char *buff = NULL;
    get_soc_str(&buff, connfd);
    Lexer lexer = {0};
    Lexer *lex = &lexer;
    lex->src = buff;

    Arena *string_arena = arena_new(0);
    Da_str da = da_str_new(string_arena);

    char *msg_str = paint_str("Message:", GREEN);

    for (;;) {
        lex_get_line(lex);
        if (lex->status != LEXER_SUCCSESS)
            break;
        char *line = lex->str;
        assert(line != NULL);
        da_str_push(&da, a_strdup(da.arena, line));
#ifdef DEBUG_MODE
        printf("Proc %d | %s %s\n", getpid(), msg_str, line);
#endif
    }

    free(msg_str);
    if (buff == NULL)
        exit(1);

    /*Check for bad request*/
    /*TLS start code sequance*/
    // 16 03 01 02 7e 01
    // 16 03 01 07 4f 01
    if (da.size > 0) {
        if (strcmp(da.list[0], "\x16\x03\x01\x02\x7e\x01") == 0 ||
            strcmp(da.list[0], "\x16\x03\x01\x07\x4f\x01") == 0) {
            return_with_bad_request(connfd);
        }
    }

    Req *req = http_parse_req(da.list, da.size);

    if (req == NULL) {
        goto free_label;
    }

    Resp resp = {0};

    SET_FIELDS_NEW(resp);

    resp.protocol = a_strdup(resp.fields.keys->arena, SUPPORTED_PROTOCOL);

    req_handler(req, &resp);

    http_send_resp_ok(connfd, resp);

    req_free(req);
free_label:
    a_map_free(resp.fields);
    if (resp.body.body != NULL)
        free(resp.body.body);
    arena_free(string_arena);
    lex_destroy(lex);
    free(buff);
}

void reap_handler(int sig) {

    (void)sig;

    pid_t st_code;
    pid_t proc;
#ifdef DEBUG_MODE
    char buf[256];
#endif
    while ((proc = waitpid(-1, &st_code, WNOHANG)) > 0) {
#ifdef DEBUG_MODE
        sprintf(buf, "proc %d, status code: %u", proc, st_code);
        char *painted_str = paint_str(buf, GREEN);
        puts(painted_str);
        free(painted_str);
#endif
    };
}

void sigint_handler(int sig) {
    (void)sig;

    exit(0); // Will flush stdio buffers, free memory
}

#define MAKE_FREE_FUNC(ptr)                                                    \
    void free_func_##ptr(void) {                                               \
        free(ptr);                                                             \
        ptr = NULL;                                                            \
    }

MAKE_FREE_FUNC(ROOT_FOLDER)

void deinit_mime() { deinit(mime_info); }

int main(int argc, char **argv) {

    {
        int opt;
#define HELP_MESSAGE                                                           \
    "Help Message\n"                                                           \
    "-r [dir] - Specify a root directory to serve\n"
        /*
         ! Determine Help message!
        */
        while ((opt = getopt(argc, argv, "hr:")) != -1) {
            switch (opt) {
            case 'h':
                puts(HELP_MESSAGE);
                break;
            case 'r':
                if (!is_dir(optarg)) {
                    fprintf(stderr, "The string specified with -r flag is not "
                                    "a valid directory\n");
                    exit(1);
                }
                ROOT_FOLDER = strdup(optarg);
                if (atexit(free_func_ROOT_FOLDER)) {
                    puts("At exit failed\n");
                    exit(1);
                }
                break;
            default:
                fprintf(stderr, HELP_MESSAGE);
                exit(1);
                break;
            }
        }
        printf("%s is the current root directory\n", ROOT_FOLDER);
    }

    { /*Setting a Signal for reaping handlers*/
        struct sigaction sa;

        sa.sa_handler = reap_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGCHLD, &sa, NULL);

        signal(SIGINT, sigint_handler); // Free stdio buffers
    }

    int sockfd, connfd;
    struct sockaddr_in servaddr = {0};

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Socket could not be created!\n");
        exit(1);
    }

    /*Load Mime to Mem*/
    mime_info = load_mimes();
    atexit(deinit_mime);

    printf("Socket Created Successfully!\n");

    printf("Serving %s on Port %d\n", IP, PORT);

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    servaddr = (struct sockaddr_in){
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = {inet_addr(IP)},
    };

    if ((bind(sockfd, (sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
        printf("Socket failed to bind probably there is another bind on the "
               "same port!\n");
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

        pid_t childID;
        if ((childID = fork()) == -1) {
            perror("fork err\n");
            exit(1);
        }

        if (childID == 0) {
#ifdef DEBUG_MODE
            printf("A message from a child proc!\n");
#endif
            close(sockfd);
            set_nonblocking(connfd);
            echo_message(connfd);
            exit(EXIT_SUCCESS);
        } else {
            close(connfd);
#ifdef DEBUG_MODE
            printf("client !!!!! from ID %d\n", childID);
#endif
        }
    }

    return 0;
}

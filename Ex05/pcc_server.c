#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#define SUCCESS 0
#define NUM_PRINTABLES 95

typedef struct counts {
    int total;
    int printable_counts[NUM_PRINTABLES];
} counts;

typedef struct sockaddr_in sockaddr_in;

int is_printable(char c);
int init_sigint_handler();
void my_sigint_handler();
void shutdown_srv();

int connfd = -1;
int sigint_flag = 0;
counts count = {0};

/*
Arguments:
    argv[1]: server’s IP address

FLOW:
    1. Initialize a data structure pcc_total that will count how many times each 
       printable character was observed in all client connections.
    2. Listen to incoming TCP connections on the specified server port.
    3. Enter a loop, in which each iteration:
        a) Accepts a TCP connection.
        b)When a connection is accepted, reads a stream of bytes from the client,
          computes its printable character count and writes the result to the client over the TCP connection.
          After sending the result to the client, updates the pcc_total global data structure.
    4. Handle SIGINT as described.
*/
int main(int argc, char** argv)
{
    int listenfd  = -1;
    int total_sent = -1;
    int total_read = -1;
    sockaddr_in serv_addr;
    int nread, nsent;
    int port;
    int N, i;
    int status;
    char *buff_N, *buff;
    uint32_t serv_N, serv_count;
    char *serv_count_p;
    int not_read, not_written;

    if (argc != 2){
        perror("Error! Invalid amount of argumnets. should be 1.");
        exit(1);
    }
    port = atoi(argv[1]);


    status = init_sigint_handler();
    if (status < SUCCESS) {
        perror("Error! Could not initialize sigint handler successfully.");
        exit(1);
    }

    status = listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (status < SUCCESS) {
        perror("Error! Could not create socket successfully.");
        exit(1);
    }

    status = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    if (status < SUCCESS) {
        perror("Error! Could not opt socket successfully.");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    status = bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (status < SUCCESS) {
        perror("Error! Could not bind socket successfully.");
        exit(1);
    }

    status = listen(listenfd, 10);
    if (status < SUCCESS) {
        perror("Error! Could not listen successfully.");
        exit(1);
    }

    while (1) {
        if (sigint_flag == 1) {
            shutdown_srv();
        }

        status = connfd = accept(listenfd, NULL, NULL);
        if(status < SUCCESS) {
            perror("Error! Could not accept connection successfully.");
            exit(1);
        }

        buff_N = (char *)&serv_N;
        total_read = 0;
        not_read = 4;
        while (not_read > 0) {
            nread = read(connfd, buff_N + total_read, not_read);
            total_read += nread;
            not_read -= nread;
        }
        //TODO: take care ERRORS! 

        N = ntohl(serv_N);
        buff = (char *) malloc(N);
        if (buff == NULL) {
            perror("Error! Could not malloc successfully.");
            exit(1);
        }

        total_read = 0;
        not_read = N;
        while (not_read > 0) {
            nread = read(connfd, buff + total_read, not_read);
            total_read += nread;
            not_read -= nread;
        }
        //TODO: take care ERRORS!

        // Process data received
        for (i = 0; i < N; i++) {
            if (is_printable(buff[i])) {
                count.total += 1;
            }
        }

        serv_count = htonl(count.total);
        serv_count_p = (char*)&serv_count;
        total_sent = 0;
        not_written = 4;
        while (not_written > 0) {
            nsent = write(connfd, serv_count_p + total_sent, not_written);
            if (nsent < 0) {
                //TODO: ERROR
            }
            total_sent += nsent;
            not_written -= nsent;
        }

        for (i = 0; i < N; i++) {
            if (is_printable(buff[i])) {
                count.printable_counts[((int)buff[i] - 32)] += 1;
            }
        }

        close(connfd);
        connfd = -1;
    }
}

/*
    Return 1 iff c is printable
*/
int is_printable(char c)
{
    return (32 <= (int)c) | ((int)c <= 126);
}

/*
    Initialize the SIGINT
*/
int init_sigint_handler()
{
    struct sigaction signal = {{0}};
    signal.sa_handler =&my_sigint_handler;
    signal.sa_flags = SA_RESTART;
    return sigaction(SIGINT, &signal, 0);
}

/*
    SIGINT func
*/
void my_sigint_handler()
{
    if (connfd >= 0) {
        sigint_flag = 1;
    } else {
        shutdown_srv();
    }
}

/*
    Shutdown server. Print char's info as req.
*/
void shutdown_srv() {
    int i;
    for (i = 0; i < 95; i++) {
        printf("char '%c' : %u times\n", 
                i+32, count.printable_counts[i]);
    }
}
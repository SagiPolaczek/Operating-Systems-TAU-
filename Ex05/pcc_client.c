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

int get_file_size(FILE* fd);

typedef struct sockaddr_in sockaddr_in;

/*
Arguments:
    argv[1]: server’s IP address
    argv[2]: server’s port
    argv[3]: path of the file to send

FLOW:
    1. Open the specified file for reading & obtain it's size N.
    2. Create a TCP connection to the specified server port on the specified server IP.
    3. Transfer the contents of the file to the server over the TCP
       connection and receive the count of printable characters computed by the server.
    4. Print the number of printable characters obtained to stdout.
    5. Exit with exit code 0.
*/
int main(int argc, char** argv)
{
    FILE *file;
    int N, i;
    uint32_t N_read;
    uint32_t C;
    uint32_t serv_N;
    char *buff_N;
    int nsent, nread;
    int total_sent = -1;
    int total_read = -1;
    int not_written, not_read;
    int sockfd = -1;
    int port, status;
    char* buff;
    sockaddr_in serv_addr;
    int scanned;
    char *path;
    int i_debbug;

    if (argc != 4) {
        perror("Error! Invalid amount of argumnets. should be 3.");
        exit(1);
    }

    // --- Stage 1 ------
    port = atoi(argv[2]);
    path = argv[3];
    file = fopen(path, "rb");
    if (file == NULL) {
        perror("Error! Could not load file successfully.");
        exit(1);
    }

    N = get_file_size(file);


    // --- Stage 2 ------
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error! Could not create socket successfully.");
        exit(1);
    }

    // Conf server
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    status = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    if (status < SUCCESS) {
        perror("Error! Could not connect successfully.");
        exit(1);
    }

    // --- Stage 3 ------
    // Set buff & read into it
    buff = (char *) malloc(N);
    if (buff == NULL) {
        perror("Error! Could not allocate memmory successfully.");
        exit(1);
    }
    for (i = 0; i < N; i++) {
        scanned = fscanf(file, "%c", &buff[i]);
        if (scanned <= 0) {
            perror("Error! Could not fscanf successfully.");
            exit(1);
        }
    }
    fclose(file);

    serv_N = htonl(N);
    buff_N = (char*)&serv_N;
    not_written = 4;
    total_sent = 0;
    i_debbug = 0;
    while (not_written > 0) {
        i_debbug++;
        printf("i_debbug = %d\n", i_debbug);
        nsent = write(sockfd, buff_N + total_sent, not_written);
        if (nsent < 0) {
            perror("Error! Could not write to socket (1) successfully.");
            exit(1);
        }
        total_sent += nsent;
        not_written -= nsent;
    }

    total_sent = 0;
    not_written = N;
    while (not_written > 0) {
        nsent = write(sockfd, buff + total_sent, not_written);
        if (nsent < 0) {
            perror("Error! Could not write to socket (2) successfully.");
            exit(1);
        }
        total_sent += nsent;
        not_written -= nsent;
    }
    free(buff);
    
    total_read = 0;
    not_read = 4;
    while (not_read > 0) {
        nread = read(sockfd, buff_N + total_read, not_read);
        if (nread < 0) {
            perror("Error! Could not read from socket successfully.");
            exit(1);
        }
        total_read += nread;
        not_read -= nread;
    }
    close(sockfd);

    // --- Stage 4 ------
    C = ntohl(serv_N);
    printf("# of printable characters: %u\n", C);

    // --- Stage 5 ------
    exit(0);
}

// ####### ADDITIONAL FUNCTIONS ########
/*
    Return the file's size in Bytes
*/
int get_file_size(FILE* fd)
{
    int sz;
    int status;

    status = fseek(fd, 0L, SEEK_END);
    if (status != SUCCESS) {
        perror("Error! Could not get file size (1) successfully.");
        exit(1);
    }
    sz = ftell(fd);
    fseek(fd, 0L, SEEK_SET);
    if (status != SUCCESS) {
        perror("Error! Could not get file size (2) successfully.");
        exit(1);
    }

    return sz;
}

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include <signal.h>

int connfd = -1;
int sigint_flag = 0;
uint32_t pcc_total[95] = {0};

void close_server(){
    for (int i = 0; i < 95; i++) {
        printf("char '%c' : %u times\n", (i+32), pcc_total[i]);
    }
    exit(0);
}

void sigint_handler(){
    if(connfd < 0){ // not connected (so not processing a client)
        close_server();
    }else{ 
        sigint_flag = 1; // will be caught in server's while loop after client processing finished
    }
}

int main(int argc, char *argv[])
{
    int rt = 1;
    int listenfd = -1;
    int bytes;
    int total_bytes;
	char *int_buff;
	char *rcv_buff;
    uint32_t N, net_int, C;
    uint32_t pcc_buff[95] = {0};
    struct sockaddr_in serv_addr;

    // validate number of arguments
    if (argc != 2){
        fprintf(stderr, "invalid number of arguments, Error: %s\n", strerror(EINVAL));
        exit(1);
    }

    // initiate SIGINT handler
    struct sigaction sigint;
	sigint.sa_handler = &sigint_handler;
	sigemptyset(&sigint.sa_mask);
	sigint.sa_flags = SA_RESTART;
	if (sigaction(SIGINT, &sigint, 0) != 0) {
		fprintf(stderr, "Signal handle registration failed. Error: %s\n", strerror(errno));
		exit(1);
	}

    // create socket and set address to be reuseable
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "Could not create socket, Error: %s\n", strerror(errno));
        exit(1);
	}
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &rt, sizeof(int)) < 0){
        fprintf(stderr, "setsockopt Error: %s\n", strerror(errno));
        exit(1);
    }

    // set server info
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY = any local machine address
    serv_addr.sin_port = htons(atoi(argv[1])); // set port from the argument given

    if (0 != bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))){
        fprintf(stderr, "Bind Failed, Error: %s \n", strerror(errno));
        exit(1);
    }

    if (0 != listen(listenfd, 10)){
        fprintf(stderr, "Error : Listen Failed. %s \n", strerror(errno));
        exit(1);
    }

    // enter loop to accept TCP connections
    while (1){
        // in case of SIGINT mid-processing or an unlikely slip of SIGINT before 'connfd = -1;'
        if(sigint_flag){
            close_server();
        }
        // Accept a connection.
        connfd = accept(listenfd, NULL, NULL);
        if (connfd < 0){
            fprintf(stderr, "Accept Failed, Error: %s\n", strerror(errno));
            exit(1);
        }
        // read N from client (4 bytes)
        int_buff = (char*)&net_int;
        total_bytes = 0;
        bytes = 1;
        while (bytes > 0){
		    bytes = read(connfd, int_buff+total_bytes, 4-total_bytes);
            total_bytes += bytes;
	    }
	    if(bytes < 0 ){
            if (!(errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE)){
                fprintf(stderr, "N read failed, Error: %s\n", strerror(errno));
                exit(1);
            }else{
                fprintf(stderr, "TCP Error while reading N: %s\n", strerror(errno));
                close(connfd);
                connfd = -1;
                continue;
            }
	    }else{ // (bytes == 0)
            if(total_bytes != 4){ // indicating client process killed unexpectedly
                fprintf(stderr, "unexpected conncection close while reading N, "
                                "server will keep accepting new client connections. Error: %s\n", strerror(errno));
                close(connfd);
                connfd = -1;
                continue;
            }
        }
        // convert N to host form and allocate memory for data
        N = ntohl(net_int);
        rcv_buff = malloc(N);
        // read file data from client (N bytes)
        total_bytes = 0;
        bytes = 1;
        while (bytes > 0){
		    bytes = read(connfd, rcv_buff+total_bytes, N-total_bytes);
            total_bytes += bytes;
	    }
	    if(bytes < 0 ){
            if (!(errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE)){
                fprintf(stderr, "data read failed, Error: %s\n", strerror(errno));
                exit(1);
            }else{
                fprintf(stderr, "TCP Error while reading data: %s\n", strerror(errno));
                free(rcv_buff);
                close(connfd);
                connfd = -1;
                continue;
            }
	    }else{ // (bytes == 0)
            if(total_bytes != N){ // indicating client process killed unexpectedly
                fprintf(stderr, "unexpected conncection close while reading data, "
                                "server will keep accepting new client connections. Error: %s\n", strerror(errno));
                free(rcv_buff);
                close(connfd);
                connfd = -1;
                continue;
            }
        }
        // calculate C and count readable chars in pcc_buff
        C = 0;
        for(int i = 0; i < 95; i++){
            pcc_buff[i] = 0;
        }
        for(int i = 0; i < N; i++){
            if(32 <= rcv_buff[i] && rcv_buff[i] <= 126){
                C++;
                pcc_buff[(int)(rcv_buff[i]-32)]++;
            }
        }
        free(rcv_buff);
        // send C to client (4 bytes)
        net_int = htonl(C);
        total_bytes = 0;
        bytes = 1;
        while(bytes > 0){ // use totalbytes?
		    bytes = write(connfd, int_buff+total_bytes, 4-total_bytes);
		    total_bytes += bytes;
	    }
	    if(bytes < 0){ 
            if (!(errno == ETIMEDOUT || errno == ECONNRESET || errno == EPIPE)){
                fprintf(stderr, "C write failed, Error: %s\n", strerror(errno));
                exit(1);
            }else{
                fprintf(stderr, "TCP Error while writing C: %s\n", strerror(errno));
                close(connfd);
                connfd = -1;
                continue;
            }
	    }else{ // (bytes == 0)
            if(total_bytes != 4){ // indicating client process killed unexpectedly
                fprintf(stderr, "unexpected conncection close while writing C, "
                                "server will keep accepting new client connections. Error: %s\n", strerror(errno));
                close(connfd);
                connfd = -1;
                continue;
            }
        }
        //update pcc total
        for(int i = 0; i < 95; i++){
            pcc_total[i] += pcc_buff[i];
        }
        close(connfd);
        connfd = -1;
    }
}
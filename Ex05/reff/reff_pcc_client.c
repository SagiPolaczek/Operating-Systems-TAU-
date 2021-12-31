#define _DEFAULT_SOURCE
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	FILE *in;
	char *int_buff;
	char *send_buff;
	int bytes;
	int total_bytes;
	int sockfd = -1;
	uint32_t N, net_int, C;
	struct sockaddr_in serv_addr; // where we Want to get to

	// validate number of arguments
	if (argc != 4) {
        fprintf(stderr, "invalid number of arguments, Error: %s\n", strerror(EINVAL));
        exit(1);
    }

	in = fopen(argv[3],"rb");
	if (in == NULL) {
        fprintf(stderr, "Can't open input file: %s\n Error: %s\n", argv[3], strerror(errno));
        exit(1);
    }
	// save file size in bytes (N)
	fseek(in, 0, SEEK_END);
	N = ftell(in);
	// save file data as (dynamically allocated) array of chars
	fseek(in, 0, SEEK_SET);
	send_buff = malloc(N); // allocating N bytes for the buffer
	if (send_buff == NULL){
		fprintf(stderr, "Error allocating N=%u bytes for the buffer: %s\n", N, strerror(errno));
        exit(1);
	}
	if (fread(send_buff, 1, N, in) != N){
		fprintf(stderr, "Error reading from file: %s\n", strerror(errno));
        exit(1);
	}
	fclose (in);

	// create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "Could not create socket, Error: %s\n", strerror(errno));
        exit(1);
	}

	// set server info
	memset(&serv_addr, 0, sizeof(serv_addr));
	inet_aton(argv[1], &serv_addr.sin_addr); // sets IP from args
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2])); // sets port from args, Note: htons for endiannes

	// connect socket to the target address
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		fprintf(stderr, "Connect Failed, Error: %s\n", strerror(errno));
        exit(1);
	}

	// write file size to server (4 bytes)
	net_int = (htonl(N));
	int_buff = (char*)&net_int;
	total_bytes = 0;
	while(total_bytes < 4){
		bytes = write(sockfd, int_buff+total_bytes, 4-total_bytes);
		if(bytes < 0){ // == 0? break? assuming server doesn't die unexpectedly?
			fprintf(stderr, "Writing N to server failed, Error: %s\n", strerror(errno));
        	exit(1);
		}
		total_bytes += bytes;
	}
	// write file data to server (N bytes)
	total_bytes = 0;
	while(total_bytes < N){
		bytes = write(sockfd, send_buff+total_bytes, N-total_bytes);
		if(bytes < 0){
			fprintf(stderr, "Writing data to server failed, Error: %s\n", strerror(errno));
        	exit(1);
		}
		total_bytes += bytes;
	}
	free(send_buff);
	// read C from server (4 bytes)
	total_bytes = 0;
	while (total_bytes < 4){
		bytes = read(sockfd, int_buff+total_bytes, 4-total_bytes);
		if(bytes < 0){
			fprintf(stderr, "Reading C from server failed, Error: %s\n", strerror(errno));
        	exit(1);
		}
		total_bytes += bytes;
	}
	close(sockfd);

	// convert C to host form and print
	C = ntohl(net_int);
	printf("# of printable characters: %u\n", C);
	exit(0);
}
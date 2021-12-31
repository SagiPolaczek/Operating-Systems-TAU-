all:
	gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 pcc_server.c
	gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 pcc_client.c

server:
	gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 pcc_server.c -o server
	
client:
	gcc -O3 -D_POSIX_C_SOURCE=200809 -Wall -std=c11 pcc_client.c -o client

clean:
	rm a.out
#include "message_slot.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/fcntl.h> 
#include <sys/ioctl.h>      
#include <unistd.h>     


int main(int argc, char **argv)
{
    int status;
    int total_read;

    if (argc != 3) {
        perror("reader: invalid number of arguments\n");
        exit(1);
    }

    // Parsing args
    char* msg_slot_file_path = argv[1];
    unsigned int target_msg_channel_id = atoi(argv[2]);

    int fd = open(msg_slot_file_path, O_RDONLY);
    if (fd < 0) {
        perror("reader: Error has occured while opening the file\n");
        exit(1);
    }

    status = ioctl(fd, MSG_SLOT_CHANNEL, target_msg_channel_id);
    if (status != SUCCESS) {
        perror("reader: Error has occured while executing ioctl\n");
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    total_read = read(fd, buffer, BUFFER_SIZE);
    printf("reader: read from msg_slot = %d\n", total_read);
    if (status < SUCCESS) {
        perror("reader: Error has occured while reading the msg\n");
        exit(1);
    }

    status = write(1, buffer, total_read);
    printf("reader: total write to console = %d\n", status);
    if (status == FAILURE) {
        perror("reader: Error has occured while printing the msg\n");
        exit(1);
    }

    close(fd);
    return SUCCESS;

}
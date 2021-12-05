
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

    if (argc != 4) {
        // TODO: raise error.
    }

    // parsing arguments
    char* msg_slot_file_path = argv[1];
    unsigned int target_msg_channel_id = atoi(argv[2]);
    char* msg = argv[3];
    int length = strlen(msg);

    // Open file
    int fd = open(msg_slot_file_path, O_WRONLY);
    if (fd < 0) {
        // TODO: perror
        exit(1);
    }
    
    // Set channel id
    status = ioctl(fd, MSG_SLOT_CHANNEL, target_msg_channel_id);
    if (status != SUCCESS) {
        // TODO: perror
        exit(1);
    }

    // Write
    status = write(fd, msg, length);
    if (status != length) {
        // TODO: perror
        exit(1);
    }

    close(fd);
    return SUCCESS;
}
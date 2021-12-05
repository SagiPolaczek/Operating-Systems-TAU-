#include "message_slot.h"


int main(int argc, char **argv)
{
    int status;
    int total_written;

    if (argc != 3) {
        //TODO: raise error
        exit(1);
    }

    // Parsing args
    char* msg_slot_file_path = argv[1];
    unsigned int target_msg_channel_id;
    sscanf(argv[2], "%p", &target_msg_channel_id);

    fd = open(msg_slot_file_path, O_RDONLY);
    if (fd < 0) {
        // TODO: perror msg
        exit(1);
    }

    status = ioctl(fd, MSG_SLOT_CHANNEL, target_msg_channel_id);
    if (status != SUCCESS) {
        // TODO: perror msg
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    total_written = read(fd, buffer, BUFFER_SIZE);
    if (status < SUCCESS) {
        // TODO: perror msg
        exit(1);
    }

    status = write(STDOUT_FILENO, buffer, total_written);
    if (status == FAILURE) {
        // TODO: perror
        exit(1);
    }

    close(fd);
    return SUCCESS;

}
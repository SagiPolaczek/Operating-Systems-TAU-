#pragma once

#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <errno.h>

MODULE_LICENSE("GPL");

#define SUCCESS 0
#define FAILURE -1
#define DEVICE_RANGE_NAME "message_slot"
#define BUFFER_SIZE 128
#define MINOR_AMOUNT_LIMIT 257
#define DEVICE_FILE_NAME "message_slot"
#define MAJOR_NUM 240

typedef struct channel_node {
    int minor;
    int channel_id; // key
    char msg_buffer[BUFFER_SIZE]; // ????
    int msg_size = -1;
    channel_node* next = NULL;
} channel_node;

typedef struct file_p_data {
    unsigned int channel_id;
    int minor;
} file_p_data;
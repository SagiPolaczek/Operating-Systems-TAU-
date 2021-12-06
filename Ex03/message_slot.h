#pragma once

#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#define SUCCESS 0
#define FAILURE -1
#define DEVICE_RANGE_NAME "message_slot"
#define BUFFER_SIZE 128
#define MINOR_AMOUNT_LIMIT 257
#define DEVICE_FILE_NAME "message_slot"
#define MAJOR_NUM 240
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned int)

typedef struct channel_node {
    int minor;
    int channel_id; // key
    char msg_buffer[BUFFER_SIZE]; // ????
    int msg_size;
    struct channel_node* next;
} channel_node;

typedef struct file_p_data {
    unsigned int channel_id;
    int minor;
} file_p_data;


channel_node* find_channel_node(channel_node** ch_slots, int minor, int channel_id);
channel_node* insert_channel_node(channel_node** ch_slots, int minor, int channel_id);
int free_sll(channel_node* head_node);
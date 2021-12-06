#include "message_slot.h"

#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/
#include <linux/slab.h>
#include <linux/ioctl.h>

MODULE_LICENSE("GPL");

// Keep an array of 256 entries.
// The i'th entry contains a POINTER for a channel's SLL of the minor 'i'.
channel_node* ch_slots[MINOR_AMOUNT_LIMIT];

//================== DEVICE FUNCTIONS ===========================
static int device_open(struct inode* inode, struct file*  file)
{
    file_p_data* file_data;

    printk("Initiating 'device_open'.");

    file -> private_data = (void*) kmalloc(sizeof(file_p_data), GFP_KERNEL);
    file_data = (file_p_data*)file -> private_data;
    if (file_data == NULL) {
        // TODO: raise error
        return FAILURE;
    }

    file_data -> minor = iminor(inode);
    file_data -> channel_id = 0;
    return SUCCESS;
}

//---------------------------------------------------------------
static int device_release(struct inode* inode, struct file*  file)
{
    printk("Initiating 'device_open'.");
    kfree(file -> private_data);
    return SUCCESS;
}

//---------------------------------------------------------------
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset )
{
    int status, minor, msg_size, i;
    unsigned int channel_id;
    channel_node* node;
    char* msg_buffer;

    printk("Initiating 'device_read'.");

    // get channel id
    channel_id = ((file_p_data*) file -> private_data) -> channel_id;

    // Check msg ch id validation   
    if (channel_id == 0) {
        return -EINVAL;
    }

    // get minor
    minor = ((file_p_data*) file -> private_data) -> minor;
    
    // find channel node, if exist
    node = find_channel_node(ch_slots, minor, channel_id);
    if (node == NULL)  {
        return -ENOSPC;
    }

    msg_size = node -> msg_size;
    if (length < msg_size) {
        return -ENOSPC;
    }

    // Read msg
    msg_buffer = node -> msg_buffer;
    for (i = 0; i < length; i++) {
        status = put_user(msg_buffer[i], &buffer[i]); // maybe need fix
        if (status != SUCCESS) {
            // TODO: raise error
        }
    }


    return length;
}

//---------------------------------------------------------------
// a processs which has already opened
// the device file attempts to write to it
static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset)
{
    int status, minor, i;
    unsigned int channel_id;
    char* msg_buffer;
    channel_node* node;

    printk("Initiating 'device_write'.");
    // Check msg length validation
    if (length <= 0 || length > 128) {
        return -EMSGSIZE;
    }

    channel_id = ((file_p_data*) file -> private_data) -> channel_id;
    // Check msg ch id validation   
    if (channel_id == 0) {
        return -EINVAL;
    }

    // Get minor from file's private data
    minor = ((file_p_data*) file -> private_data) -> minor;
    
    node = find_channel_node(ch_slots, minor, channel_id);
    if (node == NULL) {
        node = insert_channel_node(ch_slots, minor, channel_id);
    }

    // Write msg
    msg_buffer = node -> msg_buffer;
    for (i = 0; i < length; i++) {
        status = put_user(buffer[i], &msg_buffer[i]); // maybe need fix
        if (status != SUCCESS) {
            // TODO: raise error
        }
    }

    node -> msg_size = length;
    return length;
}

//----------------------------------------------------------------
static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param )
{
    printk("Initiating 'device_ioctl'.");

    // Switch according to the ioctl called
    if( ioctl_command_id == MSG_SLOT_CHANNEL && ioctl_param != 0) {
        // Get the parameter given to ioctl by the process
        // Set current file's channel using the private data.
        file_p_data *p_data = file -> private_data;
        p_data -> channel_id = ioctl_param;
        return SUCCESS;
    }

    return -EINVAL;
}

//==================== DEVICE SETUP =============================

// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations fops =
{
    .owner	        = THIS_MODULE, 
    .read           = device_read,
    .write          = device_write,
    .open           = device_open,
    .unlocked_ioctl = device_ioctl,
    .release        = device_release,
};

//---------------------------------------------------------------
// Initialize the module - Register the character device
static int __init device_init(void)
{
    int status;
    printk("Initiating 'device_init'.");

    // Register driver capabilities. Obtain major num
    status = register_chrdev( MAJOR_NUM, DEVICE_RANGE_NAME, &fops );

    // Negative values signify an error
    if ( status <=FAILURE ) {
    printk(KERN_ERR "%s registraion failed for  %d\n",
                        DEVICE_FILE_NAME, MAJOR_NUM );
    return FAILURE;
    }

    return SUCCESS;
}

//---------------------------------------------------------------
static void __exit device_cleanup(void)
{
    printk("Initiating 'device_cleanup'.");

    for (int i = 0; i < MINOR_AMOUNT_LIMIT; i++) {
        free_sll(ch_slots[i]);
    }
    // Unregister the device
    // Should always succeed
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

//---------------------------------------------------------------
module_init(device_init);
module_exit(device_cleanup);



// ========================= ADDITIONAL FUNCTIONS =========================


// Use minor and channel_id as keys to find the relevant msg slot
// return NULL on failure
channel_node* find_channel_node(channel_node** ch_slots, int minor, int channel_id)
{
    channel_node* curr_node = ch_slots[minor];
    while (curr_node != NULL) {
        if ((curr_node -> channel_id) == channel_id) {
            return curr_node;
        }
        curr_node = curr_node -> next;
    }
    return NULL;
}

// Create and insert a channel_node for the specific minor & channel_id
channel_node* insert_channel_node(channel_node** ch_slots, int minor, int channel_id)
{
    channel_node* node  = (channel_node*)kmalloc(sizeof(channel_node), GFP_KERNEL);
    if (node == NULL) {
        // TODO: raise error
    }
    node -> minor = minor;
    node -> channel_id = channel_id;
    node -> next = ch_slots[minor];
    ch_slots[minor] = node;

    return node;
}

// TODO: implement free function
int free_sll(channel_node* head_node) {
    channel_node* curr_node = head_node;
    channel_node next_node;
    if (head_node == NULL) {
        return SUCCESS;
    }
    while (curr_node -> next != NULL){
        next_node = curr_node -> next;
        kfree(curr_node);
        curr_node = next_node;
    }

    return SUCCESS;
}

//========================= END OF FILE =========================

#include <linux/limits.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdio.h>

#define SUCCESS 0
#define FAILURE 1


// Function's declarations 

// Structs:
typedef struct dir_node {
    char path[PATH_MAX];
    struct dir_node *next;
} dir_node;

typedef struct dir_queue {
    dir_node *head;
    dir_node *tail;
    int size;
} dir_queue;

typedef struct thread_node {
    struct thread_node *next;
} thread_node;

typedef struct thread_queue {
    thread_node *head;
    thread_node *tail;
    int size;
    int busy;
} thread_queue;


/*
argv[1] = search root directory
argv[2] = search term
argv[3] = # of searching threads 

FLOW:
1. Create a FIFO queue that holds directories.
2. Put the search root directory in the queue.
3. Create n (argv[3]) searching threads
4. After all searching threads are created, the main thread signals them to start searching.
5. The program exits in one of the following cases:
    a) there are no more directories in the queue and all live searching threads are idle.
    b) all searching threads have died due to an error.

*/
int main(int argc, char* argv)
{
    // Declerations
    int num_threads;
    dir_queue queue;

    // Validating input:
    if (argc != 4) {
        // TODO: Raise an error - wrong number of arguments
    }
    // TODO: Check the validation of search's path

    num_threads = atoi(argv[3]);
    pthread_t *threads_arr = (pthread_t*)calloc(num_threads, sizeof(pthread_t));
    // TODO: Check valid

    
    
    // Create a FIFO queue



    // 3. Create threads





}



// Additional functions


// ----- Directory's Queue & Node Functionallity -----
int is_empty(dir_queue *queue)
{
    return (queue -> size == 0);
}

dir_queue create_dir_queue()
{
    // TODO: implement
}

dir_node create_dir_node()
{
    // TODO: implement
}

void free_dir_queue(dir_queue *queue)
{

}

// ----- Threads' Queue & Node Functionallity -----


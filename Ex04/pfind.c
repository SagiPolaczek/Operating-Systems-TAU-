#include <linux/limits.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define SUCCESS 0
#define FAILURE 1

// Structs:
typedef struct dir_node {
    char path[PATH_MAX];
    struct dir_node *next;
} dir_node;

typedef struct dir_queue {
    dir_node *head;
    int size;
} dir_queue;

typedef struct thread_node {
    struct thread_node *next;
} thread_node;

typedef struct thread_queue {
    thread_node *head;
    int size;
    int busy;
} thread_queue;

typedef struct stat stats;
typedef struct dirent dirent;

// Function's declarations 
int is_empty(dir_queue *queue);
dir_queue *create_dir_queue();
dir_node *create_dir_node();
void free_dir_queue(dir_queue *queue);
void add(dir_queue queue, dir_node node);
int pop_path(dir_queue queue, char* path);
void dummy_func();


// Define global veriables
int num_threads, init_threads = 0, dead_threads = 0;
int waiting_threads = 0;
char *search_term;
atomic_int matched_files = 0;
int unmatched_files = 0;
int total_files = 0;

dir_queue *queue;

pthread_mutex_t start_mutex;
pthread_mutex_t matched_mutex;
pthread_mutex_t queue_mutex;

pthread_cond_t ready_cv;
pthread_cond_t *threads_cvs;


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
int main(int argc, char **argv)
{
    // Declerations
    char *search_path;
    int i, status/*, thread_status*/;
    dir_node *node;
    pthread_t *threads;

    // Validating input:
    if (argc != 4) {
        // TODO: Raise an error - wrong number of arguments
    }
    // TODO: Check the validation of search's path
    search_path = argv[1];
    search_term = argv[2];
    num_threads = atoi(argv[3]);

    // Create an array for the threads
    threads = (pthread_t*)calloc(num_threads, sizeof(pthread_t)); //TODO: FREE ! & Check valid
    
    // --- 1. Create a FIFO queue --------
    queue = create_dir_queue();

    // --- 2. Put the search root directory in the queue -------
    node = create_dir_node(search_term);
    add(queue, node);

    // --- 3. Create threads -------
    // Create diff CV for each thread
    threads_cvs = (pthread_cond_t*)calloc(num_threads, sizeof(pthread_cond_t)); // TODO: check valid & FREE

    // Launching threads
    pthread_mutex_init(&start_mutex, NULL);
    printf("Start Launching Threads.\n");
    for (i = 0; i < num_threads; ++i) {
        printf("Main: creating thread %d\n", i);
        pthread_cond_init(&threads_cvs[i], NULL);
        status = pthread_create(&threads[i], NULL, thread_routine, (void *)i);
        if (status != SUCCESS) {
            printf("ERROR in creating thread: %d with error: %s.\n", i, strerror(status));
            exit(-1);
        }
    }
    // Wait for all threads to be launched
    pthread_cond_wait(&ready_cv, &start_mutex);

    // Init mutexes
    // pthread_mutex_init(&matched_mutex, NULL); // TODO: delete???
    pthread_mutex_init(&queue_mutex, NULL);

    // 4. Signal all threads to start searching


}



// --- Additional functions ------

// --- Thread's routine ------
/*
    FLOW:
    1. Wait for all threads to be created and for the main thread
       to signal that the searching should start.
    2. dequeue the head from the FIFO queue. If queue empty:
        Wait until becomes non-empty. if waiting_threads == num_threads - exit.
    3. Iterate throught each directory entry in the directory obtained from the queue.
    4. When done iterating, repeat from 2.
*/
void *thread_routine(void *i)
{
    int thread_idx = (int)i;

    // 1. Wait for all threads to be created
    pthread_mutex_lock(&start_mutex);
    init_threads++;
    if (init_threads == num_threads) {
        // Signal to the main thread that all threads are initialized.
        pthread_cond_signal(&ready_cv);
    }
    pthread_mutex_unlock(&start_mutex);
    pthread_cond_wait(&threads_cvs[thread_idx], &start_mutex);

    // Get into the searching loop
    while (1)
    {
        // 2. Dequ

    }
}

int search_dir(char *dir_path)
{
    dirent *entry;
    char *entry_name;
    stats entry_stats;
    dir_node *entry_node;
    char entry_path[PATH_MAX];
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        // TODO: return error
    }

    // Iterate directory
    while ((entry = readdir(dir) != NULL))
    {
        // Get entry's full path
        entry_name = entry -> d_name;
        sprinft(entry_path, "%s/%s", dir_path, entry_name);
        if (stat(entry_path, &entry_stats) != SUCCESS) {
            // TODO: handle error
        }

        // If the entry is a directory
        if (S_ISDIR(entry_stats.st_mode)) {
            // Creating a node for the entry and adding it to the queue
            entry_node = create_dir_node(entry_path);
            pthread_mutex_lock(&queue_mutex);
            add(queue, entry_node);
            pthread_mutex_unlock(&queue_mutex);
        }

        // If the entry is a file
        if (S_ISREG(entry_stats.st_mode)) {
            // Entry contains the search term
            if (strstr(entry_name, search_term) != NULL) {
                // pthread_mutex_lock(&matched_mutex); //TODO: delete?
                // matched_files is atomic integer
                matched_files++;
                // pthread_mutex_unlock(&matched_mutex); // TODO: delete?
            }
            // Entry doesn't contains the search term (Debugging purposes)
            else {

            }
        }
    }

    closedir(dir);
}


// --- Directory's Queue & Node Functionallity ------

/*
    return 1 if queue is empty, else 0
*/
int is_empty(dir_queue *queue)
{
    return (queue -> size == 0);
}

/*
    return new queue
*/
dir_queue *create_dir_queue()
{
    queue = (dir_queue*)malloc(sizeof(dir_queue));
    if (queue == NULL) {
        // TODO: drop error
    }
    queue -> head = NULL;
    queue -> tail = NULL;
    queue -> size = 0;
    return queue;
}

/*
    return new directory node with path.
*/
dir_node *create_dir_node(char *path)
{
    dir_node *node = (dir_node*)malloc(sizeof(dir_node));
    if (node == NULL) {
        // TODO: drop error
    }
    strcpy(node -> path, path);
    node -> next = NULL;
    return node;
}

/*
    Add the node into the queue.
    Should always succeed.
*/
void add(dir_queue queue, dir_node node)
{
    node -> next = queue -> head;
    queue -> head = node;
    (queue -> size)++;
    // TODO: update tail ??? if not needed, delete it!
}

/*
    Pop last node and put its path into 'path' arg.
    return status (FAILURE / SUCCESS)
*/
int pop_path(dir_queue queue, char* path)
{
    dir_node *temp;
    if (size == 0) {
        return FAILURE;
    }
    strcpy(path, &(queue -> head -> path));
    temp = queue -> head;
    queue -> head = head -> next;
    (queue -> size)--;
    
    free(temp);
}

void free_dir_queue(dir_queue *queue)
{

}

void dummy_func() {
    return;
}
// ----- Threads' Queue & Node Functionallity -----


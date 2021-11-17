#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#define STATUS_SUCCESS 0
#define STATUS_FAILURE -1

typedef enum Case {
    case_DEFAULT,
    case_AND,
    case_PIPE,
    case_REDIRECT
} Case;

Case get_case(int count, char** arglist);
void my_signal_handler(int signum);
void zombie_handler(int signum);

int prepare()
{
    int status;
    struct sigaction null_action;
    memset(&null_action, 0, sizeof(null_action));
    null_action.sa_handler = my_signal_handler;
    null_action.sa_flags = SA_RESTART;
    status = sigaction(SIGINT, &null_action, NULL);
    // TODO: handle status

    // struct sigaction chld_action;
    // memset(&chld_action, 0, sizeof(chld_action));
    // chld_action.sa_handler = zombie_handler;
    // chld_action.sa_flags = SA_RESTART;
    // status = sigaction(SIGCHLD, &chld_action, NULL);
    // TODO: handle status

    return STATUS_SUCCESS;
}


int process_arglist(int count, char** arglist)
{
    int pid, status;
    Case curr_case = get_case(count, arglist);

    switch (curr_case) 
    {
        case case_DEFAULT:
            printf("case_DEFAULT\n"); // TODO: delete
            pid = fork();
            if (pid == 0) {
                signal(SIGINT, SIG_DFL);
                execvp(arglist[0], arglist);
            } else {
                wait(&status);
                return 1;
            }
            break;

        case case_AND:
            printf("case_AND\n"); // TODO: delete
            pid = fork();
            if (pid == 0) {

                arglist[count - 1] = NULL;
                execvp(arglist[0], arglist);
            } else {
                return 1;
            }
            break;
        
        case case_PIPE:
            printf("case_PIPE\n"); // TODO: delete
            int symbol_index;
            for (int i = 0; i < count; i++) {
                // Locate and save the '|'s index
                symbol_index = arglist[i][0] == '|' ? i : symbol_index;
            }
            arglist[symbol_index] = NULL;

            char** first_arglist = arglist;
            char** second_arglist = arglist + symbol_index + 1;

            int pipefd[2]; // 0 - read, 1 - write
            pipe(pipefd);
            int readerfd = pipefd[0];
            int writerfd = pipefd[1];
            
            pid = fork();
            if (pid == 0) {
                int pid_child = fork();
                
                if (pid_child == 0) { // Child 1 - writes to pipe
                    signal(SIGINT, SIG_DFL);
                    close(readerfd);
                    dup2(writerfd, 1);
                    execvp(first_arglist[0], first_arglist);
                    close(writerfd);

                } else { // Child 2 - read from pipe
                    signal(SIGINT, SIG_DFL);
                    close(writerfd); 
                    dup2(readerfd, 0);  
                    execvp(second_arglist[0], second_arglist);
                    close(readerfd);
                }

            } else { // Parent
                wait(&status);
            }

            break;

        case case_REDIRECT:
            printf("case_REDIRECT\n"); // TODO: delete
            char* file_name = arglist[count - 1];
            int fd = open(file_name, O_CREAT | O_TRUNC | O_WRONLY);
            arglist[count - 2] = NULL;
            pid = fork();
            
            if (pid == 0) {
                signal(SIGINT, SIG_DFL);
                dup2(fd, 1);
                execvp(arglist[0], arglist);
            } else {
                wait(&status);
                return 1;
            }
            break;
    }
    
    return STATUS_SUCCESS;
}


int finalize()
{
    return STATUS_SUCCESS;
}


/* 
###### Additional functions ######
*/

Case get_case(int count, char** arglist)
{   
    if (count < 2) {
        return case_DEFAULT;
    }
    if (arglist[count - 1][0] == '&') {
        return case_AND;
    }
    if (arglist[count - 2][0] == '>') {
        return case_REDIRECT;
    }
    for (int i = 0; i < count; i++) {
        if (arglist[i][0] == '|') {
            return case_PIPE;
        }
    }
    return case_DEFAULT;
}

void my_signal_handler(int signum)
{
    printf("\n42 <3\n");
    return;
}

void zombie_handler(int signum)
{
    int status;
    waitpid(-1, &status, WNOHANG);
}

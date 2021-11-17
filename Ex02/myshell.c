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

// Enums for cases' readability
typedef enum Case {
    case_DEFAULT,
    case_AND,
    case_PIPE,
    case_REDIRECT
} Case;

// Additional functions declaration
Case get_case(int count, char** arglist);
void my_signal_handler(int signum);
void zombie_handler(int signum);

int prepare()
{
    int status0, status1;

    // Nullify the SIGINT signal.
    struct sigaction null_action;
    memset(&null_action, 0, sizeof(null_action));
    null_action.sa_handler = my_signal_handler;
    null_action.sa_flags = SA_RESTART;
    status0 = sigaction(SIGINT, &null_action, NULL);

    // Modify SIGCHLD to handle zombies
    struct sigaction chld_action;
    memset(&chld_action, 0, sizeof(chld_action));
    chld_action.sa_handler = zombie_handler;
    chld_action.sa_flags = SA_RESTART;
    status1 = sigaction(SIGCHLD, &chld_action, NULL);

    if (status0 + status1 < 0) {
        perror("Error, sigaction.");
        return STATUS_FAILURE;
    }
    return STATUS_SUCCESS;
}


int process_arglist(int count, char** arglist)
{
    int pid, pid_child, status, symbol_index;
    Case curr_case = get_case(count, arglist);

    switch (curr_case) 
    {
        case case_DEFAULT:
            pid = fork();
            if (pid == 0) {
                signal(SIGINT, SIG_DFL);
                execvp(arglist[0], arglist);
                perror("Error, execvp:");
                exit(1);
            } else {
                wait(&status);
                if (status < 1) {
                    perror("Error, wait:");
                }
                return 1;
            }
            break;

        case case_AND:
            pid = fork();
            if (pid == 0) {

                arglist[count - 1] = NULL;
                execvp(arglist[0], arglist);
                perror("Error, execvp:");
                exit(1);
            } else {
                return 1;
            }
            break;
        
        case case_PIPE:
            pid_child = 0;
            symbol_index = 0;
            for (int i = 0; i < count; i++) {
                // Locate and save the '|'s index
                symbol_index = arglist[i][0] == '|' ? i : symbol_index;
            }
            arglist[symbol_index] = NULL;

            char** first_arglist = arglist;
            char** second_arglist = arglist + symbol_index + 1;

            int pipefd[2]; // 0 - read, 1 - write
            status = pipe(pipefd);
            if (status < 1) {
                perror("Error, pipe:");
            }

            int readerfd = pipefd[0];
            int writerfd = pipefd[1];
            
            pid = fork();
            if (pid == 0) { // Child 1 - writes to pipe
                signal(SIGINT, SIG_DFL);
                close(readerfd);
                dup2(writerfd, 1);
                close(writerfd);
                execvp(first_arglist[0], first_arglist);
                perror("Error, execvp:");
                exit(1);

            } else { 
                pid_child = fork();
                if (pid_child == 0) { // Child 2 - read from pipe
                    signal(SIGINT, SIG_DFL);
                    close(writerfd); 
                    dup2(readerfd, 0);
                    close(readerfd);
                    execvp(second_arglist[0], second_arglist);
                    perror("Error, execvp:");
                    exit(1);

                } else { // Parent
                    close(writerfd);
                    close(readerfd);

                    waitpid(pid, &status, 0);
                    if (status < 1) {
                        perror("Error, waitpid:");
                    }
                    waitpid(pid_child, &status, 0);
                    if (status < 1) {
                        perror("Error, waitpid:");
                    }
                    return 1;
                }

            }

            break;

        case case_REDIRECT:
            printf("case_REDIRECT\n"); // TODO: delete
            char* file_name = arglist[count - 1];
            int fd = open(file_name, O_CREAT | O_TRUNC | O_WRONLY, 0600);
            arglist[count - 2] = NULL;
            pid = fork();
            
            if (pid == 0) {
                signal(SIGINT, SIG_DFL);
                dup2(fd, 1);
                execvp(arglist[0], arglist);
                perror("Error, execvp:");
                exit(1);
            } else {
                wait(&status);
                if (status < 1) {
                    perror("Error, wait:");
                }
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
    return;
}

void zombie_handler(int signum)
{
    int status;
    waitpid(-1, &status, WNOHANG);
    if (status < 1) {
        perror("Error, waitpid:");
    }
}

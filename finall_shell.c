#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 80

char history[MAX_LINE] = "";

int parse_input(char *input, char **args, int *background) {
    int i = 0;
    *background = 0;
    
    if (strcmp(input, "!!") == 0) {
        if (strlen(history) == 0) {
            printf("No commands in history.\n");
            return -1;
        }
        printf("%s\n", history);
        strcpy(input, history);
    } else {
        strcpy(history, input);
    }

    char *token = strtok(input, " \t\n");
    while (token != NULL) {
        if (strcmp(token, "&") == 0) {
            *background = 1;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    return i;
}

void exec_pipe_command(char **args, int pipe_pos) {
    args[pipe_pos] = NULL;
    char **cmd1 = args;
    char **cmd2 = &args[pipe_pos + 1];

    int pipefd[2]; 
    if (pipe(pipefd) == -1) {
        perror("Pipe failed");
        return;
    }

    if (fork() == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        if (execvp(cmd1[0], cmd1) == -1) {
            perror("Cmd1 failed");
            exit(1);
        }
    }

    if (fork() == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        if (execvp(cmd2[0], cmd2) == -1) {
            perror("Cmd2 failed");
            exit(1);
        }
    }

    close(pipefd[0]);
    close(pipefd[1]);
    wait(NULL);
    wait(NULL);
}

int main(void) {
    char *args[MAX_LINE/2 + 1]; 
    char input[MAX_LINE];
    int should_run = 1;
    int background;

    while (should_run) {
        printf("uinxsh> ");
        fflush(stdout);

        if (fgets(input, MAX_LINE, stdin) == NULL) break;
        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0) continue;

        if (strcmp(input, "exit") == 0) {
            should_run = 0;
            continue;
        }

        int args_count = parse_input(input, args, &background);
        if (args_count <= 0) continue;

        if (strcmp(args[0], "cd") == 0) {
            if (args[1]) {
                if (chdir(args[1]) != 0) perror("cd failed");
            } else {
                fprintf(stderr, "cd: expected argument\n");
            }
            continue;
        }

        int pipe_pos = -1;
        for (int i = 0; i < args_count; i++) {
            if (strcmp(args[i], "|") == 0) {
                pipe_pos = i;
                break;
            }
        }

        if (pipe_pos != -1) {
            exec_pipe_command(args, pipe_pos);
        } else {
            pid_t pid = fork();
            if (pid < 0) {
                perror("Fork failed");
            } else if (pid == 0) {
                if (execvp(args[0], args) == -1) {
                    printf("Command not found: %s\n", args[0]);
                }
                exit(1);
            } else {
                if (!background) wait(NULL);
                else printf("[Background PID: %d]\n", pid);
            }
        }
    }
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

// Header files containing common data and function prototypes 
#include "task3.h"
#include "task2/task2.h"

// Contains builtin functions which are used when user enters the proper command in Tiny Shell
#include "task2/task2_builtin_functions.c"

// Execute a builtin command - Task 2
void execute_builtin_void(char **args) {
    int i = 0;
    while (builtin_list[i].name != NULL) {
        if (strcmp(args[0], builtin_list[i].name) == 0) {
            builtin_list[i].method(args);
            return;
        }
        i++;
    }
    fprintf(stderr, "Command not found: %s\n", args[0]);
}

// Execute a shell command pipeline - modified from Task 1
void execute_external_pipeline(char ***commands) {
    int i = 0;
    int pipe_fds[2];
    int input = 0; // File descriptor for input to the current stage

    while (commands[i] != NULL) {
        char **args = commands[i];

        // Create pipe for communication between stages
        if (commands[i + 1] != NULL) {
            // Check for pipe failures
            if (pipe(pipe_fds) == -1) {
                perror("Pipe creation failed");
                exit(EXIT_FAILURE);
            }
        }

        // Creake fork process
        pid_t pid_process = fork();

        // Check for failures with fork()
        if (pid_process < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid_process == 0) {
            // Child process

            // Redirect input from previous stage
            dup2(input, STDIN_FILENO);
            close(input);

            // Redirect output to next stage (if available)
            if (commands[i + 1] != NULL) {
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[1]);
            }

            // Execute the command
            if (execvp(args[0], args) == -1) {
                perror("Execvp failed");
                exit(EXIT_FAILURE);
            }
        } else {
            // Parent process

            // Close unused pipe ends
            if (commands[i + 1] != NULL) {
                close(pipe_fds[1]);
            }

            // Wait for child process to complete
            waitpid(pid_process, NULL, 0);

            // Update input for the next stage
            if (commands[i + 1] != NULL) {
                input = pipe_fds[0];
            }
        }

        i++;
    }
}

int main() {
    char input[256];

    while (true) {
        printf("Tiny Shell> ");

        if (fgets(input, sizeof(input), stdin) != NULL) {
            // Remove trailing newline character
            input[strcspn(input, "\n")] = '\0';

            // Tokenize the input
            char *token;
            char *tokens[256];
            int token_count = 0;

            token = strtok(input, " ");

            // Check that token is valid
            while (token != NULL) {
                tokens[token_count] = token;
                token_count++;
                token = strtok(NULL, " ");
            }
            tokens[token_count] = NULL;

            // Execute built-in command if available, otherwise execute external command pipeline
            if (token_count > 0) {
                execute_builtin_void(tokens);

                // Check if the first token is "exit" - if yes then exit program
                if (strcmp(tokens[0], "exit") == 0) {
                    break;
                }
            } else {
                fprintf(stderr, "No command entered\n");
            }
        }
    }

    return EXIT_SUCCESS;
}

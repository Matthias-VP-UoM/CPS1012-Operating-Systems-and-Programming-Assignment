#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Header files containing common data and function prototypes 
#include "task4.h"
#include "task2/task2.h"

// Contains builtin functions which are used when user enters the proper command in Tiny Shell
#include "task2/task2_builtin_functions.c"

#define MAX_COMMAND_LENGTH 1024
#define MAX_TOKENS 64

// Tokenize the input string into separate tokens
int tokenize_input(char *input, char **tokens) {
    int token_count = 0;
    char *token = strtok(input, " \t\n");

    // Check that token is valid
    while (token != NULL) {
        tokens[token_count] = token;
        token_count++;
        token = strtok(NULL, " \t\n");
    }
    tokens[token_count] = NULL;
    return token_count;
}

// Check if a string is enclosed in double quotation marks
int is_quoted(const char *str) {
    int length = strlen(str);
    return (length >= 2 && str[0] == '"' && str[length - 1] == '"');
}

// Execute a shell command pipeline
void execute_pipeline(char **pipeline[]) {
    int i = 0;
    int pipe_fds[2];
    int input_fd = 0;
    pid_t pid_process;

    while (pipeline[i] != NULL) {
        char **args = pipeline[i];

        // Check for errors with pipe creation
        if (pipe(pipe_fds) < 0) {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }

        // Create fork process
        pid_process = fork();

        // Check for failures with fork()
        if (pid_process < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid_process == 0) {
            // Child process
            if (i > 0) {
                // Set input to the previous stage's output
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }

            if (pipeline[i + 1] != NULL) {
                // Set output to the next stage's input
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[1]);
            }

            // Execute the command
            execvp(args[0], args);
            perror("Execvp failed");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            if (i > 0) {
                close(input_fd);
            }

            close(pipe_fds[1]);
            input_fd = pipe_fds[0];
            i++;
        }
    }

    // Wait for the last child process to finish
    while (wait(NULL) > 0);

    // Close the last input file descriptor
    close(input_fd);
}

// Execute a command
int execute_command(char **args) {
    // Check if the command is a builtin - if yes then execute
    for (int i = 0; builtin_list[i].name != NULL; i++) {
        if (strcmp(args[0], builtin_list[i].name) == 0) {
            return builtin_list[i].method(args);
        }
    }

    // Execute an external command pipeline
    execute_pipeline((char **[]) {args, NULL});
    return EXIT_FAILURE;
}

// Main shell loop
void shell_loop(void) {
    char input[MAX_COMMAND_LENGTH];
    char *tokens[MAX_TOKENS];

    while (1) {
        printf("tish$> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        // Tokenize the input
        int token_count = tokenize_input(input, tokens);
        if (token_count == 0) {
            continue;
        }

        /// Check if the first token is "exit" - if yes then exit program
        if (strcmp(tokens[0], "exit") == 0) {
            break;
        }

        // Check if the first token is a quoted string
        int is_quoted_input = is_quoted(tokens[0]);

        // Remove the quotation marks if present
        if (is_quoted_input) {
            memmove(tokens[0], tokens[0] + 1, strlen(tokens[0]));
            tokens[0][strlen(tokens[0]) - 1] = '\0';
        }

        // Execute the command
        int result = execute_command(tokens);

        // Add back the quotation marks if necessary
        if (is_quoted_input) {
            memmove(tokens[0] - 1, tokens[0], strlen(tokens[0]) + 1);
            tokens[0][0] = '"';
            tokens[0][strlen(tokens[0])] = '"';
        }
    }
}

int main(void) {
    shell_loop();
    return EXIT_SUCCESS;
}

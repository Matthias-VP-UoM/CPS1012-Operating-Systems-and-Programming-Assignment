#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

// Header files containing common data and function prototypes 
#include "task4.h"
#include "task2/task2.h"

// Contains builtin functions which are used when user enters the proper command in Tiny Shell
#include "task2/task2_builtin_functions.c"

#define MAX_COMMAND_LENGTH 1024
#define MAX_TOKENS 64

// Tokenize the input string
int tokenize_input(char *input, char **tokens) {
    char *token;
    int token_count = 0;
    int token_index = 0;
    int in_quoted_string = 0;
    int num_of_quotation_chars = 0;

    // Check that token is valid which inclusion to the character ';'
    while ((token = strtok(input, " \t\n\r;")) != NULL) {
        // Store length of current token
        int len = strlen(token);

        // Boolean variable to check if character '\\' has been accessed
        bool escape_char = false;
        for(int i = 0; i < len; i++) {
            // If current token character is escape character '\\'
            if (token[i] == '\\'){
                for(int j = i; j < len; j++){
                    token[j] = token[j+1];
                }
                escape_char = true;
                
                len--;
                i--;
            }

            // If current token character is character '"'
            if (token[i] == '"'){
                if ((i != 0 || token[i-1] != '\\') && !escape_char){    // Not escape character
                    in_quoted_string = 1;
                    for(int j = i; j < len; j++){
                        token[j] = token[j+1];
                    }
                
                    len--;
                    i--;
                    num_of_quotation_chars++;
                }else if (escape_char){ // Escape character
                    escape_char = false;
                }
            }
        }

        tokens[token_index++] = token;
        token_count++;

        // Reset in_quoted_string if the current token was the end of a quoted string
        if (in_quoted_string && token[strlen(token) - 1] == '"') {
            in_quoted_string = 0;
        }

        input = NULL;  // For subsequent calls to strtok
    }

    // Check that there is a valid number of double quotation characters
    if (num_of_quotation_chars % 2 != 0){
        fprintf(stderr, "Invalid number of quotation characters!\n");
        return 0; // do not perform operation
    }

    tokens[token_index] = NULL;
    return token_count;
}



// Check if a string is enclosed in double quotation marks (not used much)
int is_quoted(const char *str) {
    int length = strlen(str);
    return (length >= 2 && str[0] == '"' && str[length - 1] == '"');
}

// Execute a shell command pipeline - modified from Task 1
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

        // Check for any failures with fork()
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
    // Check if the command entered is a builtin command - if it is then execute
    for (int i = 0; builtin_list[i].name != NULL; i++) {
        if (strcmp(args[0], builtin_list[i].name) == 0) {
            return builtin_list[i].method(args);
        }
    }

    // Store the original stdin and stdout
    int orig_stdin = dup(STDIN_FILENO);
    int orig_stdout = dup(STDOUT_FILENO);

    int i = 0;
    int is_pipeline = 0;
    char **pipeline[64];

    FILE *input;
    FILE *output;

    bool used_output = false;
    bool used_input = false;

    // Check for any special characters - |, <, > , >> - Task 3
    while (args[i] != NULL && strchr(args[i], '"') == NULL) {
        if (strcmp(args[i], "|") == 0) {    // Pipe character
            is_pipeline = 1;
            pipeline[i] = NULL;

            // Create pipe for communication between commands
            int pipe_fds[2];

            // Check for pipe failures
            if (pipe(pipe_fds) < 0) {
                perror("Pipe creation failed");
                return EXIT_FAILURE;
            }

            // Execute the left side of the pipeline
            pid_t left_pid_process = fork();

            // Check for fork process failures
            if (left_pid_process < 0) {
                perror("Fork failed");
                return EXIT_FAILURE;
            } else if (left_pid_process == 0) {
                // Child process (left side)

                // Close the read end of the pipe
                close(pipe_fds[0]);

                // Redirect stdout to the write end of the pipe
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[1]);

                // Execute the left command
                execute_pipeline((char **[]) {args, NULL});
                exit(EXIT_FAILURE);
            }

            // Execute the right side of the pipeline
            pid_t right_pid_process = fork();

            // Check for fork process failures
            if (right_pid_process < 0) {
                perror("Fork failed");
                return EXIT_FAILURE;
            } else if (right_pid_process == 0) {
                // Child process (right side)

                // Close the write end of the pipe
                close(pipe_fds[1]);

                // Redirect stdin to the read end of the pipe
                dup2(pipe_fds[0], STDIN_FILENO);
                close(pipe_fds[0]);

                // Check for output redirection
                if (strcmp(args[i + 1], ">") == 0) {
                    // Output redirection to a file
                    char *output_file = args[i + 2];
                    FILE *output = freopen(output_file, "w", stdout);
                    if (output == NULL) {
                        perror("Freopen failed");
                        exit(EXIT_FAILURE);
                    }
                    args[i + 1] = NULL; // Set the current argument to NULL
                }

                // Execute the right command
                execute_pipeline((char **[]) {&args[i + 1], NULL});
                exit(EXIT_FAILURE);
            }

            // Parent process

            // Close both ends of the pipe
            close(pipe_fds[0]);
            close(pipe_fds[1]);

            // Wait for both child processes to finish
            int status;
            waitpid(left_pid_process, &status, 0);
            waitpid(right_pid_process, &status, 0);

        } else if (strcmp(args[i], ">") == 0) { // Output redirection to a file
            args[i] = NULL;  // Set the current argument to NULL
            char *output_file = args[i + 1];

            // Open the file in which to perform operation
            output = freopen(output_file, "w", stdout);
            if (output == NULL) {
                perror("Freopen failed");
                return EXIT_FAILURE;
            }
            used_output = true;
        } else if (strcmp(args[i], ">>") == 0) {    // Output redirection to a file (append mode)
            args[i] = NULL;  // Set the current argument to NULL
            char *output_file = args[i + 1];

            // Open the file in which to perform operation
            output = freopen(output_file, "a", stdout);
            if (output == NULL) {
                perror("Freopen failed");
                return EXIT_FAILURE;
            }
            used_output = true;
        } else if (strcmp(args[i], "<") == 0) { // Input redirection from a file
            args[i] = NULL;  // Set the current argument to NULL
            char *input_file = args[i + 1];

            // Open the file in which to perform operation
            input = freopen(input_file, "r", stdin);
            if (input == NULL) {
                perror("Freopen failed");
                return EXIT_FAILURE;
            }
            used_input = true;
        }

        if (is_pipeline) {
            pipeline[i] = args + i + 1;
        }

        i++;
    }

    // Execute command/s
    if (is_pipeline) {
        pipeline[i] = NULL;
        execute_pipeline(pipeline);
    } else {
        execute_pipeline((char **[]) {args, NULL});
    }

    // Restore the original stdin and stdout
    fflush(stdout);
    dup2(orig_stdin, STDIN_FILENO);
    dup2(orig_stdout, STDOUT_FILENO);

    // Close the duplicated file descriptors - ensures that tish$> is printed out again after input/output redirection
    close(orig_stdin);
    close(orig_stdout);

    return EXIT_SUCCESS;
}


// Main shell loop
void shell_loop(void) {
    char input[MAX_COMMAND_LENGTH];
    char *tokens[MAX_TOKENS];

    // perform this while the program is running
    while (true) {
        printf("tish$> ");
        fflush(stdout);

        char *statement = fgets(input, sizeof(input), stdin);

        if (statement == NULL) {
            break;
        }

        // Tokenize the input
        int token_count = tokenize_input(input, tokens);
        if (token_count == 0) {
            continue;
        }

        // Check if the first token is "exit" - if yes then exit program
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

// Main method - used to implement the program
int main(void) {
    shell_loop();
    return EXIT_SUCCESS;
}

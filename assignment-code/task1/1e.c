#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

// Execute commands with input and output redirection using fork()
pid_t fork_exec_pipe_ex(char **args, int pipe_fd_in[2], int pipe_fd_out[2], char *file_in, char *file_out, bool append_out) {
    // Create fork instance
    pid_t pid_process = fork();

    // Check for any failures with fork()
    if (pid_process < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid_process == 0) {
        // Child process

        // Input redirection
        if (file_in != NULL) {
            int fd_in = open(file_in, O_RDONLY);

            // Check for any input redirection errors
            if (fd_in < 0) {
                perror("Input redirection failed");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        } else {
            close(pipe_fd_in[1]);  // Close write end of input pipe
            dup2(pipe_fd_in[0], STDIN_FILENO);  // Redirect input from input pipe
        }

        // Output redirection
        if (file_out != NULL) {
            int fd_out;
            if (append_out) {
                fd_out = open(file_out, O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                fd_out = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }

            // Check for any output redirection errors
            if (fd_out < 0) {
                perror("Output redirection failed");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        } else {
            close(pipe_fd_out[0]);  // Close read end of output pipe
            dup2(pipe_fd_out[1], STDOUT_FILENO);  // Redirect output to output pipe
        }

        // Execute the program
        execvp(args[0], args);
        perror("Execvp failed");
        exit(EXIT_FAILURE);
    }

    return pid_process;
}

// Execute pipeline command
int execute_pipeline(char ***pipeline_args) {
    int num_stages = 0;
    while (pipeline_args[num_stages] != NULL) {
        num_stages++;
    }

    int pipe_fds[num_stages - 1][2];

    // Create pipes for each stage except the last
    for (int i = 0; i < num_stages - 1; i++) {
        // Check for errors with pipe creation
        if (pipe(pipe_fds[i]) < 0) {
            perror("Pipe creation failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_stages; i++) {
        char *file_in = NULL;
        char *file_out = NULL;
        bool append_out = false;

        if (i == 0 && pipeline_args[i + 1] != NULL) {
            // Input redirection for the first stage
            file_in = pipeline_args[i + 1][0];
        } else if (i == num_stages - 1 && pipeline_args[i - 1] != NULL) {
            // Output redirection for the last stage
            file_out = pipeline_args[i - 1][0];
            append_out = true;
        }

        pid_t pid_process = fork_exec_pipe_ex(pipeline_args[i], (i > 0) ? pipe_fds[i - 1] : NULL, (i < num_stages - 1) ? pipe_fds[i] : NULL, file_in, file_out, append_out);

        // Close pipe ends in the parent process
        if (i > 0) {
            close(pipe_fds[i - 1][0]);  // Close read end of previous stage's output pipe
            close(pipe_fds[i - 1][1]);  // Close write end of previous stage's output pipe
        }
        if (i < num_stages - 1) {
            close(pipe_fds[i][0]);  // Close read end of current stage's input pipe
            close(pipe_fds[i][1]);  // Close write end of current stage's input pipe
        }

        if (i == num_stages - 1) {
            // Return the last stage's process ID
            return pid_process;
        }
    }

    return -(EXIT_FAILURE);  // Error
}

// Execute pipeline command depending on whether to block parent's execution until last stage is completed
int execute_pipeline_async(char ***pipeline_args, bool async) {
    int last_pid_process = execute_pipeline(pipeline_args);

    if (!async) {
        int status;
        waitpid(last_pid_process, &status, 0);
        return WEXITSTATUS(status);
    }

    return EXIT_SUCCESS;
}

int main(void) {
    char *command_1[] = {"ls", "-la", NULL};
    char *command_2[] = {"grep", "john", "file_temp.txt", NULL};
    char *command_3[] = {"echo", "Hello, World!", ">", "file.txt", NULL};
    char **pipeline[] = {command_1, command_2, command_3, NULL};

    int result = execute_pipeline_async(pipeline, false);

    printf("Pipeline completed with exit status: %d\n", result);

    return EXIT_SUCCESS;
}

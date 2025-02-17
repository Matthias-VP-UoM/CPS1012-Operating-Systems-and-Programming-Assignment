#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Execute commands with input and output redirection using fork()
pid_t fork_exec_pipe(char **args, int pipe_fd_in[2], int pipe_fd_out[2]) {
    // Create fork instance
    pid_t pid_process = fork();

    int pipe_in = pipe_fd_in[0];
    int pipe_out = pipe_fd_out[1];

    // Check for errors with pipe creation
    if (pipe(pipe_fd_in) < 0 || pipe(pipe_fd_out) < 0) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    // Check for any failures with fork()
    if (pid_process < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid_process == 0) {
        // Child process

        // Close unused pipe ends
        close(pipe_fd_in[1]);
        close(pipe_fd_out[0]);

        // Redirect pipe ends to standard input and output
        dup2(pipe_in, STDIN_FILENO);
        dup2(pipe_out, STDOUT_FILENO);

        // Execute the first program
        execvp(args[0], args);
        perror("Execvp failed");
        exit(EXIT_FAILURE);
    }

    // Parent process

    // Close unused pipe ends
    close(pipe_fd_in[0]);
    close(pipe_fd_out[1]);

    return pid_process;
}

int main(void) {
    char *args1[] = {"ls", "-l", NULL};
    char *args2[] = {"echo", "Hello, World!", NULL};

    int pipe_fd[2];
    pid_t child_pid_process;

    if (pipe(pipe_fd) < 0) {
        perror("Pipe creation failed!");
        exit(EXIT_FAILURE);
    }

    // Launch the first program
    child_pid_process = fork_exec_pipe(args1, pipe_fd, pipe_fd);

    if (child_pid_process == 0) {
        // Child process (first program)
        close(pipe_fd[0]);  // Close unused read end of the pipe
    } else {
        // Parent process
        close(pipe_fd[1]);  // Close unused write end of the pipe

        // Launch the second program
        pid_t child_pid_process2 = fork_exec_pipe(args2, pipe_fd, pipe_fd);

        if (child_pid_process2 > 0) {
            printf("Child process %d finished execution.\n", child_pid_process2);
        }

        // Wait for the second program to finish
        waitpid(child_pid_process2, NULL, 0);
    }

    // Wait for the first program to finish
    waitpid(child_pid_process, NULL, 0);

    return EXIT_SUCCESS;
}

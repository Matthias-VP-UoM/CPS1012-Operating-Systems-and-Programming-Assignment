#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

// Execute pipeline command depending on whether to block parent's execution until last stage is completed
int execute_pipeline_async(char ***pipeline_args, bool async) {
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

    pid_t last_stage_pid_process;

    for (int i = 0; i < num_stages; i++) {
        // Create fork instance
        pid_t pid_process = fork();

        // Check for any failures with fork()
        if (pid_process < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pid_process == 0) {
            // Child process

            // Close unused pipe ends
            if (i > 0) {
                close(pipe_fds[i - 1][1]);  // Close write end of previous pipe
                dup2(pipe_fds[i - 1][0], STDIN_FILENO);  // Redirect input from previous pipe
            }

            if (i < num_stages - 1) {
                close(pipe_fds[i][0]);  // Close read end of current pipe
                dup2(pipe_fds[i][1], STDOUT_FILENO);  // Redirect output to current pipe
            }

            // Execute the program for this stage
            execvp(pipeline_args[i][0], pipeline_args[i]);
            perror("Execvp failed");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            if (i == num_stages - 1) {
                last_stage_pid_process = pid_process;  // Save the PID of the last stage
            }
        }
    }

    // Close all pipe ends in the parent process
    for (int i = 0; i < num_stages - 1; i++) {
        close(pipe_fds[i][0]);
        close(pipe_fds[i][1]);
    }

    if (!async) {
        // Block and wait for the last stage to finish
        int status;
        waitpid(last_stage_pid_process, &status, 0);
        return WEXITSTATUS(status);
    }

    return EXIT_SUCCESS;
}

int main(void) {
    char *command_1[] = {"ls", "-la", NULL};
    char *command_2[] = {"grep", "john", "file_temp.txt", NULL};
    char *command_3[] = {"wc", "-l", NULL};
    char **pipeline[] = {command_1, command_2, command_3, NULL};

    int result = execute_pipeline_async(pipeline, false);

    printf("Pipeline execution finished. Result: %d\n", result);

    return EXIT_SUCCESS;
}

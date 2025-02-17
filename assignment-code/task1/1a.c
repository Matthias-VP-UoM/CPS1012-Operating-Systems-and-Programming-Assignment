#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Execute command using fork()
pid_t fork_exec(char **args) {
    // Create fork instance
    pid_t pid_process = fork();

    // Check for any failures with fork()
    if (pid_process < 0) {
        perror("Fork failed");
        return pid_process;
    } else if (pid_process == 0) {
        // Child process
        execvp(args[0], args);
        perror("Execvp failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        waitpid(pid_process, &status, 0);
        return pid_process;
    }
}

int main(void) {
    char *args[] = {"ls", "-l", NULL};

    pid_t child_pid_process = fork_exec(args);

    if (child_pid_process > 0) {
        printf("Child process %d finished execution.\n", child_pid_process);
    }

    return EXIT_SUCCESS;
}
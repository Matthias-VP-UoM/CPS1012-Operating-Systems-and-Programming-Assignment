#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Header file containing common data and function prototypes
#include "task2.h"

int execute_builtin_task_2(char *input_string, char **args) {
    // Check if the command entered is a builtin command - if it is then execute
    int n = 0;
    while (builtin_list[n].name != NULL) {
        int min_length = strlen(builtin_list[n].name) < strlen(input_string) ? strlen(builtin_list[n].name) : strlen(input_string);
        if (strncmp(builtin_list[n].name, input_string, min_length) == 0) {
            return builtin_list[n].method(args);
        }
        n++;
    }

    return -(EXIT_FAILURE); // Builtin command not found
}

int main(void) {
    char *command = "exit";
    char *args[] = {command, NULL};

    int result = execute_builtin_task_2(command, args);

    if (result == -1) {
        printf("Builtin command not found!\n");
    } else {
        printf("Builtin command executed successfully!\n");
    }

    return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Header file containing common data and function prototypes 
#include "task2.h"

// Execute exit command
int builtin_exit(char **args) {
    // Perform any necessary cleanup or additional logic here
    exit(EXIT_SUCCESS);
}

// Prints an example sentence
int builtin_example(char **args) {
    printf("This is an example of a built-in command.\n");
    return EXIT_SUCCESS;
}

// Execute cd command - does not do anything is this program but added to prevent unnecessary errors
int builtin_cd(char **args){
    return EXIT_SUCCESS;
}

// Execute cwd command - does not do anything is this program but added to prevent unnecessary errors
int builtin_cwd(char **args){
    return EXIT_SUCCESS;
}

// Execute ver command - does not do anything is this program but added to prevent unnecessary errors
int builtin_ver(char **args){
    return EXIT_SUCCESS;
}

// Defines list of type builtin_command - different from that in task2.h since it contains limited functionality
struct builtin_command builtin_list_2a[] = {
    {"exit", &builtin_exit},
    {"example", &builtin_example},
    {NULL, NULL} // End of the list marker
};

// Execute a builtin command
int execute_builtin_task_2(char *input_string, char **args) {
    int n = 0;
    while (builtin_list_2a[n].name != NULL) {
        int min_length = strlen(builtin_list_2a[n].name) < strlen(input_string) ? strlen(builtin_list_2a[n].name) : strlen(input_string);
        if (strncmp(builtin_list_2a[n].name, input_string, min_length) == 0) {
            return builtin_list_2a[n].method(args);
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

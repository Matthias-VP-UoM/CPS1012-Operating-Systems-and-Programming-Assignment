// Contains builtin functions which are used when user enters the proper command in Tiny Shell

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PATH_MAX 4096

// Execute exit command
int builtin_exit(char **args) {
    // Ignore the args parameter
    exit(EXIT_SUCCESS);
}

// Execute cd command
int builtin_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "cd: No directory specified\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
    }
    return EXIT_SUCCESS;
}

// Execute cwd command
int builtin_cwd(char **args) {
    // Ignore the args parameter
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("cwd");
    }
    return EXIT_SUCCESS;
}

// Execute ver command
int builtin_ver(char **args) {
    // Ignore the args parameter
    printf("Tiny Shell (Tish) - Version 1.0\n");
    printf("Author: Matthias Vassallo Pulis\n");
    printf("This shell is tiny, but it gets the job done!\n");
    return EXIT_SUCCESS;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS 100
#define MAX_TOKEN_LENGTH 100

// Tokenize the input string
void tokenize_input(char *input, char **tokens) {
    const char delimiter[] = " ";
    char *token = strtok(input, delimiter);
    int token_count = 0;

    // Check that token is valid
    while (token != NULL) {
        tokens[token_count] = token;
        token_count++;

        // In case cumber of token exceeds the number of max tokens
        if (token_count >= MAX_TOKENS) {
            fprintf(stderr, "Too many tokens! Maximum token count exceeded!\n");
            return;
        }

        token = strtok(NULL, delimiter);
    }

    tokens[token_count] = NULL;
}

int main(void) {
    char input[MAX_TOKEN_LENGTH * MAX_TOKENS]; // Assuming input line length won't exceed the maximum tokens
    char *tokens[MAX_TOKENS];

    printf("Enter a command: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fprintf(stderr, "Error reading input.\n");
        return EXIT_FAILURE;
    }

    // Remove newline character if present
    char *newline = strchr(input, '\n');
    if (newline != NULL) {
        *newline = '\0';
    }

    tokenize_input(input, tokens);

    // Validate tokens based on the specified rules
    int i = 0;
    while (tokens[i] != NULL) {
        // Check for pipe operator
        if (strcmp(tokens[i], "|") == 0) {
            if (i == 0 || tokens[i + 1] == NULL) {
                fprintf(stderr, "Invalid use of pipe operator.\n");
                return EXIT_FAILURE;
            }
        }

        // Check for output redirection operators
        if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i], ">>") == 0) {
            if (tokens[i + 1] == NULL) {
                fprintf(stderr, "Invalid use of output redirection operator!\n");
                return EXIT_FAILURE;
            }
        }

        // Check for input redirection operator
        if (strcmp(tokens[i], "<") == 0) {
            if (tokens[i + 1] == NULL) {
                fprintf(stderr, "Invalid use of input redirection operator!\n");
                return EXIT_FAILURE;
            }
        }

        i++;
    }

    // Print the tokenized input for testing
    i = 0;
    while (tokens[i] != NULL) {
        printf("Token %d: %s\n", i, tokens[i]);
        i++;
    }

    return EXIT_SUCCESS;
}

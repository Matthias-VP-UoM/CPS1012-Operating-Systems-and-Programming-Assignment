// constant since PATH_MAX does not exist in limits.h
#define PATH_MAX 4096

// Define builtin_t datatype
typedef int (*builtin_t)(char **);

// Defines command
struct builtin_command {
    char *name;
    builtin_t method;
};

// function prototypes
int builtin_exit(char **args);
int builtin_cd(char **args);
int builtin_cwd(char **args);
int builtin_ver(char **args);
int execute_builtin_task_2(char *input_string, char **args);


// Defines list of type builtin_command
struct builtin_command builtin_list[] = {
    {"exit", &builtin_exit},
    {"cd", &builtin_cd},
    {"cwd", &builtin_cwd},
    {"ver", &builtin_ver},
    {NULL, NULL} // End of the list marker
};
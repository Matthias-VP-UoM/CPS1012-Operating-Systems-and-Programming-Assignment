#include <sys/types.h>
#include <sys/wait.h>

// function prototypes
pid_t fork_exec_pipe_ex(char **args, int pipe_fd_in[2], int pipe_fd_out[2], char *file_in, char *file_out, bool append_out);
int execute_pipeline(char ***pipeline_args);
int execute_pipeline_async(char ***pipeline_args, bool async);

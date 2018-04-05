#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

char **make_args(char *line, size_t *param_qtty) {
    char **args = NULL;
    *param_qtty = 0;
    char *token = strtok(line, " \n");
    while (token) {
        args = realloc(args, sizeof(char *) * ++(*param_qtty));
        if (args == 0) exit(EXIT_FAILURE);
        else args[(*param_qtty) - 1] = token;
        token = strtok(NULL, " \n");
    }
    args = realloc(args, sizeof(char *) * ++(*param_qtty));
    args[(*param_qtty) - 1] = NULL;
    return args;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Bad parameters\n");
        exit(EXIT_FAILURE);
    }
    const char *filename = argv[1];

    FILE *file;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    size_t line_no = 0;

    file = fopen(filename, "r");
    if (file == 0) {
        perror("Fopen error: ");
        exit(EXIT_FAILURE);
    }
    while ((read = getline(&line, &len, file)) != -1) {
        ++line_no;
        if (line[0] == '\n') continue;
//        printf("%s", line);

        size_t param_qtty;
        char **args = make_args(line, &param_qtty);
        if (param_qtty < 2) continue;

        pid_t pid;
        pid = fork();
        if (pid == 0) {
//            child's work
            if (execvp(args[0], args) < 0) {
                perror("Exec error: ");
                exit(EXIT_FAILURE);
            }
        } else if (pid == -1) {
            perror("Fork error: ");
            exit(EXIT_FAILURE);
        } else {
//            parent's work
            pid_t end_id;
            int status;
            do {
                end_id = waitpid(pid, &status, WNOHANG | WUNTRACED);
                if (end_id == -1) {
                    perror("Waitpid error: ");
                    exit(EXIT_FAILURE);
                } else if (end_id == pid) {
                    if (WIFEXITED(status)) {
                        int exit_status = WEXITSTATUS(status);
                        if (exit_status != 0) {
                            fprintf(
                                    stderr,
                                    "Execution of line %zu (%s) failed! Exit status: %d\n",
                                    line_no,
                                    args[0],
                                    exit_status
                            );
                            exit(EXIT_FAILURE);
                        }
                    } else if (WIFSIGNALED(status)) {
                        perror("Child process ended because of an uncaught signal: ");
                        if (WCOREDUMP(status)) {
                            fprintf(stderr, "Core dumped :(\n");
                        }
                    } else if (WIFSTOPPED(status))
                        perror("Child process has stopped: ");
                }
            } while (end_id == 0);
        }
        free(args);
    }
    if (line) free(line);
    fclose(file);

    exit(EXIT_SUCCESS);
}

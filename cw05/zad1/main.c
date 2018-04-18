#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "rainbow.h"

#define DELIM " \n"
#define PIPEC "|"

void show_start_msg(size_t line_no, char *task_name) {
    printf(
            GRN
            BOLD
            ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n"
            YEL
            "Starting execution of task from line %zu.\n"
            "\"%s\"\n"
            GRN
            "----------------------------------------"
            RESET
            "\n",
            line_no,
            task_name
    );
}

void show_end_msg() {
    printf(
            RED
            BOLD
            "----------------------------------------\n"
            "End of task execution\n"
            RED
            "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
            RESET
            "\n"
    );
}

char **make_args(char *line, size_t *param_qtty, char *delim) {
    char **args = NULL;
    *param_qtty = 0;
    char *token = strtok(line, delim);
    while (token) {
        args = realloc(args, sizeof(char *) * ++(*param_qtty));
        if (args == 0) {
            perror("Memory reallocation error");
            free(args);
            return NULL;
        } else {
            args[(*param_qtty) - 1] = token;
        }
        token = strtok(NULL, delim);
    }
    args = realloc(args, sizeof(char *) * ++(*param_qtty));
    args[(*param_qtty) - 1] = NULL;
    return args;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Too few parameters\n");
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

        size_t comm_qtty;
        char **comm = make_args(line, &comm_qtty, PIPEC);
        for (size_t i = 0; i < comm_qtty; ++i) {
            char *com = comm[i];
            size_t param_qtty;
            char **args = make_args(com, &param_qtty, DELIM);
            if (args == NULL) {
                exit(EXIT_FAILURE);
            }
            if (param_qtty < 2) continue;

            show_start_msg(line_no, args[0]);

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
                                        BCYN
                                        BLK
                                        "Execution of line %zu (%s) failed! Exit status: %d"
                                        RESET
                                        "\n",
                                        line_no,
                                        args[0],
                                        exit_status
                                );
                                exit(EXIT_FAILURE);
                            }
                        } else if (WIFSIGNALED(status)) {
                            fprintf(
                                    stderr,
                                    BMAG
                                    BLK
                                    "Child process ended because of an uncaught signal: %d"
                                    RESET
                                    "\n",
                                    WTERMSIG(status)
                            );
                            if (WCOREDUMP(status)) {
                                fprintf(stderr, "Core dumped :c\n");
                            }
                        }
                    }
                } while (end_id == 0);
            }

            show_end_msg();

            free(args);
        }
    }
    if (line) free(line);
    fclose(file);

    exit(EXIT_SUCCESS);
}


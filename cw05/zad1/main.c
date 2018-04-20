#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "rainbow.h"

#define DELIM " \n"
#define PIPEC "|"

int in_pipe[2];
int out_pipe[2];

void show_start_msg(size_t line_no, char *task_name) {
    printf(
            GRN
            BOLD
            ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n"
            YEL
            "Starting execution of line %zu.\n"
            "%s"
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

int first_non_space_is_newline(char *line, size_t len) {
    size_t i;
    for (i = 0; i < len && line[i] == ' '; ++i);
    return (line[i] == '\n');
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

void process_file(const char *filename) {
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
        if (first_non_space_is_newline(line, read)) continue;
//        if (line[0] == '\n') continue;
//        printf("%s : %zu", line, read);

//        show_start_msg(line_no, line);

        // initial set of pipes
        in_pipe[0] = STDIN_FILENO;
        in_pipe[1] = -1;

        size_t comm_qtty;
        char **comm = make_args(line, &comm_qtty, PIPEC);
        for (size_t i = 0; i < comm_qtty - 1; ++i) {
            char *com = comm[i];
            size_t param_qtty;
            char **args = make_args(com, &param_qtty, DELIM);
            if (args == NULL) {
                exit(EXIT_FAILURE);
            }
            if (param_qtty < 2) continue;

            // create new output pipe (only if not last process)
            if (i == comm_qtty - 2) {
                // final set of pipes
                out_pipe[0] = -1;
                out_pipe[1] = STDOUT_FILENO;
            } else {
                if (pipe(out_pipe) == -1) {
                    perror("Pipe creation error");
                    exit(EXIT_FAILURE);
                }
            }

            pid_t pid;
            pid = fork();
            if (pid == 0) {
//            child's work

                // connect in and out pipes
                dup2(in_pipe[0], STDIN_FILENO);
                dup2(out_pipe[1], STDOUT_FILENO);

                // close unused sides
                close(in_pipe[1]);
                close(out_pipe[0]);

                if (execvp(args[0], args) < 0) {
                    perror("Exec error: ");
                    exit(EXIT_FAILURE);
                }
            } else if (pid == -1) {
                perror("Fork error: ");
                exit(EXIT_FAILURE);
            } else {
//            parent's work
                //none
            }

            // close pipes
            if (i > 0) {
                close(in_pipe[0]);
            }
            if (i < comm_qtty - 2) {
                close(out_pipe[1]);
            }

            in_pipe[0] = out_pipe[0];
            in_pipe[1] = out_pipe[1];

            free(args);
        }

        while (wait(NULL)) {
            if (errno == ECHILD) {
//                perror("Waiting error");
//                usleep(10000);
                break;
            }
        }

//        show_end_msg();

//        pid_t end_id;
//        int status;
//        do {
//            end_id = waitpid(pid, &status, WNOHANG | WUNTRACED);
//            if (end_id == -1) {
//                perror("Waitpid error: ");
//                exit(EXIT_FAILURE);
//            } else if (end_id == pid) {
//                if (WIFEXITED(status)) {
//                    int exit_status = WEXITSTATUS(status);
//                    if (exit_status != 0) {
//                        fprintf(
//                                stderr,
//                                BCYN
//                                BLK
//                                "Execution of line %zu (%s) failed! Exit status: %d"
//                                RESET
//                                "\n",
//                                line_no,
//                                args[0],
//                                exit_status
//                        );
//                        exit(EXIT_FAILURE);
//                    }
//                } else if (WIFSIGNALED(status)) {
//                    fprintf(
//                            stderr,
//                            BMAG
//                            BLK
//                            "Child process ended because of an uncaught signal: %d"
//                            RESET
//                            "\n",
//                            WTERMSIG(status)
//                    );
//                    if (WCOREDUMP(status)) {
//                        fprintf(stderr, "Core dumped :c\n");
//                    }
//                }
//            }
//        } while (end_id == 0);
    }
    if (line) free(line);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Too few parameters\n");
        exit(EXIT_FAILURE);
    }
    const char *filename = argv[1];

    process_file(filename);

    exit(EXIT_SUCCESS);
}


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/resource.h>

#include "rainbow.h"

#define DELIM " \n"

#define PREEXIT_FREE() {\
free(args);             \
if (line) free(line);   \
fclose(file);           \
}

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

void show_times(char *name, struct rusage *begin, struct rusage *end) {
    time_t u_time =
            (end->ru_utime.tv_sec - begin->ru_utime.tv_sec) * 1000000 + end->ru_utime.tv_usec - begin->ru_utime.tv_usec;
    time_t s_time =
            (end->ru_stime.tv_sec - begin->ru_stime.tv_sec) * 1000000 + end->ru_stime.tv_usec - begin->ru_stime.tv_usec;
    printf(
            RED
            BOLD
            "----------------------------------------\n"
            YEL
            "End of execution.\n"
            "User time: %ld us\n"
            "System time: %ld us\n"
            RED
            "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
            RESET
            "\n",
            u_time,
            s_time
    );
}

void limit_res(int type, rlim_t value) {
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = (rlim_t) value;
    if (setrlimit(type, &rlim) == -1) {
        perror("Setrlimit error (CPU)");
        exit(EXIT_FAILURE);
    }
}

char **make_args(char *line, size_t *param_qtty, size_t *cpu_limit, size_t *mem_limit) {
    char **args = NULL;
    *param_qtty = 0;
    size_t flag = 0;
    char *token = strtok(line, DELIM);

    args = realloc(args, sizeof(char *) * ++(*param_qtty));
    if (args == 0) {
        perror("Memory allocation error");
        free(args);
        return NULL;
    } else {
        args[(*param_qtty) - 1] = token;
    }
    token = strtok(NULL, DELIM);

    if (token == NULL) {
        free(args);
        return NULL;
    }
    *cpu_limit = strtoul(token, NULL, 10);
//    printf("cpu_limit: %zu\n", *cpu_limit);
    token = strtok(NULL, DELIM);

    if (token == NULL) {
        free(args);
        return NULL;
    }
    *mem_limit = strtoul(token, NULL, 10);
//    printf("mem_limit: %zu\n", *mem_limit);
    token = strtok(NULL, DELIM);

    while (token) {
        args = realloc(args, sizeof(char *) * ++(*param_qtty));
        if (args == 0) {
            perror("Memory reallocation error");
            free(args);
            return NULL;
        } else {
            args[(*param_qtty) - 1] = token;
        }
        token = strtok(NULL, DELIM);
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
        size_t cpu_limit;
        size_t mem_limit;
        char **args = make_args(line, &param_qtty, &cpu_limit, &mem_limit);
        if (args == NULL) {
            //free..
            if (line) free(line);
            fclose(file);
            exit(EXIT_FAILURE);
        }

        show_start_msg(line_no, args[0]);

        // timing
        struct rusage ru_begin, ru_end;
        getrusage(RUSAGE_CHILDREN, &ru_begin);

        pid_t pid;
        pid = fork();
        if (pid == 0) {
//            child's work
            // set cpu limit
            limit_res(RLIMIT_CPU, cpu_limit);
            // set mem limit
            limit_res(RLIMIT_AS, mem_limit * 1000000);
            // execute
            if (execvp(args[0], args) < 0) {
                perror("Exec error");
                exit(EXIT_FAILURE);
            }
        } else if (pid == -1) {
            perror("Fork error");
            PREEXIT_FREE();
            exit(EXIT_FAILURE);
        } else {
//            parent's work
            pid_t end_id;
            int status;
            do {
                end_id = waitpid(pid, &status, WNOHANG | WUNTRACED);
                if (end_id == -1) {
                    perror("Waitpid error");
                    PREEXIT_FREE();
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
                            //free..
                            PREEXIT_FREE();
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

        // timing
        getrusage(RUSAGE_CHILDREN, &ru_end);
        show_times(args[0], &ru_begin, &ru_end);

        free(args);
    }
    if (line) free(line);
    fclose(file);

    exit(EXIT_SUCCESS);
}


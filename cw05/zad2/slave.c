#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "rainbow.h"

char *get_date() {
    // initialize variables and structures
    FILE *date_output;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // open a pipe
    date_output = popen("date", "r");

    if (date_output == NULL) {
        perror("Error opening a named pipe");
        exit(EXIT_FAILURE);
    }

    // read a line from pipe
    if ((read = getline(&line, &len, date_output)) == -1) {
        perror("Error getting date");
        exit(EXIT_FAILURE);
    }

    pclose(date_output);

    return line;
}

int main(int argc, char *argv[]) {

    srand((unsigned int) time(NULL) * getpid());

    // parse command line
    if (argc < 3) {
        fprintf(
                stderr,
                RED
                "Too few parameters"
                RESET
                "\n");
        exit(EXIT_FAILURE);
    }
    const char *pipe_path = argv[1];
    size_t count = strtoul(argv[2], NULL, 10);

    // open a pipe
    FILE *pipe;
    pipe = fopen(pipe_path, "r+");
    if (pipe == 0) {
        perror("Error opening a named pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = getpid();
    // print pid
    printf("%d\n", pid);

    // write date to a pipe `count` times
    for (size_t i = 0; i < count; ++i) {
        char *date = get_date();
        fprintf(pipe, "%d: %s", pid, date);
        free(date);
        fflush(pipe);

        sleep((rand() % 4) + 2);
    }

    exit(EXIT_SUCCESS);
}


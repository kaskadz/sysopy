#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "rainbow.h"

int check_for_existance_and_being_fifo(const char *filename) {
    struct stat st;
    if (stat(filename, &st) != 0) {
        return 0;           // doesn't exist
    }
    if (S_ISFIFO(st.st_mode)) {
        return 1;           // exists and is FIFO
    } else {
        return -1;          // exists, but is not FIFO
    }
}

int main(int argc, char *argv[]) {
    // parse command line
    if (argc < 2) {
        fprintf(
                stderr,
                RED
                "Too few parameters"
                RESET
                "\n");
        exit(EXIT_FAILURE);
    }
    const char *pipe_path = argv[1];

    int status = check_for_existance_and_being_fifo(pipe_path);
    if (status == 0) {          // pipe has to be created
        // create a named pipe
        if (mkfifo(pipe_path, 0644) == -1) {
            perror("Error creating a named pipe");
            exit(EXIT_FAILURE);
        }
    } else if (status == -1) {  // pipe can't be created
        fprintf(
                stderr,
                RED
                "File named %s already exists. Please specify another name."
                RESET
                "\n",
                pipe_path
        );
        exit(EXIT_FAILURE);
    }                           // pipe already exists

    // initialize variables and structures
    FILE *pipe;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    printf(
            GRN
            "Hello!"
            RESET
            "\n");

    // open a pipe
    pipe = fopen(pipe_path, "r");
    if (pipe == 0) {
        perror("Error opening a named pipe");
        exit(EXIT_FAILURE);
    }

    // read from pipe line by line
    while (1) {
        if ((read = getline(&line, &len, pipe)) != -1) {
            if (strcmp(line, "ala123\n") == 0) break;
            printf("%s", line);
        }
    }

    printf(
            GRN
            "Googbye!"
            RESET
            "\n");
    exit(EXIT_SUCCESS);
}


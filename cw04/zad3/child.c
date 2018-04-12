#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>

#include "rainbow.h"

int received = 0;

void respond_handler(int signal) {
    printf(
            "["
            GRN
            "CHILD"
            RESET
            "] "
            "Child received %dnth signal %d\n",
            ++received,
            signal
    );
    kill(getppid(), signal);
}

void exit_handler(int signal) {
    printf(
            "["
            GRN
            "CHILD"
            RESET
            "] "
            "Child received exit signal %d\n",
            signal
    );
    exit(EXIT_SUCCESS);
}

int main(void) {
    signal(SIGUSR1, &respond_handler);
    signal(SIGRTMIN + 1, &respond_handler);
    signal(SIGUSR2, &exit_handler);
    signal(SIGRTMIN + 2, &exit_handler);

    while (1);
}

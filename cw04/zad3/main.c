#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define _XOPEN_SOURCE

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "rainbow.h"

#define CUSTOM_SIG_1 (SIGRTMIN + 1);
#define CUSTOM_SIG_2 (SIGRTMIN + 2);

int L;
int Type;

pid_t child;
bool responded = false;
int sent_signals_counter = 0;
int received_signals_counter = 0;

pid_t make_child(void); // spawn a child

void define_signal_handling(); // define handlers

// custom handlers
void response_handler(int signal);

void sigint_handler(int signal);

// main
int main(int argc, char *argv[]) {
    // command line parsing
    if (argc < 2) {
        fprintf(
                stderr,
                RED
                "Too few parameters"
                RESET
                "\n"
        );
        exit(EXIT_FAILURE);
    }

    L = strtoul(argv[1], NULL, 10);
    Type = strtoul(argv[2], NULL, 10);

    // setting handlers
    define_signal_handling();

    // define which signals to send
    int signal1;
    int signal2;
    if (Type == 3) {
        signal1 = CUSTOM_SIG_1;
        signal2 = CUSTOM_SIG_2;
    } else {
        signal1 = SIGUSR1;
        signal2 = SIGUSR2;
    }

    // spawn child
    child = make_child();

    sleep(1);

    // signal sending loop
    for (int i = 0; i < L; ++i) {
        printf(
                "["
                YEL
                "PARENT"
                RESET
                "] "
                "Sending %dnth signal %d to child\n",
                ++sent_signals_counter,
                signal1
        );

        responded = false;
        kill(child, signal1); // send a signal to a child

        if (Type == 2) {
            while (!responded); // wait for response from a child
        }
    }
    printf(
            "["
            YEL
            "PARENT"
            RESET
            "] "
            "Sent all signals\n"
    );
    // send ending signal and wait for a child to end
    kill(child, signal2);
    waitpid(child, NULL, WNOHANG);
}

void define_signal_handling(void) {
    struct sigaction *sg = calloc(1, sizeof(struct sigaction));

    sg->sa_flags = 0;//| SA_NODEFER;
    sg->sa_handler = &response_handler;
    sigaction(SIGUSR1, sg, NULL);
    int csig = CUSTOM_SIG_1
    sigaction(csig, sg, NULL);

    sg->sa_handler = &sigint_handler;
    sigaction(SIGINT, sg, NULL);

    free(sg);
}

pid_t make_child(void) {
    pid_t pid = fork();
    if (pid == 0) {
        execl("./child", "child", NULL);
        exit(EXIT_FAILURE);
    } else if (pid == -1) {
        perror("Fork error");
        exit(EXIT_FAILURE);
    } else {
        return pid;
    }
}

void response_handler(int signal) {
    responded = true;
    printf(
            "["
            YEL
            "PARENT"
            RESET
            "] "
            "Parent received %dnth signal %d\n",
            ++received_signals_counter,
            signal
    );
}

void sigint_handler(int signal) {
    (void) signal;
    int csig = CUSTOM_SIG_2;
    kill(child, (Type == 3) ? csig : SIGUSR2);
    exit(EXIT_SUCCESS);
}

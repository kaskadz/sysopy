#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

pid_t pid;
sig_atomic_t isChildRunning;
sig_atomic_t hasAChild;

void start_child() {
    pid = fork();
    if (pid == 0) {
        // run infinite loop script
        execlp("bash", "bash", "./date.sh", NULL);
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Fork error");
        exit(EXIT_FAILURE);
    } else {
        // report that child is running
        isChildRunning = TRUE;
    }
}

void end_child() {
    kill(pid, SIGKILL);
    // report that child is not running
    isChildRunning = FALSE;
}

void handleTSTP(int signum) {
    if (!isChildRunning) {
        // start a child
        start_child();
    } else {
        // kill child
        end_child();
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGINT);
        sigdelset(&mask, SIGTSTP);
        sigsuspend(&mask);
    }
}

void handleINT(int signum) {
    // kill a child
    end_child();
    printf("Odebrano sygnał SIGINT\n");
    exit(EXIT_SUCCESS);
}

int main() {
    // defining signal handling behavior via signal function
    signal(SIGTSTP, handleTSTP);

    // defining signal handling behavior via sigaction function and struct sigaction
    struct sigaction act;
    act.sa_handler = handleINT;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    // infinite loop
    start_child();
    while (1);
}
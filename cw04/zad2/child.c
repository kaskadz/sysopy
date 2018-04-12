#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#define MAX_DELAY 10
int delay;

void set_rand_delay() {
    delay = rand() % (MAX_DELAY + 1);
//    printf("[%d] Delay: %d\n", getpid(), delay);
}

int rand_rt_int() {
    return SIGRTMIN + (rand() % (SIGRTMAX - SIGRTMIN + 1));
}

void handler(int signal) {
    (void) signal;
    kill(getppid(), rand_rt_int());

    exit(delay);
}

int main(void) {
    srand(time(NULL) * getpid());
    set_rand_delay();

    signal(SIGALRM, &handler);

    // sleep and send request
    sleep(delay);
    kill(getppid(), SIGUSR1);

    // wait for a signal
    pause();
}

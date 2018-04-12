#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#define _XOPEN_SOURCE

#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "rainbow.h"

#define D_SPAWN
#define D_SIGRT
#define D_CHLD
#define D_REQUEST
#define D_ALLOWS

int N;
int K;

sig_atomic_t request_counter = 0;
sig_atomic_t alive_children_counter = 0;

pid_t *children = NULL; // pids of children
bool *permission_granted = NULL; // start requests

void kinder_mache();

int get_child_id(pid_t pid);

pid_t make_child(void);

void define_signal_handling();

void send_allow(pid_t child);

void allow_all(void);

void request_handler(int signal, siginfo_t *info, void *ucontext);

void sigchld_handler(int signal, siginfo_t *info, void *ucontext);

void rt_handler(int signal, siginfo_t *info, void *ucontext);

//void sigint_handler(int signal, siginfo_t *info, void *ucontext) ;
void sigint_handler(void);

int main(int argc, char **argv) {

    // Parse command line
    if (argc < 3) {
        fprintf(stderr, "Too few parameters\n");
        exit(EXIT_FAILURE);
    }

    N = strtoul(argv[1], NULL, 10);
    K = strtoul(argv[2], NULL, 10);

    if (N < 1) {
        fprintf(stderr, "N should be GTE 1\n");
        exit(EXIT_FAILURE);
    }

    if (N < K) {
        fprintf(stderr, "N should be GTE M\n");
        exit(EXIT_FAILURE);
    }
    // End of parsing

    children = calloc(N, sizeof(pid_t));
    permission_granted = calloc(N, sizeof(bool));

    define_signal_handling();

    kinder_mache();

    while (alive_children_counter > 0);

    free(children);
    free(permission_granted);

    exit(EXIT_SUCCESS);
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
        ++alive_children_counter;
        return pid;
    }
}

void kinder_mache() {
    for (int i = 0; i < N; ++i) {
        children[i] = make_child();
#ifdef D_SPAWN
        printf("%d (%3d): Spawned\n", children[i], i);
#endif // D_SPAWN
    }
}

void define_signal_handling() {
    sigset_t to_mask;
    sigfillset(&to_mask);

    struct sigaction *sg = calloc(1, sizeof(struct sigaction));
    sg->sa_flags = SA_SIGINFO;
    sg->sa_sigaction = &request_handler;
    sg->sa_mask = to_mask;
    sigaction(SIGUSR1, sg, NULL); // handle signal from child

    sg->sa_sigaction = &sigchld_handler;
    sigaction(SIGCHLD, sg, NULL); // handle child termination

    sg->sa_sigaction = (void (*)(int, siginfo_t *, void *)) &sigint_handler;
    sigaction(SIGINT, sg, NULL); // handle costam kurde...

    sg->sa_sigaction = &rt_handler;
    for (int sig = SIGRTMIN; sig <= SIGRTMAX; ++sig) {
        sigaction(sig, sg, NULL);
    }
    free(sg);
}

void allow_all(void) {
    for (int i = 0; i < N; ++i) {
        if (permission_granted[i]) {
            send_allow(children[i]);
        }
    }
}

void send_allow(pid_t child) {
#ifdef D_ALLOWS
    printf("%d (%3d): Sending confirmation\n", child, get_child_id(child));
#endif
    kill(child, SIGALRM);
}

void request_handler(int signal, siginfo_t *info, void *ucontext) {
    assert(signal == SIGUSR1);
    (void) ucontext;

    pid_t caller = info->si_pid;
    int childnum = get_child_id(caller);

#ifdef D_REQUEST
    printf("%d (%3d): Requested permission\n", caller, childnum);
#endif

    if (!permission_granted[childnum]) {
        ++request_counter;
        permission_granted[childnum] = true;
    }

    if (request_counter == K) {
        allow_all();
    } else if (request_counter > K) {
        send_allow(caller);
    }
}


void sigchld_handler(int signal, siginfo_t *info, void *ucontext) {
    assert(signal == SIGCHLD);
    (void) ucontext; // unused

    int left = --alive_children_counter;
    int status = info->si_status;
    pid_t child = info->si_pid;
    int i = get_child_id(child);

    children[i] = 0;

    assert(status <= 10); // maximum selected time

#ifdef D_CHLD
    printf(
            "%d (%3d): Exited with status: %d\t%d children left\n",
            info->si_pid,
            i,
            status,
            left
    );
#endif //D_CHLD

    // collect zombie
    waitpid(info->si_pid, NULL, 0);
}

void rt_handler(int signal, siginfo_t *info, void *ucontext) {
    (void) ucontext; // unused
    assert(signal >= SIGRTMIN && signal <= SIGRTMAX);
#ifdef D_SIGRT
    printf("%d (%3d): Received SIGRT%d\n", info->si_pid, get_child_id(info->si_pid), signal - SIGRTMIN);
#endif // D_SIGRT
}

void sigint_handler(void) {
    for (int i = 0; i < N; ++i) {
        if (children[i] != 0) {
            kill(children[i], SIGKILL);
#ifdef D_CHLD
            printf(
                    "%d (%3d): Killed on parent exit\n",
                    children[i],
                    i
            );
#endif // D_CHLD
        }
    }
    exit(EXIT_SUCCESS);
}

int get_child_id(pid_t pid) {
    int id = 0;
    while (id < N && children[id] != pid) ++id;
    if (id < N) {
        return id;
    } else {
        return -1;
    }
}

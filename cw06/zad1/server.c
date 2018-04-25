#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "rainbow.h"
#include "protocol.h"

// - |MACROS| ---------------------------- //
#define PERR_EXIT(_MSG_) {              \
perror( RED _MSG_ RESET );              \
exit(EXIT_FAILURE);                     \
}

#define ERR_EXIT(_MSG_, ...) {                          \
fprintf(stderr, RED _MSG_ RESET "\n", ##__VA_ARGS__);   \
exit(EXIT_FAILURE);                                     \
}

#define WARNING(_MSG_, ...) { fprintf(stdout, YEL _MSG_ RESET "\n", ##__VA_ARGS__ ); }
#define INFO(_MSG_, ...) { fprintf(stdout, CYN _MSG_ RESET "\n", ##__VA_ARGS__ ); }


// - |GLOBAL VARIABLES| ------------------ //
int server_queue_id = -1;
int client_queues[MAX_CLIENTS];
int client_pids[MAX_CLIENTS];
int client_count = 0;


// - |FUNCTION DECLARATIONS| ------------- //
void setup();

void cleanup();

void sigint_handler(int signal);

void receive_loop();


void h_mirror(Msg *msg);

void h_calc(Msg *msg);

void h_time(Msg *msg);

void h_end(Msg *msg);

void h_stop(Msg *msg);

void h_signup(Msg *msg);


char *fetch_date();

int calc(char op[4], int arg1, int arg2);


// - |MAIN| ------------------------------ //
int main() {
    receive_loop();

    exit(EXIT_SUCCESS);
}


// - |UTILITIES| ------------------------- //
void setup() {
    if (atexit(cleanup) == -1) PERR_EXIT("Error registering exit handler");

    char *pwd = getenv("HOME");
    if (pwd == NULL) PERR_EXIT("Error getting HOME env variable");

    key_t public_key = ftok(pwd, PROJECT_ID);
    if (public_key == -1) PERR_EXIT("Public key generation failed");

    server_queue_id = msgget(public_key, IPC_CREAT | IPC_EXCL | 0666);
    if (server_queue_id == -1) PERR_EXIT("Public queue generation failed");

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_pids[i] = -1;
        client_queues[i] = -1;
    }
}

void cleanup() {
    if (msgctl(server_queue_id, IPC_RMID, NULL) < 0) {
        PERR_EXIT("Queue deletion failed");
    }
}

void sigint_handler(int signal) {
    assert(signal == SIGINT);
    exit(2);
}

void receive_loop() {
    Msg msg;
    ssize_t read;

    while ((read = msgrcv(server_queue_id, &msg, MSG_SIZE, 0, 0)) != -1) {
        int client_id = msg.id;
        if (msg.msg_type == SIGNUP) {
            h_signup(&msg);
        } else {
            assert(client_id < MAX_CLIENTS);
            assert(client_pids[client_id] == msg.pid);
            switch (msg.msg_type) {
                case MIRROR:
                    h_mirror(&msg);
                    break;
                case CALC:
                    h_calc(&msg);
                    break;
                case TIME:
                    h_time(&msg);
                    break;
                case STOP:
                    h_stop(&msg);
                    break;
                case END:
                    h_end(&msg);
                    return;
                default: WARNING(
                        "Unknown message type: %zu from client: (id, pid) = (%d, %d)",
                        msg.msg_type,
                        client_id,
                        msg.pid
                );
            }
        }
    }
}


// - |REQUEST HANDLERS| ------------------ //
void h_mirror(Msg *msg) {
    int len = strlen(msg->contents);

    Msg resp;
    resp.msg_type = MIRROR;
    resp.id = SERVER_ID;
    resp.pid = getpid();

    for (int i = 0; i < len; ++i) {
        resp.contents[i] = msg->contents[len - 1 - i];
    }

    if (msgsnd(client_queues[msg->id], &resp, MSG_SIZE, 0) < 0) {
        PERR_EXIT("Error responding to MIRROR request");
    }
}

void h_calc(Msg *msg) {
    char op[4];
    int arg1 = 0;
    int arg2 = 0;
    sscanf(msg->contents, "%3s %d %d", op, &arg1, &arg2);
    int result = calc(op, arg1, arg2);

    Msg resp;
    resp.msg_type = CALC;
    resp.id = SERVER_ID;
    resp.pid = getpid();

    sprintf(resp.contents, "%d", result);

    if (msgsnd(client_queues[msg->id], &resp, MSG_SIZE, 0) < 0) {
        PERR_EXIT("Error responding to CALC request");
    }
}

void h_time(Msg *msg) {
    Msg resp;
    resp.msg_type = CALC;
    resp.id = SERVER_ID;
    resp.pid = getpid();

    char *date = fetch_date();
    strcpy(resp.contents, date);
    free(date);

    if (msgsnd(client_queues[msg->id], &resp, MSG_SIZE, 0) < 0) {
        PERR_EXIT("Error responding to TIME request");
    }
}

void h_end(Msg *msg) {
    INFO("END message received from client: (id, pid) = (%d, %d)", msg->id, client_pids[msg->id]);
}

void h_stop(Msg *msg) {
//    msgctl(client_queues[msg->id], IPC_RMID, NULL);
    client_queues[msg->id] = -1;
    client_pids[msg->id] = -1;
    // anything else?!
}

void h_signup(Msg *msg) {
    assert(msg->id = -1);

    key_t client_key;
    if (sscanf(msg->contents, "%d", &client_key) < 0) {
        ERR_EXIT("Error reading client key: %s", msg->contents);
    }

    int client_queue_id = msgget(client_key, 0);
    if (client_queue_id == -1) {
        ERR_EXIT("Error opening client's queue");
    }

    if (client_count >= MAX_CLIENTS) {
        WARNING("Max client count reached")
    } else {
        client_pids[client_count] = msg->pid;
        client_queues[client_count] = client_queue_id;

        Msg resp;
        resp.msg_type = SIGNUP;
        resp.id = SERVER_ID;
        resp.pid = getpid();

        sprintf(resp.contents, "%d", client_count);

        if (msgsnd(client_queues[client_count], &resp, MSG_SIZE, 0) < 0) {
            PERR_EXIT("Error sending client's id");
        }
        ++client_count;
    }
}


// - |AUXILIARY FUNCTIONS| --------------- //
int calc(char op[4], int arg1, int arg2) {
    if (strcmp(op, "ADD") == 0) {
        return arg1 + arg2;
    } else if (strcmp(op, "SUB") == 0) {
        return 1 - arg2;
    } else if (strcmp(op, "MUL") == 0) {
        return 1 * arg2;
    } else if (strcmp(op, "DIV") == 0) {
        if (arg2 == 0) return 0;
        return 1 / arg2;
    } else {
        return 0;
    }
}

char *fetch_date() {
    FILE *pipe = popen("date", "r");

    char *line = NULL;
    size_t len = 0;
    if (getline(&line, &len, pipe) == -1) {
        PERR_EXIT("Error fetching date");
    }

    pclose(pipe);
    return line;
}
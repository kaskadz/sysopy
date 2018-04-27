#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 200809L

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
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>

#include "rainbow.h"
#include "protocol.h"

// - |MACROS| ---------------------------- //
#define PERR_EXIT(_MSG_) {              \
perror( "[" MAG "Server" RESET "][" RED "PERROR" RESET "]" _MSG_ );              \
exit(EXIT_FAILURE);                     \
}

#define ERR_EXIT(_MSG_, ...) {                                          \
fprintf(stderr, "[" MAG "Server" RESET "][" RED "ERROR" RESET "] " _MSG_ "\n", ##__VA_ARGS__);  \
exit(EXIT_FAILURE);                                                     \
}

#define WARNING(_MSG_, ...) { fprintf(stdout, "[" MAG "Server" RESET "][" YEL "WARNING" RESET "] " _MSG_ "\n", ##__VA_ARGS__ ); }
#define INFO(_MSG_, ...) { fprintf(stdout, "[" MAG "Server" RESET "][" CYN "INFO" RESET "] " _MSG_ "\n", ##__VA_ARGS__ ); }


// - |GLOBAL VARIABLES| ------------------ //
mqd_t server_queue_id = -1;
mqd_t client_queues[MAX_CLIENTS];
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

void set_nonblock();


// - |MAIN| ------------------------------ //
int main() {
    setup();

    receive_loop();

    exit(EXIT_SUCCESS);
}


// - |UTILITIES| ------------------------- //
void setup() {
    INFO("Setting up..");
    if (atexit(cleanup) == -1) PERR_EXIT("Error registering exit handler");
    if (signal(SIGINT, &sigint_handler) == SIG_ERR) PERR_EXIT("Error registering SIGINT handler");

    struct mq_attr attr = {
            .mq_flags = 0,
            .mq_maxmsg = 10,
            .mq_curmsgs = 0,
            .mq_msgsize = MSG_SIZE
    };

    server_queue_id = mq_open(SERVER_QUEUE, O_RDONLY | O_CREAT | O_EXCL, 0644, &attr);
    if (server_queue_id < 0) PERR_EXIT("Public queue generation failed");

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_pids[i] = -1;
        client_queues[i] = -1;
    }
}

void cleanup() {
    INFO("Cleaning up..");
    INFO("Removing queue %d\n", server_queue_id);
    mq_unlink(SERVER_QUEUE);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (client_queues[i] > 0) {
            INFO("Closing queue with descriptor %d\n", client_queues[i]);
            mq_close(client_queues[i]);
        }
    }
}

void sigint_handler(int signal) {
    INFO("Received sigint :P");
    assert(signal == SIGINT);
    exit(2);
}

void receive_loop() {
    INFO("Entering receiving loop..");
    Msg msg;
    ssize_t read;
    unsigned priority;

    while ((read = mq_receive(server_queue_id, (char *) &msg, MSG_SIZE, &priority)) != -1) {
        assert(priority == PRIORITY);
        assert(read > 0);
        assert(read <= (ssize_t) MSG_SIZE);

        INFO("Received a message!");
        int client_id = msg.id;
        if (msg.msg_type == SIGNUP) {
            h_signup(&msg);
        } else {
            if (client_id >= MAX_CLIENTS) ERR_EXIT("Exceeded number of registered clients");
            if (client_pids[client_id] != msg.pid) {
                WARNING("Received message from unregistered client");
            }
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
                    set_nonblock();
                    break;
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
    INFO("Handling MIRROR message");
    int len = strlen(msg->contents);

    Msg resp;
    resp.msg_type = MIRROR;
    resp.id = SERVER_ID;
    resp.pid = getpid();

    for (int i = 0; i < len; ++i) {
        resp.contents[i] = msg->contents[len - 1 - i];
    }
    resp.contents[len] = '\0';

    if (mq_send(client_queues[msg->id], (char *) &resp, MSG_SIZE, PRIORITY) < 0) {
        PERR_EXIT("Error responding to MIRROR request");
    }

    memset(msg->contents, 0, CONTENTS_SIZE);
}

void h_calc(Msg *msg) {
    INFO("Handling CALC message");
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

    if (mq_send(client_queues[msg->id], (char *) &resp, MSG_SIZE, PRIORITY) < 0) {
        PERR_EXIT("Error responding to CALC request");
    }
}

void h_time(Msg *msg) {
    INFO("Handling TIME message");
    Msg resp;
    resp.msg_type = CALC;
    resp.id = SERVER_ID;
    resp.pid = getpid();

    char *date = fetch_date();
    strcpy(resp.contents, date);
    free(date);

    if (mq_send(client_queues[msg->id], (char *) &resp, MSG_SIZE, PRIORITY) < 0) {
        PERR_EXIT("Error responding to TIME request");
    }
}

void h_end(Msg *msg) {
    INFO("Handling END message");
    INFO("END message received from client: (id, pid) = (%d, %d)", msg->id, client_pids[msg->id]);
}

void h_stop(Msg *msg) {
    INFO("Handling STOP message");
    INFO("Unregistering client: (id, pid) = (%d, %d)", msg->id, msg->pid);
    mq_close(client_queues[msg->id]);
    client_queues[msg->id] = -1;
    client_pids[msg->id] = -1;
}

void h_signup(Msg *msg) {
    INFO("Handling SIGNUP message");
    assert(msg->id == -1);

    if (client_count >= MAX_CLIENTS) {
        WARNING("Max client count reached. Client not registered!");
    } else {
        client_pids[client_count] = msg->pid;
        client_queues[client_count] = mq_open(msg->contents, O_WRONLY);;

        INFO("Registered new client: (pid, id) = (%d, %d)", msg->pid, client_count);

        Msg resp;
        resp.msg_type = SIGNUP;
        resp.id = SERVER_ID;
        resp.pid = getpid();

        sprintf(resp.contents, "%d", client_count);

        if (mq_send(client_queues[client_count], (char *) &resp, MSG_SIZE, PRIORITY) < 0) {
            PERR_EXIT("Error sending client's id");
        }
        INFO("Register response sent to client");
        ++client_count;
    }
}


// - |AUXILIARY FUNCTIONS| --------------- //
int calc(char op[4], int arg1, int arg2) {
    if (strcmp(op, "ADD") == 0) {
        return arg1 + arg2;
    } else if (strcmp(op, "SUB") == 0) {
        return arg1 - arg2;
    } else if (strcmp(op, "MUL") == 0) {
        return arg1 * arg2;
    } else if (strcmp(op, "DIV") == 0) {
        if (arg2 == 0) return 0;
        return arg1 / arg2;
    } else {
        return 0;
    }
}

char *fetch_date() {
//    INFO("Getting date...");
    FILE *pipe = popen("date", "r");

    char *line = NULL;
    size_t len = 0;
    if (getline(&line, &len, pipe) == -1) {
        PERR_EXIT("Error fetching date");
    }

    pclose(pipe);
    return line;
}

void set_nonblock(void) {
    struct mq_attr *attr = calloc(1, sizeof(struct mq_attr));
    mq_getattr(server_queue_id, attr);
    attr->mq_flags |= O_NONBLOCK;
    mq_setattr(server_queue_id, attr, NULL);
    free(attr);
}

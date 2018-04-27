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
#include <time.h>

#include "rainbow.h"
#include "protocol.h"


// - |MACROS| ---------------------------- //
#define PROMPT_CHAR '>'

#define PERR_EXIT(_MSG_) {              \
perror( "[" GRN "Client" RESET "]" RED _MSG_ RESET );              \
exit(EXIT_FAILURE);                     \
}
#define ERR_EXIT(_MSG_, ...) {                          \
fprintf(stderr, "[" GRN "Client" RESET "][" RED "ERROR" RESET "] " _MSG_ "\n", ##__VA_ARGS__);   \
exit(EXIT_FAILURE);                                     \
}
#define WARNING(_MSG_, ...) { fprintf(stdout, "[" GRN "Client" RESET "][" YEL "WARNING" RESET "] " _MSG_ "\n", ##__VA_ARGS__ ); }
#define INFO(_MSG_, ...) { fprintf(stdout, "[" GRN "Client" RESET "][" CYN "INFO" RESET "] " _MSG_ "\n", ##__VA_ARGS__ ); }


// - |GLOBAL VARIABLES| ------------------ //
char *client_queue_name = NULL;
mqd_t client_queue_id = -1;
mqd_t server_queue_id = -1;
int client_id = -1;
int display_prompt = 0;

// - |FUNCTION DECLARATIONS| ------------- //
void setup();

void cleanup();

void sigint_handler(int signal);

void register_client();

void receive_loop();


void send_mirror(char *str);

void send_calc(char *str);

void send_time();

void send_end();

void send_stop();


char **make_args(char *line, size_t *param_qtty, char *delim);

char *get_command_params(char *str);

char *generate_queue_name();


// - |MAIN| ------------------------------ //
int main(int argc, char *argv[]) {
    FILE *file;
    if (argc == 1) {
        file = stdin;
        INFO("Using stdin as input");
        display_prompt = 1;
    } else if (argc <= 2) {
        file = fopen(argv[1], "r");
        INFO("Using %s as input", argv[1]);
        if (file == NULL) {
            PERR_EXIT("Error opening a file");
        }
    }

    setup();

    register_client();

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    if (display_prompt) printf("%c ", PROMPT_CHAR);
    while ((read = getline(&line, &len, file)) != -1) {
        // copy line buffer contents
        char *line_copy = calloc(read + 1, sizeof(char));
        strcpy(line_copy, line);

        size_t elem_qtty;
        char **elems = make_args(line, &elem_qtty, " \n\r");
        if (elem_qtty < 1) continue;
        if (strcmp(elems[0], "MIRROR") == 0) {
            if (elem_qtty > 1) {
                char *content = get_command_params(line_copy);
                if (content == NULL) {
                    WARNING("MIRROR received wrong argumets!");
                } else {
                    INFO("Mirroring %s", content);
                    send_mirror(content);
                    free(content);
                }
            } else {
                WARNING("MIRROR received no arguments! Message was not sent!");
            }
        } else if (strcmp(elems[0], "CALC") == 0) {
            if (elem_qtty == 4) {
                char *content = get_command_params(line_copy);
                if (content == NULL) {
                    WARNING("CALC received wrong argumets!");
                } else {
                    send_calc(content);
                    free(content);
                }
            } else {
                WARNING("CALC received wrong number of arguments (%zu)! Message was not sent!", elem_qtty);
            }
        } else if (strcmp(elems[0], "TIME") == 0) {
            send_time();
        } else if (strcmp(elems[0], "STOP") == 0) {
            break;
        } else if (strcmp(elems[0], "END") == 0) {
            send_end();
            break;
        } else {
            WARNING("Unknown command: '%s'", elems[0]);
        }
        free(line_copy);
        free(elems);
        if (display_prompt) printf("%c ", PROMPT_CHAR);
    }

    free(line);

    exit(EXIT_SUCCESS);
}

// - |UTILITIES| ------------------------- //
void setup() {
    INFO("Setting up..");
    srand(time(NULL)*getpid());
    if (atexit(cleanup) == -1) PERR_EXIT("Error registering exit handler");
    if (signal(SIGINT, &sigint_handler) == SIG_ERR) PERR_EXIT("Error registering SIGINT handler");

    // create private queue
    client_queue_name = generate_queue_name();
    struct mq_attr attr = {
            .mq_flags = 0,
            .mq_maxmsg = 10,
            .mq_curmsgs = 0,
            .mq_msgsize = MSG_SIZE
    };
    client_queue_id = mq_open(client_queue_name, O_RDONLY | O_CREAT | O_EXCL, 0644, &attr);
    if (client_queue_id < 0) PERR_EXIT("Queue opening failed");

    // open server queue
    server_queue_id = mq_open(SERVER_QUEUE, O_WRONLY);
    if (server_queue_id < 0) PERR_EXIT("Error getting server queue");
}

void cleanup() {
    INFO("Cleaning up..");
    send_stop();
    mq_close(client_queue_id);
    mq_unlink(client_queue_name);
}

void sigint_handler(int signal) {
    INFO("Received sigint :P");
    assert(signal == SIGINT);
    exit(EXIT_SUCCESS);
}

void register_client() {
    INFO("Registering client...")
    Msg msg = {
            .id = client_id,
            .pid = getpid(),
            .msg_type = SIGNUP,
    };
    sprintf(msg.contents, "%s", client_queue_name);

    if (mq_send(server_queue_id, (char *) &msg, MSG_SIZE, PRIORITY) < 0) {
        PERR_EXIT("Error sending SIGNUP message");
    }

    Msg resp;
    ssize_t read;
    unsigned priority;
    if ((read = mq_receive(client_queue_id, (char *) &resp, MSG_SIZE, &priority)) == -1) {
        PERR_EXIT("Error receiving SIGNUP response");
    }
    assert(PRIORITY == priority);
    assert(resp.id == SERVER_ID);
    sscanf(resp.contents, "%d", &client_id);
    assert(client_id != -1);
    INFO("Client registered!");
}

// - |COMMAND HANDLERS| ------------------ //
void send_mirror(char *str) {
    INFO("Sending MIRROR request...");
    int len = strlen(str);
    Msg msg = {
            .id = client_id,
            .pid = getpid(),
            .msg_type = MIRROR,
    };
    --len;
    if (str[len] == '\n' || str[len] == '\r') --len;
    memcpy(msg.contents, str, (len > CONTENTS_SIZE) ? CONTENTS_SIZE : len);

    if (mq_send(server_queue_id, (char *) &msg, MSG_SIZE, PRIORITY) < 0) {
        PERR_EXIT("Error sending MIRROR request");
    }

    Msg resp;
    ssize_t read;
    unsigned priority;
    if ((read = mq_receive(client_queue_id, (char *) &resp, MSG_SIZE, &priority)) == -1) {
        PERR_EXIT("Error receiving MIRROR response");
    }
    INFO("Received response for MIRROR request!");
    assert(resp.id == SERVER_ID);

    printf("Mirrored message: %s\n", resp.contents);
}

void send_calc(char *str) {
    INFO("Sending CALC request...");
    Msg msg = {
            .msg_type = CALC,
            .id = client_id,
            .pid = getpid(),
    };

    strcpy(msg.contents, str);

    if (mq_send(server_queue_id, (char *) &msg, MSG_SIZE, PRIORITY) < 0) {
        PERR_EXIT("Error sending CALC request");
    }

    Msg resp;
    ssize_t read;
    unsigned priority;
    if ((read = mq_receive(client_queue_id, (char *) &resp, MSG_SIZE, &priority)) == -1) {
        PERR_EXIT("Error receiving CALC response");
    }
    INFO("Received response for CALC request!");
    assert(resp.id == SERVER_ID);

    printf("Computed expression: %s\n", resp.contents);
}

void send_time() {
    INFO("Sending TIME request...");
    Msg msg = {
            .msg_type = TIME,
            .id = client_id,
            .pid = getpid(),
    };

    if (mq_send(server_queue_id, (char *) &msg, MSG_SIZE, PRIORITY) < 0) {
        PERR_EXIT("Error sending TIME request");
    }

    Msg resp;
    ssize_t read;
    unsigned priority;
    if ((read = mq_receive(client_queue_id, (char *) &resp, MSG_SIZE, &priority)) == -1) {
        PERR_EXIT("Error receiving TIME response");
    }
    INFO("Received response for TIME request");
    assert(resp.id == SERVER_ID);

    printf("Received date: %s\n", resp.contents);
}

void send_end() {
    INFO("Sending END request...");
    Msg msg = {
            .msg_type = END,
            .id = client_id,
            .pid = getpid(),
    };

    if (mq_send(server_queue_id, (char *) &msg, MSG_SIZE, PRIORITY) < 0) {
        PERR_EXIT("Error sending END request");
    }
}

void send_stop() {
    INFO("Sending STOP request");
    Msg msg = {
            .msg_type = STOP,
            .id = client_id,
            .pid = getpid(),
    };

    if (mq_send(server_queue_id, (char *) &msg, MSG_SIZE, PRIORITY) < 0) {
        WARNING("Error sending STOP request. Probably server is down.");
    }
}

// - |AUXILIARY FUNCTIONS| --------------- //
char **make_args(char *line, size_t *param_qtty, char *delim) {
    char **args = NULL;
    *param_qtty = 0;
    char *token = strtok(line, delim);
    while (token) {
        args = realloc(args, sizeof(char *) * ++(*param_qtty));
        if (args == 0) {
            perror("Memory reallocation error");
            free(args);
            return NULL;
        } else {
            args[(*param_qtty) - 1] = token;
        }
        token = strtok(NULL, delim);
    }
    return args;
}

char *get_command_params(char *str) {
    int len = strlen(str);
    int i = 0;
    while (i <= len && str[i] == ' ') ++i;  // find first word (command)
    while (i <= len && str[i] != ' ') ++i;  // skip first word (command)
    while (i <= len && str[i] == ' ') ++i;  // skip spaces after command
    int new_len = len - i + 1;
    if (new_len <= 1) {
        return NULL;
    }
    char *result = calloc(new_len, sizeof(char));
    strcpy(result, &str[i]);
    return result;
}

char *generate_queue_name(void) {
    const int LEN = 6;
    char *name = calloc(LEN + 2, sizeof(char));
    name[0] = '/';
    for (int i = 1; i < LEN + 1; ++i) {
        name[i] = (char) (rand() % ('z' - 'a' + 1) + 'a');
    }
    return name;
}
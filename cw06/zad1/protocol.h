#ifndef CW06_PROTOCOL_H
#define CW06_PROTOCOL_H

#define MAX_CLIENTS  64
#define PROJECT_ID 0x7844
#define CONTENTS_SIZE 4096
#define SERVER_ID 2137

#include <sys/types.h>

typedef enum Msg_type {
    MIRROR = 1,
    CALC = 2,
    TIME = 3,
    END = 4,
    STOP = 5,
    SIGNUP = 6
} msg_type;

typedef struct Msg {
    long msg_type;
    int id;
    pid_t pid;
    char contents[CONTENTS_SIZE];
} Msg;

const size_t MSG_SIZE = sizeof(Msg) - sizeof(long);

#endif //CW06_PROTOCOL_H


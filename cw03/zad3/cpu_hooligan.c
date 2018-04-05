#include <stdio.h>
#include <limits.h>

int main() {
    short a = 0;
    short dir = 1;
    printf("Doing cpu hooliganism...\n");
    while (1) {
        if (a == SHRT_MIN || a == SHRT_MAX) {
            dir *= -1;
        }
        a += dir;
    }
}

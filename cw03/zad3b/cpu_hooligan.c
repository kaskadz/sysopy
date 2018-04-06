#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main() {
    double x = 3.1415926;
    double y = -10101;
    int i, j;
    printf("Starting calculation...\n");
    for (i = 0; i < 100000; ++i) {
        for (j = 0; j < 100000; ++j) {
            x = sin(y);
            y = sin(x);
        }
    }
    printf("After plenty of meaningful calculations the result proudly carries value of: %lf.\n", x);
    return 0;
}
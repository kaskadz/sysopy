#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <dlfcn.h>
#include <sys/resource.h>
#include "measure.h"
#include "iocomp.h"

int fail_on_parameter();

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (argc <= 1) {
        return fail_on_parameter();
    }

    size_t r, b;
    const char *command = argv[1];
    if (strcmp(command, "generate") == 0) {
        if (argc < 5) return fail_on_parameter();

        const char *file = argv[2];
        r = strtoul(argv[3], NULL, 10);
        b = strtoul(argv[4], NULL, 10);

        printf("%s: %s %zu %zu\n", command, file, r, b);

        return measure_generate(file, r, b);
    } else if (strcmp(command, "sort") == 0) {
        if (argc < 5) return fail_on_parameter();

        const char *file = argv[2];
        r = strtoul(argv[3], NULL, 10);
        b = strtoul(argv[4], NULL, 10);

        printf("%s: %s %zu %zu\n", command, file, r, b);

        return measure_sort(file, r, b);
    } else if (strcmp(command, "copy") == 0) {
        if (argc < 6) return fail_on_parameter();

        const char *file1 = argv[2];
        const char *file2 = argv[3];
        r = strtoul(argv[4], NULL, 10);
        b = strtoul(argv[5], NULL, 10);

        printf("%s: %s %s %zu %zu\n", command, file1, file2, r, b);

        return measure_copy(file1, file2, r, b);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return EXIT_FAILURE;
    }
}

int fail_on_parameter() {
    fprintf(stderr, "Wrong parameters!\n");
    return EXIT_FAILURE;
}


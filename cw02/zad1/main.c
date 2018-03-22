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

#define ALLOC_CHECK(ptr)                                                      \
    if ((ptr) == NULL) {                                                      \
        printf("Memory allocation error!");                                 \
        exit(EXIT_FAILURE);                                                   \
    }

int fail_on_parameter();

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (argc <= 1) {
        exit(fail_on_parameter());
    }

    size_t r, b;
    const char *command = argv[1];
    if (strcmp(command, "generate") == 0) {
        if (argc < 5) return fail_on_parameter();

        const char *file = argv[2];
        r = strtoul(argv[3], NULL, 10);
        b = strtoul(argv[4], NULL, 10);

        printf("generating: %s of dimensions: %zu x %zu\n", file, r, b);

        exit(measure_generate(file, r, b));
    } else if (strcmp(command, "sort") == 0) {
        if (argc < 5) return fail_on_parameter();

        const char *file = argv[2];
        r = strtoul(argv[3], NULL, 10);
        b = strtoul(argv[4], NULL, 10);

        printf("sorting: %s of dimensions: %zu x %zu\n", file, r, b);

        exit(measure_sort(file, r, b));
    } else if (strcmp(command, "show") == 0) {
        if (argc < 5) return fail_on_parameter();

        const char *file = argv[2];
        r = strtoul(argv[3], NULL, 10);
        b = strtoul(argv[4], NULL, 10);

        printf("showing: %s of dimensions: %zu x %zu\n", file, r, b);

        exit(show(file, r, b));
    } else if (strcmp(command, "copy") == 0) {
        if (argc < 6) return fail_on_parameter();

        const char *file1 = argv[2];
        const char *file2 = argv[3];
        r = strtoul(argv[4], NULL, 10);
        b = strtoul(argv[5], NULL, 10);

        printf("copying: %s to %s dimension: %zu x %zu\n", file1, file2, r, b);

        exit(measure_copy(file1, file2, r, b));
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        exit(EXIT_FAILURE);
    }
}

int fail_on_parameter() {
    fprintf(stderr, "Wrong parameters!\n");
    return EXIT_FAILURE;
}


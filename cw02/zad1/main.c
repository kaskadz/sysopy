#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <dlfcn.h>
#include <sys/resource.h>

typedef struct timeval timeval;
typedef struct timespec timespec;

typedef struct SURTime {
    timespec sys;
    timespec usr;
    timespec rea;
} SURTime;

timespec tvconvert(timeval);

SURTime current_SURTime();

void show_SURTime(SURTime, SURTime, uint);

void show_timediff(timespec, timespec, const char *, uint);

int fail_on_parameter();

int generate_sys(const char *, size_t, size_t);

int generate_lib(const char *, size_t, size_t);

int sort_sys(const char *, size_t, size_t);

int sort_lib(const char *, size_t, size_t);

int copy_sys(const char *, const char *, size_t, size_t);

int copy_lib(const char *, const char *, size_t, size_t);

int test_generate(const char *, size_t, size_t);

int test_sort(const char *, size_t, size_t);

int test_copy(const char *, const char *, size_t, size_t);

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
    } else if (strcmp(command, "sort") == 0) {
        if (argc < 5) return fail_on_parameter();

        const char *file = argv[2];
        r = strtoul(argv[3], NULL, 10);
        b = strtoul(argv[4], NULL, 10);

        printf("%s: %s %zu %zu\n", command, file, r, b);
    } else if (strcmp(command, "copy") == 0) {
        if (argc < 6) return fail_on_parameter();

        const char *file1 = argv[2];
        const char *file2 = argv[3];
        r = strtoul(argv[4], NULL, 10);
        b = strtoul(argv[5], NULL, 10);

        printf("%s: %s %s %zu %zu\n", command, file1, file2, r, b);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return EXIT_FAILURE;
    }

    printf("Hello, World!\n");
    return 0;
}

int fail_on_parameter() {
    fprintf(stderr, "Wrong parameters!\n");
    return EXIT_FAILURE;
}

SURTime current_SURTime() {
    SURTime ts;
    struct rusage ru;
    struct timespec real;

    clock_gettime(CLOCK_REALTIME, &real);
    ts.rea = real;
    getrusage(RUSAGE_SELF, &ru);
    ts.sys = tvconvert(ru.ru_stime);
    ts.usr = tvconvert(ru.ru_utime);
    return ts;
}

timespec tvconvert(timeval time) {
    timespec ts;
    ts.tv_sec = time.tv_sec;
    ts.tv_nsec = time.tv_usec * 1000;
    return ts;
}

void show_SURTime(SURTime start, SURTime end, uint times) {
    if (times > 1) {
        printf("Total times of %u actions:\n", times);
        show_timediff(start.usr, end.usr, "User time", 1);
        show_timediff(start.sys, end.sys, "System time", 1);
        show_timediff(start.rea, end.rea, "Real time", 1);
        printf("Average action time:\n");
    }
    show_timediff(start.usr, end.usr, "User time", times);
    show_timediff(start.sys, end.sys, "System time", times);
    show_timediff(start.rea, end.rea, "Real time", times);
}

void show_timediff(timespec start, timespec end, const char *message, uint times) {
    long double ns_diff = (end.tv_sec - start.tv_sec) * 1000000000.0 / times + (end.tv_nsec - start.tv_nsec) / times;
    printf("%11s: %8.5Lf s = %10.5Lf ms = %12.5Lf us = %13.5Lf ns\n", message,
           ns_diff / 1000000000, ns_diff / 1000000, ns_diff / 1000, ns_diff);
}
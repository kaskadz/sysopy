#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <dlfcn.h>
#include "../zad1/blockarray.h"

#define DLL_NAME "libblockarray.so"
#define MAX_CYCLES 10000

const char *stat = "STATIC";
const char *dyna = "DYNAMIC";

typedef struct timeval timeval;
typedef struct timespec timespec;
typedef unsigned int uint;
typedef unsigned uint;

typedef struct SURTime {
    timespec sys;
    timespec usr;
    timespec rea;
} SURTime;

typedef struct Funs {
    BlockArray *(*arr_create)(size_t, size_t, bool);

    void (*arr_delete)(BlockArray *);

    char *(*blk_create)(const BlockArray *, size_t);

    void (*blk_delete)(const BlockArray *, size_t);

    char *(*blk_get)(BlockArray *, size_t);

    void (*blk_put)(char *, char *, size_t);

    void (*blk_put_string)(char *, char *);

    void (*blk_put_random)(char *, size_t);

    size_t (*blk_find)(const BlockArray *, size_t);
} Funs;

timespec tvconvert(timeval);

SURTime current_SURTime();

BlockArray *arr_create_and_fill(size_t, size_t, bool, Funs);

void show_SURTime(SURTime, SURTime, uint);

void show_timediff(timespec, timespec, const char *, uint);

void measure_create(size_t, size_t, bool, size_t, Funs);

void measure_delete(const BlockArray *, size_t, Funs);

void measure_find(BlockArray *, size_t, Funs);

void measure_add(const BlockArray *, size_t, Funs);

void measure_add_and_delete(const BlockArray *, size_t, Funs);

void arr_introduce(size_t, size_t, bool);

const char *type(bool is_static);

Funs fun;
void *handle;

int main(int argc, char *argv[]) {
    srand((uint) time(NULL));
#ifndef DLL
    fun.arr_create = &arr_create;
    fun.arr_delete = &arr_delete;
    fun.blk_create = &blk_create;
    fun.blk_delete = &blk_delete;
    fun.blk_get = &blk_get;
    fun.blk_put = &blk_put;
    fun.blk_put_string = &blk_put_string;
    fun.blk_put_random = &blk_put_random;
    fun.blk_find = &blk_find;
#else
    handle = dlopen(DLL_NAME, RTLD_LAZY);
    if(!handle) {
        printf("Could not open library %s: %s\n", DLL_NAME, dlerror());
        return EXIT_FAILURE;
    }
    fun.arr_create = dlsym(handle, "arr_create");
    fun.arr_delete = dlsym(handle, "arr_delete");
    fun.blk_create = dlsym(handle, "blk_create");
    fun.blk_delete = dlsym(handle, "blk_delete");
    fun.blk_get = dlsym(handle, "blk_get");
    fun.blk_put = dlsym(handle, "blk_put");
    fun.blk_put_string = dlsym(handle, "blk_put_string");
    fun.blk_put_random = dlsym(handle, "blk_put_random");
    fun.blk_find = dlsym(handle, "blk_find");
#endif

    if (argc == 1) {
        printf("main BLOCK_QUANTITY BLOCK_SIZE ALLOCATION [OPERATION ARG] \n"
                       "List of operations: arr_create N, "
                       "blk_find I, blk_delete I, "
                       "blk_add I, remove_and_add N\n"
        );
        return EXIT_SUCCESS;
    }

    if (argc < 4) {
        printf("Too few parameters.\n");
        return EXIT_FAILURE;
    }

    if (argc % 2 == 1) {
        printf("Wrong parameters given.\n");
        return EXIT_FAILURE;
    }

    size_t block_qtty = strtoul(argv[1], NULL, 10);
    size_t block_size = strtoul(argv[2], NULL, 10);
    if (block_qtty <= 0 || block_size <= 0 || block_qtty >= 1000000 || block_size >= 1000) {
        printf("Wrong array dimensions.\n");
        return EXIT_FAILURE;
    }
    bool is_static;
    if (strcmp(argv[3], "static") == 0) {
        is_static = true;
    } else if (strcmp(argv[3], "dynamic") == 0) {
        is_static = false;
    } else {
        printf("Allocation mode should be either \'static\' or \'dynamic\'\n");
        return EXIT_FAILURE;
    }

    arr_introduce(block_qtty, block_size, is_static);

    for (int i = 4; i < argc; i += 2) {
        const char *command = argv[i];
        size_t n = strtoul(argv[i + 1], NULL, 10);
        if (strcmp(command, "arr_create") == 0) {
            if (n == 0) continue;
            measure_create(block_qtty, block_size, is_static, n, fun);
        } else if (strcmp(command, "blk_add") == 0) {
            if (n == 0) continue;
            measure_add((*fun.arr_create)(block_qtty, block_size, is_static), n, fun);
        } else if (strcmp(command, "blk_delete") == 0) {
            if (n == 0) continue;
            measure_delete(arr_create_and_fill(block_qtty, block_size, is_static, fun), n, fun);
        } else if (strcmp(command, "blk_add_and_delete") == 0) {
            if (n == 0) continue;
            measure_add_and_delete((*fun.arr_create)(block_qtty, block_size, is_static), n, fun);
        } else if (strcmp(command, "blk_find") == 0) {
            measure_find(arr_create_and_fill(block_qtty, block_size, is_static, fun), n, fun);
        } else {
            printf("Unknown operation: %d\n", i);
            return EXIT_FAILURE;
        }
        printf("\n");
    }

#ifdef DLL
    dlclose(handle);
#endif

    return EXIT_SUCCESS;
}

void arr_introduce(size_t qtty, size_t size, bool is_static) {
    printf("===============================================\n"
                   " Array type is: %s\n"
                   " It consists of >>%zu<< blocks and every block\n"
                   "    consists of >>%zu<< chars.\n"
                   "===============================================\n",
           type(is_static), qtty, size);
}

const char *type(bool is_static) {
    return (is_static) ? stat : dyna;
}

BlockArray *arr_create_and_fill(size_t block_qtty, size_t block_size, bool is_static, Funs fun) {
    BlockArray *arr = (*fun.arr_create)(block_qtty, block_size, is_static);
    for (size_t i = 0; i < block_qtty; ++i) {
        (*fun.blk_create)(arr, i);
        (*fun.blk_put_random)((*fun.blk_get)(arr, i), arr->block_size);
    }
    return arr;
}

void measure_create(size_t block_qtty, size_t block_size, bool is_static, size_t times, Funs fun) {
    if (times > MAX_CYCLES) times = MAX_CYCLES;

    printf("Starting measurement of %s array creation time. (%zu times)\n",
           type(is_static), times);

    BlockArray **arrays = calloc(times, sizeof(BlockArray *));

    SURTime begin = current_SURTime();
    for (uint c = 0; c < times; ++c) {
        arrays[c] = (*fun.arr_create)(block_qtty, block_size, is_static);
        for (size_t i = 0; i < block_qtty; ++i)
            (*fun.blk_create)(arrays[c], i);
    }
    SURTime end = current_SURTime();

    for (uint c = 0; c < times; ++c)
        (*fun.arr_delete)(arrays[c]);

    show_SURTime(begin, end, times);
}

void measure_add(const BlockArray *barr, size_t block_qtty, Funs fun) {
    assert(block_qtty <= barr->block_qtty);

    printf("Starting time measurement of adding %zu blocks to %s array.\n",
           block_qtty, type(barr->is_static));

    SURTime begin = current_SURTime();
    for (size_t i = 0; i < block_qtty; ++i)
        (*fun.blk_create)(barr, i);
    SURTime end = current_SURTime();

    show_SURTime(begin, end, block_qtty);
}

void measure_delete(const BlockArray *barr, size_t block_qtty, Funs fun) {
    assert(block_qtty <= barr->block_qtty);

    printf("Starting time measurement of deleting %zu blocks from %s array.\n",
           block_qtty, type(barr->is_static));

    SURTime begin = current_SURTime();
    for (size_t i = 0; i < block_qtty; ++i)
        (*fun.blk_delete)(barr, i);
    SURTime end = current_SURTime();

    show_SURTime(begin, end, block_qtty);
}

void measure_add_and_delete(const BlockArray *barr, size_t times, Funs fun) {
    printf("Starting time measurement of adding and removing blocks to/from %s array. (%zu times)\n",
           type(barr->is_static), times);

    SURTime begin = current_SURTime();
    for (uint i = 0; i < times; ++i) {
        (*fun.blk_create)(barr, 0);
        (*fun.blk_delete)(barr, 0);
    }
    SURTime end = current_SURTime();

    show_SURTime(begin, end, times);
}

void measure_find(BlockArray *barr, size_t k, Funs fun) {
    size_t res = 0;

    printf("Starting time measurement of searching %s array. (%zu times)\n",
           type(barr->is_static), (size_t) MAX_CYCLES);

    SURTime begin = current_SURTime();
    for (size_t t = 0; t < MAX_CYCLES; ++t)
        res = (*fun.blk_find)(barr, k);
    SURTime end = current_SURTime();

    printf("Search result: Block %zu is closest to block %zu\n", res, k);

    show_SURTime(begin, end, MAX_CYCLES);
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <dlfcn.h>
#include <sys/resource.h>
#include "iocomp.h"

int generate_sys(const char *filename, size_t record_qtty, size_t record_size) {
    return EXIT_SUCCESS;
}

int generate_lib(const char *filename, size_t record_qtty, size_t record_size) {
    return EXIT_SUCCESS;
}

int sort_sys(const char *filename, size_t record_qtty, size_t record_size) {
    return EXIT_SUCCESS;
}

int sort_lib(const char *filename, size_t record_qtty, size_t record_size) {
    return EXIT_SUCCESS;
}

int copy_sys(const char *src_filename, const char *dest_filename, size_t record_qtty, size_t record_size) {
    return EXIT_SUCCESS;
}

int copy_lib(const char *src_filename, const char *dest_filename, size_t record_qtty, size_t record_size) {
    return EXIT_SUCCESS;
}

int measure_generate(const char *filename, size_t record_qtty, size_t record_size) {
    return EXIT_SUCCESS;
}

int measure_sort(const char *filename, size_t record_qtty, size_t record_size) {
    return EXIT_SUCCESS;
}

int measure_copy(const char *src_filename, const char *dest_filename, size_t reocrd_qtty, size_t record_size) {
    return EXIT_SUCCESS;
}

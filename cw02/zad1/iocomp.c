#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <dlfcn.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "iocomp.h"
#include "measure.h"

#define FREAD_RECORD(__FILE__, __SIZE__, __BUFFER__, __I__) {           \
if (fseek(__FILE__, get_offset(__SIZE__, __I__), 0) != 0) {             \
    fprintf(stderr, "fseek error: i = %d\n", __I__);                    \
    return 1;                                                           \
}                                                                       \
if (fread(__BUFFER__, sizeof(char), __SIZE__, __FILE__) != __SIZE__) {  \
    fprintf(stderr, "fread error\n");                                   \
    return 1;                                                           \
}                                                                       \
}

#define FWRITE_RECORD(__FILE__, __SIZE__, __BUFFER__, __I__) {          \
if (fseek(__FILE__, get_offset(__SIZE__, __I__), 0) != 0) {             \
    fprintf(stderr, "fseek error: i = %d\n", __I__);                    \
    return 1;                                                           \
}                                                                       \
if (fwrite(__BUFFER__, sizeof(char), __SIZE__, __FILE__) != __SIZE__) { \
    fprintf(stderr, "fwrite error\n");                                  \
    return 1;                                                           \
}                                                                       \
}

#define READ_RECORD(__FD__, __SIZE__, __BUFFER__, __I__) {      \
if (lseek(__FD__, get_offset(__SIZE__, __I__), SEEK_SET) < 0) { \
    fprintf(stderr, "lseek error: i = %d\n", __I__);            \
    return 1;                                                   \
}                                                               \
if (read(__FD__, __BUFFER__, __SIZE__) != __SIZE__) {           \
    fprintf(stderr, "read error\n");                            \
    return 1;                                                   \
}                                                               \
}

#define WRITE_RECORD(__FD__, __SIZE__, __BUFFER__, __I__) {     \
if (lseek(__FD__, get_offset(__SIZE__, __I__), SEEK_SET) < 0) { \
    fprintf(stderr, "lseek error: i = %d\n", __I__);            \
    return 1;                                                   \
}                                                               \
if (write(__FD__, __BUFFER__, __SIZE__) != __SIZE__) {          \
    fprintf(stderr, "write error\n");                           \
    return 1;                                                   \
}                                                               \
}

#define CHECK_FOPEN(__FILE__, __FILENAME__) {                   \
if (__FILE__ == NULL) {                                         \
    fprintf(stderr, "error fopening file: %s\n", __FILENAME__); \
    return 1;                                                   \
}                                                               \
}

#define CHECK_OPEN(__FILE__, __FILENAME__) {                    \
if (__FILE__ == -1) {                                           \
    fprintf(stderr, "error opening file: %s\n", __FILENAME__);  \
    return 1;                                                   \
}                                                               \
}

int generate(const char *filename, size_t record_qtty, size_t record_size) {
    copy_sys("/dev/urandom", filename, record_qtty, record_size);
    return 0;
}

off_t get_offset(size_t record_size, size_t i) {
    return (record_size * i);
}

int show_record(char *record, size_t record_size) {
    for (size_t j = 0; j < record_size; ++j) {
        printf("|%4d", record[j]);
    }
    printf("|\n");
}

int show(const char *filename, size_t record_qtty, size_t record_size) {
    int fd = open(filename, O_RDWR);
    char *tmp = malloc(record_size * sizeof(char));
    for (int i = 0; i < record_qtty; ++i) {
        READ_RECORD(fd, record_size, tmp, i);
        show_record(tmp, record_size);
    }
    close(fd);
}

int sort_sys(const char *filename, size_t record_qtty, size_t record_size) {
    int fd = open(filename, O_RDWR);
    CHECK_OPEN(fd, filename);
    char *tmp1 = malloc(record_size * sizeof(char));
    char *tmp2 = malloc(record_size * sizeof(char));

    for (int i = 1; i < record_qtty; ++i) {
        READ_RECORD(fd, record_size, tmp1, i);
        int j = i - 1;

        READ_RECORD(fd, record_size, tmp2, j);
        while (j >= 0 && tmp2[0] > tmp1[0]) {
            WRITE_RECORD(fd, record_size, tmp2, j + 1);

            j--;
            if (j >= 0) READ_RECORD(fd, record_size, tmp2, j);
        }
        WRITE_RECORD(fd, record_size, tmp1, j + 1);
    }
    close(fd);
    return 0;
}

int sort_lib(const char *filename, size_t record_qtty, size_t record_size) {
    FILE *file = fopen(filename, "r+");
    CHECK_FOPEN(file, filename);
    char *tmp1 = malloc(record_size * sizeof(char));
    char *tmp2 = malloc(record_size * sizeof(char));

    for (int i = 1; i < record_qtty; ++i) {
        FREAD_RECORD(file, record_size, tmp1, i);
        int j = i - 1;

        FREAD_RECORD(file, record_size, tmp2, j);
        while (j >= 0 && tmp2[0] > tmp1[0]) {
            FWRITE_RECORD(file, record_size, tmp2, j + 1);

            j--;
            if (j >= 0) FREAD_RECORD(file, record_size, tmp2, j);
        }
        FWRITE_RECORD(file, record_size, tmp1, j + 1);
    }
    fclose(file);
    return 0;
}

int copy_sys(const char *src_filename, const char *dest_filename, size_t record_qtty, size_t record_size) {
    int src = open(src_filename, O_RDONLY);
    CHECK_OPEN(src, src_filename);
    int dest = open(dest_filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if (dest == -1) {
        fprintf(stderr, "error opening file: %s\n", dest_filename);
        close(src);
        return 1;
    }
    char *tmp = malloc(record_size * sizeof(char));

    if (lseek(src, 0, SEEK_SET) < 0) {
        fprintf(stderr, "lseek error\n");
        return 1;
    }
    for (int i = 0; i < record_qtty; ++i) {
        if (read(src, tmp, record_size) != record_size) {
            fprintf(stderr, "read error\n");
            return 1;
        }
        if (write(dest, tmp, record_size) != record_size) {
            fprintf(stderr, "write error\n");
            return 1;
        }
    }
    close(src);
    close(dest);
    return 0;
}

int copy_lib(const char *src_filename, const char *dest_filename, size_t record_qtty, size_t record_size) {
    FILE *src = fopen(src_filename, "r");
    CHECK_FOPEN(src, src_filename);
    FILE *dest = fopen(dest_filename, "w");
    if (dest == NULL) {
        fprintf(stderr, "error fopening file: %s\n", dest_filename);
        fclose(src);
        return 1;
    }
    char *tmp = malloc(record_size * sizeof(char));

    if (fseek(src, 0, 0) < 0) {
        fprintf(stderr, "lseek error\n");
        return 1;
    }
    for (int i = 0; i < record_qtty; ++i) {
        if (fread(tmp, sizeof(char), record_size, src) != record_size) {
            fprintf(stderr, "read error\n");
            return 1;
        }
        if (fwrite(tmp, sizeof(char), record_size, dest) != record_size) {
            fprintf(stderr, "write error\n");
            return 1;
        }
    }
    fclose(src);
    fclose(dest);
    return 0;
}

int measure_generate(const char *filename, size_t record_qtty, size_t record_size) {
    printf("Generating %s file...\n", filename);
    SURTime start = current_SURTime();
    generate(filename, record_qtty, record_size);
    SURTime end = current_SURTime();

    show_SURTime(start, end, 1);
    return EXIT_SUCCESS;
}

int measure_sort(const char *filename, size_t record_qtty, size_t record_size) {
    char *temp = calloc(strlen(filename) + 16, sizeof(char));
    printf("Making first copy of %s...\n", filename);
    sprintf(temp,
            "cp %s %s.one",
            filename,
            filename
    );
    system((char *) temp);
    printf("Making second copy of %s...\n", filename);
    sprintf(temp,
            "cp %s %s.two",
            filename,
            filename
    );
    system((char *) temp);

    printf("Sorting %s.one using system IO functions...\n", filename);
    sprintf(temp,
            "%s.one",
            filename
    );
    SURTime start1 = current_SURTime();
    sort_sys(temp, record_qtty, record_size);
    SURTime end1 = current_SURTime();

    show_SURTime(start1, end1, 1);

    printf("Sorting %s.two using library IO functions...\n", filename);
    sprintf(temp,
            "%s.two",
            filename
    );
    SURTime start2 = current_SURTime();
    sort_lib(temp, record_qtty, record_size);
    SURTime end2 = current_SURTime();

    show_SURTime(start2, end2, 1);
    free(temp);
    return EXIT_SUCCESS;
}

int measure_copy(const char *src_filename, const char *dest_filename, size_t record_qtty, size_t record_size) {
    printf("Copying %s to %s using system IO functions...\n", src_filename, dest_filename);
    SURTime start1 = current_SURTime();
    copy_sys(src_filename, dest_filename, record_qtty, record_size);
    SURTime end1 = current_SURTime();

    show_SURTime(start1, end1, 1);

    printf("Copying %s to %s using library IO functions...\n", src_filename, dest_filename);
    SURTime start2 = current_SURTime();
    copy_lib(src_filename, dest_filename, record_qtty, record_size);
    SURTime end2 = current_SURTime();

    show_SURTime(start2, end2, 1);
    return EXIT_SUCCESS;
}

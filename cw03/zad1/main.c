#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#define __USE_XOPEN_EXTENDED

#include <ftw.h>

#define __USE_XOPEN

#include <time.h>

typedef struct dirent dirent;

time_t given_time = 0;
char comp = 0;

bool is_abs_path(const char *);

char *mode_to_string(mode_t);

char *paths_join(const char *, const char *);

void show_file(const char *, const struct stat *);

bool fulfills_time_criteria(const struct stat *);

void traverse_dir(const char *);

void traverse_dir_nftw(const char *);

int nftw_callback(const char *, const struct stat *, int, struct FTW *);

bool is_abs_path(const char *path) {
    return path[0] == '/';
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Bad parameters\n");
        exit(EXIT_FAILURE);
    }
    const char *path = argv[1];
    comp = argv[2][0];

    if (comp != '>' && comp != '<' && comp != '=') {
        fprintf(stderr, "Second parameter should be either '<', '=' or '>'\n");
        exit(EXIT_FAILURE);
    }

    struct tm time;
    if (strptime(argv[3], "%Y-%m-%d %H:%M:%S", &time) == NULL) {
        fprintf(stderr, "Wrong time format, it should be 'YY-mm-dd HH:MM:SS'\n");
        exit(EXIT_FAILURE);
    }
    given_time = mktime(&time);
    if (given_time == -1) exit(EXIT_FAILURE);

    printf("Traversal using opendir:\n");
    traverse_dir(path);

    printf("Traversal using nftw:\n");
    traverse_dir_nftw(path);

    return 0;
}

char *mode_to_string(mode_t mode) {
    char *result = calloc(10, sizeof(char));
    result[0] = (mode & S_IRUSR) ? 'r' : '-';
    result[1] = (mode & S_IWUSR) ? 'w' : '-';
    result[2] = (mode & S_IXUSR) ? 'x' : '-';
    result[3] = (mode & S_IRGRP) ? 'r' : '-';
    result[4] = (mode & S_IWGRP) ? 'w' : '-';
    result[5] = (mode & S_IXGRP) ? 'x' : '-';
    result[6] = (mode & S_IROTH) ? 'r' : '-';
    result[7] = (mode & S_IWOTH) ? 'w' : '-';
    result[8] = (mode & S_IXOTH) ? 'x' : '-';
    result[9] = 0;
    return result;
}

char *paths_join(const char *path1, const char *path2) {
    char *buf;
    size_t len1 = strlen(path1);
    size_t len2 = strlen(path2);

    if (is_abs_path(path2)) {
        buf = malloc(len2 + 1);
        strcpy(buf, path2);
    } else {
        char *sep = "/";
        buf = malloc(len1 + len2 + 2);
        if (path1[len1 - 1] == '/') {
            sep = "";
        }
        sprintf(buf, "%s%s%s", path1, sep, path2);
    }
    return buf;
}

void show_file(const char *path, const struct stat *stats) {
    char *abs_path = calloc(PATH_MAX + 1, sizeof(char));
    if (realpath(path, abs_path) == NULL) {
        fprintf(stderr, "Error, when obtaining absolute path\n");
    }
    long size = stats->st_size;
    struct timespec mtime = stats->st_mtim;
    time_t time = mtime.tv_sec;
    char *mode = mode_to_string(stats->st_mode);
    char *time_str = ctime(&time);

    printf("%s | %10ld | %.*s | %s\n", mode, size, (int) strlen(time_str) - 1, time_str, abs_path);

    free(mode);
    free(abs_path);
}

bool fulfills_time_criteria(const struct stat *stats) {
    time_t file_days = stats->st_mtim.tv_sec / (3600 * 24);
    time_t target_days = given_time / (3600 * 24);
    long int diff = file_days - target_days;

    return (comp == '<' && diff < 0) || (comp == '=' && diff == 0) || (comp == '>' && diff > 0);
}

void traverse_dir(const char *path) {
    DIR *dir = opendir(path);
    dirent *entry = NULL;
    char *filepath = NULL;
    struct stat *stats = calloc(1, sizeof(struct stat));

    if (dir == NULL) {
        fprintf(stderr, "Error opening dir %s\n", path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        filepath = paths_join(path, entry->d_name);
        lstat(filepath, stats);

        if (S_ISREG(stats->st_mode) && fulfills_time_criteria(stats)) {
            show_file(filepath, stats);
        }
        free(filepath);
    }

    rewinddir(dir);

    while ((entry = readdir(dir)) != NULL) {
        filepath = paths_join(path, entry->d_name);
        lstat(filepath, stats);

        if (S_ISDIR(stats->st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            pid_t pid;
            pid = fork();
            if (pid == 0) {
                traverse_dir(filepath);
                exit(EXIT_SUCCESS);
            } else if (pid == -1) {
                fprintf(stderr, "Fork error\n");
                exit(EXIT_FAILURE);
            }
        }
        free(filepath);
    }

    free(stats);
    closedir(dir);
}

int nftw_callback(const char *path, const struct stat *stats, int typeflag, struct FTW *ftwbuf) {
    if (typeflag == FTW_F && fulfills_time_criteria(stats)) show_file(path, stats);
    return 0;
}

void traverse_dir_nftw(const char *path) {
    nftw(path, &nftw_callback, 512, 0);
}

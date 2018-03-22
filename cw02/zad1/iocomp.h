#ifndef CW02_IOCOMP_H
#define CW02_IOCOMP_H

int generate(const char *, size_t, size_t);

int sort_sys(const char *, size_t, size_t);

int sort_lib(const char *, size_t, size_t);

int copy_sys(const char *, const char *, size_t, size_t);

int copy_lib(const char *, const char *, size_t, size_t);

int measure_generate(const char *, size_t, size_t);

int measure_sort(const char *, size_t, size_t);

int measure_copy(const char *, const char *, size_t, size_t);

int show(const char *, size_t, size_t);

int show_record(char *, size_t);

#endif //CW02_IOCOMP_H

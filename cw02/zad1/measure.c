#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dlfcn.h>
#include <sys/resource.h>
#include "measure.h"

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

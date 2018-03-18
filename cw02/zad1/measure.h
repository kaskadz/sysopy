//
// Created by Kasper on 18/03/2018.
//

#ifndef CW02_MEASURE_H
#define CW02_MEASURE_H

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

#endif //CW02_MEASURE_H

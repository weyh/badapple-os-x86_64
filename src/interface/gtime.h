#ifndef __TIME_H__
#define __TIME_H__
#include <stdint.h>

typedef struct {
    char second;
    char minute;
    char hour;
    char day;
    char month;
    char year;
} cmos_time;

cmos_time get_time();

void sdelay(uint32_t sec);

void mdelay(uint32_t ms);

#endif

#include <stdint.h>
#include <stddef.h>
#include "gtdlib.h"
#include "gtime.h"
#include "asm/x86_common.h"

static char time[10];

enum { cmos_address = 0x70, cmos_data = 0x71 };

static int cmos_ready() {
    out_byte(cmos_address, 10);
    return (in_byte(cmos_data) & 0x80);
}

void read_rtc() {
    while (cmos_ready()) {
    }

    out_byte(cmos_address, 0x00);
    time[0] = in_byte(cmos_data);
    out_byte(cmos_address, 0x02);
    time[1] = in_byte(cmos_data);
    out_byte(cmos_address, 0x04);
    time[2] = in_byte(cmos_data);
    out_byte(cmos_address, 0x06);
    time[3] = in_byte(cmos_data);
    out_byte(cmos_address, 0x07);
    time[4] = in_byte(cmos_data);
    out_byte(cmos_address, 0x08);
    time[5] = in_byte(cmos_data) - 1;
    out_byte(cmos_address, 0x09);
    time[6] = in_byte(cmos_data);
    out_byte(cmos_address, 0x32);
    time[7] = in_byte(cmos_data);
    out_byte(cmos_address, 0x0a);
    time[8] = in_byte(cmos_data);
    out_byte(cmos_address, 0x0b);
    time[9] = in_byte(cmos_data);

    // 12 hour clock
    if (!(time[9] & 2) && (time[2] & 0x80)) {
        time[2] = ((time[2] & 0x80) + 12) % 24;
    }

    // decimal stuff
    if (!(time[9] & 4)) {
        time[0] = (time[0] & 0xf) + ((time[0] / 16) * 10);
        time[1] = (time[1] & 0xf) + ((time[1] / 16) * 10);
        time[2] = (time[2] & 0xf) + ((time[2] / 16) * 10);
        time[5] = (time[5] & 0xf) + ((time[5] / 16) * 10);
    }

    time[3] = (time[3] + 5) % 7;
}

cmos_time get_time() {
    read_rtc();
    return (cmos_time){
        .year = time[7],
        .month = time[8],
        .day = time[6],
        .hour = time[2],
        .minute = time[1],
        .second = time[0],
    };
}

void sdelay(uint32_t sec) {
    cmos_time t = get_time();
    register const uint32_t seconds = t.second + sec;
    const cmos_time target = {
        .second = seconds % 60,
        .minute = t.minute + (seconds / 60),
        .hour = t.hour + (seconds / 3600),
        .day = t.day + (seconds / 86400),
        .month = t.month + (seconds / 2592000),
        .year = t.year + (seconds / 31104000),
    };

    while (!(t.second == target.second && t.minute == target.minute &&
             t.hour == target.hour && t.day == target.day &&
             t.month == target.month && t.year == target.year)) {
        __asm__("pause");
        t = get_time();
    }
}

void mdelay(uint32_t ms) {
    static const uint64_t ref_freq_fixedpoint =
        3200000000 * (1 << 16) / 1000000000;

    const uint64_t start = rdtsc();
    const uint64_t ns = (ms * (uint64_t)1000000 * ref_freq_fixedpoint) >> 16;

    while (rdtsc() - start < ns) {
        __asm__ volatile("pause");
    }
}

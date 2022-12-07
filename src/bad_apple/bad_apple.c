#include "vid.h"
#include "gtdlib.h"
#include "gtdio.h"
#include "gtring.h"
#include "gtime.h"

struct frame_data {
    int width, height, frame_rate;
};

void start_bad_apple() {
    struct frame_data data;
    char *const w = malloc(strlen(bad_apple_vid[0]) * sizeof(char) + 1);
    memcpy(w, bad_apple_vid[0], strlen(bad_apple_vid[0]) + 1);
    *(w + 2) = 0;
    char *const h = w + 3;
    *(h + 2) = 0;
    char *const f = h + 3;

    data.width = atoi(w);
    data.height = atoi(h);
    data.frame_rate = atoi(f);

    free(w);
    puts("Bad Apple! start");
    mdelay(1000);

    reset_color();
    set_cursor(0, 0);
    for (register size_t i = 1; i < BAD_APPLE_VID_LEN; i++) {
        puts(bad_apple_vid[i]);
        putc('\n');

        if (i % data.height == 0) {
            mdelay(1000 / data.frame_rate);
            set_cursor(0, 0);
        }
    }

    printf("Bad Apple! played in %d frames", BAD_APPLE_VID_LEN);
}

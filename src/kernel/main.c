#include "gtdio.h"
#include "gtdlib.h"
#include "gtime.h"

extern void start_bad_apple();

void kernel_main() {
    clear();
    puts("Kernel loading...\n");

    gtd_dma_init();
    puts("Init heap      [OK  ]\n");

    start_bad_apple();
}

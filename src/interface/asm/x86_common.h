#ifndef __ASM_X86_COMMON_H__
#define __ASM_X86_COMMON_H__
#include <stdint.h>
#include <stddef.h>

static void out_byte(uint16_t port, int8_t data) {
    __asm__ __volatile__("out %%al, %%dx" : : "a"(data), "d"(port));
}

static uint8_t in_byte(uint16_t port) {
    uint8_t data;
    __asm__ __volatile__("in %1, %0" : "=a"(data) : "d"(port));
    return data;
}

static inline void sti(void) { __asm__ __volatile__("sti"); }

static inline void cli(void) { __asm__ __volatile__("cli"); }

static uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (lo | ((uint64_t)hi << 32));
}

#endif

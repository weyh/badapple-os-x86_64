#ifndef __GTDLIB_H__
#define __GTDLIB_H__

#include <stdint.h>
#include <stddef.h>

void gtd_dma_init();
void *malloc(size_t);
void *calloc(size_t, size_t);
void free(void *);
void *realloc(void *p, size_t size);

void memcpy(void *dest, const void *src, size_t size);
void memset(void *dest, uint8_t value, size_t size);

#endif

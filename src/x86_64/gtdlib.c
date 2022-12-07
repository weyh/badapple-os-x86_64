#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "gtdlib.h"

typedef struct __block {
    size_t size;
    bool free;
    struct __block *next;
    struct __block *prev;
    uint8_t __debug_id;
} __attribute__((packed)) block;

#define BLOCK_SIZE sizeof(block)
#define DYNAMIC_MEM_TOTAL_SIZE 4 * 4096

static uint8_t dynamic_mem[DYNAMIC_MEM_TOTAL_SIZE] = {0};
static block *dynamic_mem_start;

void gtd_dma_init() {
    dynamic_mem_start = (block *)dynamic_mem;
    dynamic_mem_start->size = DYNAMIC_MEM_TOTAL_SIZE - BLOCK_SIZE;
    dynamic_mem_start->free = true;
    dynamic_mem_start->next = NULL;
    dynamic_mem_start->prev = NULL;
}

static void *gtd_dma_find_block(block *dynamic_mem, size_t size) {
    block *ret_mem_block = NULL;
    size_t ret_mem_block_size = DYNAMIC_MEM_TOTAL_SIZE + 1;

    block *current_mem_block = dynamic_mem;
    while (current_mem_block) {
        if (current_mem_block->free &&
            current_mem_block->size >= (size + BLOCK_SIZE) &&
            current_mem_block->size <= ret_mem_block_size) {

            ret_mem_block = current_mem_block;
            ret_mem_block_size = current_mem_block->size;
        }

        current_mem_block = current_mem_block->next;
    }

    return ret_mem_block;
}

uint8_t id_now = 0;

void *malloc(size_t size) {
    block *best_block = gtd_dma_find_block(dynamic_mem_start, size);

    if (best_block == NULL)
        return NULL;

    /*
        subtract newly allocated memory (incl. size of the mem block) from
        selected block
    */
    best_block->size = best_block->size - size - BLOCK_SIZE;

    /*
        create new mem block after selected block, effectively splitting the
        memory region
    */
    block *block_allocate =
        (block *)(((uint8_t *)best_block) + BLOCK_SIZE + best_block->size);
    block_allocate->size = size;
    block_allocate->free = false;
    block_allocate->next = best_block->next;
    block_allocate->prev = best_block;
    block_allocate->__debug_id = ++id_now;

    // reconnect the double linked list
    if (best_block->next != NULL)
        best_block->next->prev = block_allocate;
    best_block->next = block_allocate;

    return (void *)((uint8_t *)block_allocate + BLOCK_SIZE);
}

void *calloc(size_t num, size_t size) {
    void *ret = malloc(num * size);
    memset(ret, 0, num * size);
    return ret;
}

static void *merge_next_block_into_current(block *current_block) {
    block *next_block = current_block->next;

    if (next_block != NULL && next_block->free) {
        // add size of next block to current block
        current_block->size += current_block->next->size;
        current_block->size += BLOCK_SIZE;

        // remove next block from list
        current_block->next = current_block->next->next;
        if (current_block->next != NULL)
            current_block->next->prev = current_block;
    }

    return current_block;
}

static void merge_current_block_into_previous(block *current_block) {
    block *prev_block = current_block->prev;

    if (prev_block != NULL && prev_block->free) {
        // add size of previous block to current block
        prev_block->size += current_block->size;
        prev_block->size += BLOCK_SIZE;

        // remove current node from list
        prev_block->next = current_block->next;
        if (current_block->next != NULL) {
            current_block->next->prev = prev_block;
        }
    }
}

void free(void *p) {
    if (p == NULL)
        return;

    // get mem node associated with pointer
    block *current_block = (block *)((uint8_t *)p - BLOCK_SIZE);

    // pointer we're trying to free was not dynamically allocated it seems
    if (current_block == NULL)
        return;

    // mark block as unused
    current_block->free = true;

    // merge unused blocks
    current_block = merge_next_block_into_current(current_block);
    merge_current_block_into_previous(current_block);
}

void *realloc(void *p, size_t size) {
    if (p == NULL)
        return malloc(size);

    block *current_block = (block *)((uint8_t *)p - BLOCK_SIZE);

    // pointer we're trying to free was not dynamically allocated it seems
    if (current_block == NULL)
        return NULL;

    // if new size is smaller than current size, just return the pointer
    if (size <= current_block->size)
        return p;

    // if new size is larger than current size, allocate new memory and copy
    // contents
    void *new_p = malloc(size);
    memcpy(new_p, p, current_block->size);
    free(p);

    return new_p;
}

void memcpy(void *dest, const void *src, size_t size) {
    if (dest == NULL || src == NULL)
        return;

    uint8_t *dest8 = (uint8_t *)dest;
    const uint8_t *src8 = (uint8_t *)src;

    while (size--)
        *dest8++ = *src8++;
}

void memset(void *dest, uint8_t value, size_t size) {
    if (dest == NULL)
        return;

    uint8_t *dest8 = (uint8_t *)dest;

    while (size--)
        *dest8++ = value;
}

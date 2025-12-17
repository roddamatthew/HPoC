#pragma once
#include <stdio.h>
#include <stdint.h>

#define INITIAL_SIZE 0x40000
#define STRETCH_SIZE 0x40000

typedef struct {
    size_t prev_size;
    size_t size : 61;
    size_t NON_MAIN_ARENA : 1;
    size_t IS_MMAPPED : 1;
    size_t PREV_INUSE : 1;
    uint64_t fd;              // Only used if chunk is free
    uint64_t bk;              // Only used if chunk is free
    uint64_t fd_nextsize;     // Only used if chunk is free
    uint64_t bk_nextsize;     // Only used if chunk is free
}chunk;

#define chunk2mem(p)    ((void *)((uint64_t)(p) + 16))
#define mem2chunk(p)    ((chunk*)((uint64_t)(p) - 16))
#pragma once
#include <stdio.h>

#define INITIAL_SIZE 0x40000
#define STRETCH_SIZE 0x40000

typedef struct {
    size_t prev_size;
    size_t size : 61;
    size_t NON_MAIN_ARENA : 1;
    size_t IS_MMAPED : 1;
    size_t PREV_INUSE : 1;
    struct chunk *fd;              // Only used if chunk is free
    struct chunk *bk;              // Only used if chunk is free
    struct chunk *fd_nextsize;     // Only used if chunk is free
    struct chunk *bk_nextsize;     // Only used if chunk is free
}chunk;



#define chunk2mem(p)    (void *)((char *)(p) + 2 * sizeof(size_t))
#define mem2chunk(p)    (chunk *)((char *)p - 2 * sizeof(size_t))


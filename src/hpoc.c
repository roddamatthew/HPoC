#include <stdio.h>
#include <sys/mman.h>
#include "hpoc.h"

#define INITIAL_SIZE 0x20000
#define STRETCH_SIZE 0x20000

typedef struct {
    size_t prev_size;
    size_t size : 61;
    size_t NON_MAIN_ARENA : 1;
    size_t IS_MMAPED : 1;
    size_t PREV_INUSE : 1;
    char data[];
}a_chunk;

typedef struct {
    size_t prev_size;
    size_t size : 61;
    size_t NON_MAIN_ARENA : 1;
    size_t IS_MMAPED : 1;
    size_t PREV_INUSE : 1;
    f_chunk *fd;
    f_chunk *bk;
    size_t fd_nextsize;
    size_t bk_nextsize;
}f_chunk;

typedef struct {
    f_chunk *bins[8];
}TCache;
TCache tcache; // fd only

typedef struct {
    // TCache tcache;
    // FastBins fast_bins;
    // SmallBins small_bins;
    // LargeBins large_bins;
    f_chunk *unsorted_bin;
    a_chunk top_chunk;
}arena;

static arena main_arena = {0};


void hpoc_init() {
    char *ptr = (char *)mmap(0, INITIAL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    main_arena.top_chunk = (a_chunk) {0, INITIAL_SIZE, 0, 0, 1};

    
}

void *malloc(size_t size) {
    // If the top chunk is out of memory
    if (main_arena.top_chunk.size == 0)
        hpoc_init();

    // If the tcache for the requested size isn't empty, return the HEAD

    // ...

    // If none of this works, take from the start of the top chunk
    // PLAN
    // create a new chunk from the start of the top_chunk
    // have to move the top chunk forward size + sizeof(a_chunk_header)
    // 

    // a_chunk new_chunk = ...
    // main_arena.top_chunk

    
}

void free(void *ptr) {
    f_chunk* chunk = (f_chunk *)ptr - 16;
    
    // Place in tcache if size fits and there's room

    // Place in fastbin if size fits

    // Attempt to consolidate forwards and backwards

    // Place at the HEAD of the unsorted list to sorted later
    chunk->fd = main_arena.unsorted_bin;
    chunk->bk = NULL;
    main_arena.unsorted_bin->bk = chunk;
    main_arena.unsorted_bin = chunk;
}
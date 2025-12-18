#include <stdlib.h>
#include <sys/mman.h>
#include "../include/chunk.h"

chunk* heap_start = NULL; // for debugging :P
chunk* top_chunk = NULL;
chunk* free_list = NULL;

static void init_allocator()
{
    top_chunk = mmap(0, INITIAL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (top_chunk == MAP_FAILED) {
        perror("Couldn't mmap requested size");
        exit(1);
    }

    heap_start = top_chunk; // for debug

    // Fill the chunk attributes
    top_chunk->prev_size = 0;
    top_chunk->size = INITIAL_SIZE;
    top_chunk->NON_MAIN_ARENA = 0;
    top_chunk->IS_MMAPPED = 1;
    top_chunk->PREV_INUSE = 1;
}
void* hmalloc(size_t size)
{
    if (!top_chunk) {
        init_allocator();
    }

    // Round up to nearest 16-byte size
    size_t mem_size = (1 + ((size - 1) / 16)) * 16;
    size_t chunk_size = mem_size + 16;

    // Carve the new chunk out of the top_chunk
    chunk* new_chunk = top_chunk;

    // Bump the top_chunk forward and populate it
    top_chunk = (chunk*)((uint64_t)top_chunk + chunk_size);
    top_chunk->prev_size = chunk_size;
    top_chunk->size = new_chunk->size - chunk_size;
    top_chunk->NON_MAIN_ARENA = 0;
    top_chunk->IS_MMAPPED = 1;
    top_chunk->PREV_INUSE = 1;

    // Fill the new chunk's size
    // Other attributes are already set from previous calls
    new_chunk->size = chunk_size;

    return chunk2mem(new_chunk);
}

void hfree(void* ptr)
{
    // Doesn't need to do anything!
}
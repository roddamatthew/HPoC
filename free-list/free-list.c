#include <stdlib.h>
#include <sys/mman.h>
#include "../include/chunk.h"

static chunk* top_chunk = NULL;
static chunk* free_list = NULL;

static void init_allocator()
{
    top_chunk = mmap(0, INITIAL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (top_chunk == MAP_FAILED) {
        perror("Couldn't mmap requested size");
        exit(1);
    }

    // Fill the chunk attributes
    top_chunk->prev_size = 0;
    top_chunk->size = INITIAL_SIZE;
    top_chunk->NON_MAIN_ARENA = 0;
    top_chunk->IS_MMAPPED = 1;
    top_chunk->PREV_INUSE = 1;
}

// Traverse the free list and return a chunk of required size if found
static chunk* find_free_chunk(uint64_t size)
{
    chunk* curr_chunk = free_list;
    while(curr_chunk) {
        if (curr_chunk->size >= size) {
            return curr_chunk;
        }

        curr_chunk = (chunk *)curr_chunk->fd;
    }
    return NULL;
}

static void drop_free_chunk(chunk* c)
{
    chunk* curr_chunk = free_list;
    while(curr_chunk) {
        if (curr_chunk == c) {
            if (c->bk) {
                ((chunk *)(c->bk))->fd = c->fd;
            }
            if (c->fd) {
                ((chunk *)(c->fd))->bk = c->bk;
            }
            return;
        }

        curr_chunk = (chunk *)curr_chunk->fd;
    }
}

// Allocate a new chunk from the top chunk
static void* allocate_from_top_chunk(size_t chunk_size)
{
    // Carve the new chunk out of the top_chunk
    chunk* new_chunk = top_chunk;

    // Bump the top_chunk forward and populate it
    top_chunk = (chunk*)((uint64_t)top_chunk + chunk_size);
    top_chunk->prev_size = chunk_size;
    top_chunk->size -= chunk_size;
    top_chunk->NON_MAIN_ARENA = 0;
    top_chunk->IS_MMAPPED = 1;
    top_chunk->PREV_INUSE = 1;

    // Fill the new chunk's size
    // Other attributes are already set from previous calls
    new_chunk->size = chunk_size;
    return chunk2mem(new_chunk);
}

// Allocate a chunk from an existing chunk in the free list
static chunk* allocate_from_free_list(chunk* free_chunk, size_t new_chunk_size)
{
    // Allocate from the front of the existing chunk
    chunk* new_chunk = free_chunk;

    // TODO:
    free_chunk = (chunk*)((uint64_t)free_chunk + new_chunk_size);


    
    // update the free chunk size
    // update the free chunk location
    // update the free list to point to the free_chunk
    // check if there's even a free_chunk leftover

    // Fill in the new chunk's attributes
    new_chunk->size = new_chunk_size;
    return chunk2mem(new_chunk)
}

void* hmalloc(size_t size)
{
    if (!top_chunk) {
        init_allocator();
    }

    // Round up to nearest 16-byte size
    size_t mem_size = (1 + ((size - 1) / 16)) * 16;
    size_t chunk_size = mem_size + 16;

    // First try to allocate from the free list
    chunk* free_chunk = find_free_chunk(chunk_size);
    if (free_chunk) {
        return allocate_from_free_list(free_chunk, chunk_size);
    }

    // Otherwise allocate from the top chunk
    return allocate_from_top_chunk(chunk_size);
}

// Append to the front of the list
static void free_list_append(chunk* c)
{
    // Update the backwards pointer if the free list isn't NULL
    if (free_list) {
        free_list->bk = (uint64_t)c;
    }

    // Append the new chunk to the front of the list
    c->bk = (uint64_t)NULL;
    c->fd = (uint64_t)free_list;
    free_list = c;
}

// Place a chunk on the free_list to be allocated from later
void hfree(void* ptr)
{
    chunk* free_chunk = mem2chunk(ptr);
    free_list_append(free_chunk);
}

void print_free_list()
{
    chunk* curr_chunk = free_list;
    while(curr_chunk) {
        printf("free chunk @%p\n", curr_chunk);
        printf("\tprev_size: %lu\n", curr_chunk->prev_size);
        printf("\tsize: %lu\n", curr_chunk->size);
        printf("\tNON_MAIN_ARENA: %d\n", curr_chunk->NON_MAIN_ARENA);
        printf("\tIS_MMAPPED: %d\n", curr_chunk->IS_MMAPPED);
        printf("\tPREV_INUSE: %d\n", curr_chunk->PREV_INUSE);
        printf("\tfd: %p\n", (void *)curr_chunk->fd);
        printf("\tbk: %p\n", (void *)curr_chunk->bk);

        curr_chunk = (chunk *)curr_chunk->fd;
    }
}
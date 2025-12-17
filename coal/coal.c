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

// Pop a chunk from the free_list by updated its neighbors fd and bk pointers
static void pop_free_chunk(chunk* c)
{
    if (free_list == c) {
        free_list = (chunk*)c->fd;
    }
    
    if (c->bk) {
        ((chunk *)(c->bk))->fd = c->fd;
    }
    if (c->fd) {
        ((chunk *)(c->fd))->bk = c->bk;
    }
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

    // Check if this new allocation takes the whole free chunk
    // If we would've been left with a 2 byte chunk, include it in new_chunk
    if (new_chunk_size == free_chunk->size) {
        pop_free_chunk(free_chunk);
        return chunk2mem(new_chunk);
    }

    // Otherwise we have some leftover data
    // Bump the free_chunk forward
    size_t new_free_chunk_size = free_chunk->size - new_chunk_size;
    chunk* new_free_chunk = (chunk*)((uint64_t)free_chunk + new_chunk_size);
    new_free_chunk->prev_size = new_chunk_size;
    new_free_chunk->size = new_free_chunk_size;
    new_free_chunk->NON_MAIN_ARENA = 0;
    new_free_chunk->IS_MMAPPED = 1;
    new_free_chunk->PREV_INUSE = 1;

    // Remove the old free chunk
    pop_free_chunk(free_chunk);
    free_list_append(new_free_chunk);

    // Fill in the new chunk's attributes
    new_chunk->size = new_chunk_size;
    return chunk2mem(new_chunk);
}

// Coalesce neighboring free heap chunks to reduce fragmentation
static void coalesce()
{
    // PLAN:
    // Iterate through the free list
    // Check PREV_INUSE
    // If zero, then move back 

    chunk* curr_chunk = free_list;
    while(curr_chunk) {


        curr_chunk = (chunk*)curr_chunk->fd;
    }
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

// Place a chunk on the free_list to be allocated from later
void hfree(void* ptr)
{
    chunk* free_chunk = mem2chunk(ptr);
    free_list_append(free_chunk);
}

void print_free_list()
{
    chunk* curr_chunk = free_list;
    printf("--- FREE LIST STATE: ---\n");
    while (curr_chunk) {
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
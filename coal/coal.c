#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "../include/chunk.h"

static chunk* heap_start = NULL; // for debugging :P
static chunk* top_chunk = NULL;
static chunk* free_list = NULL;

// Forward declare all methods
static void print_chunk(chunk* c);
void print_free_list();
void print_heap();
static void init_allocator();
static chunk* find_free_chunk(uint64_t size);
static void pop_free_chunk(chunk* c);
static bool chunk_in_free_list(chunk* c);
void* hmalloc(size_t size);
void hfree(void *ptr);


static void print_chunk(chunk* c)
{
    printf("chunk @%p:\n\t", c);
    printf("prev_size: 0x%lx, ", c->prev_size);
    printf("size: 0x%lx, ", c->size);

    // Print the flags nicely
    printf("flags: |");
    if (c->NON_MAIN_ARENA) printf("A");
    else printf(".");
    printf("|");
    if (c->IS_MMAPPED) printf("M");
    else printf(".");
    printf("|");
    if (c->PREV_INUSE) printf("P");
    else printf(".");
    printf("|\n");

    if (chunk_in_free_list(c)) {
        printf("\tis_free: ");
        printf("fd: %p, ", (void *)c->fd);
        printf("bk: %p\n", (void *)c->bk);
    }
}

void print_free_list()
{
    chunk* curr_chunk = free_list;
    puts("[+]\t--- FREE LIST STATE: ---");
    while (curr_chunk) {
        print_chunk(curr_chunk);
        curr_chunk = (chunk *)curr_chunk->fd;
    }
    puts("");
}

void print_heap()
{
    chunk* curr_chunk = heap_start;
    puts("[+]\t--- HEAP STATE: ---");
    while (curr_chunk <= top_chunk) {
        // Print the current chunk
        print_chunk(curr_chunk);
        
        // Check for a corrupted-size chunk
        if (curr_chunk->size < 16) {
            perror("[!] curr_chunk->size less than minimum size (16 bytes)!");
            exit(1);
        }

        // Just in case...
        if ((uint64_t)curr_chunk > (uint64_t)heap_start + INITIAL_SIZE) {
            perror("[!] print_heap() read past the end of the heap!");
            exit(1);
        }

        // Move to the next contiguous chunk in memory
        curr_chunk = nextchunk(curr_chunk);
    }
    puts("");
}

static void init_allocator()
{
    top_chunk = mmap(0, INITIAL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (top_chunk == MAP_FAILED) {
        perror("[!] Couldn't mmap requested size");
        exit(1);
    }
    heap_start = top_chunk; // For debugging

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
    while (curr_chunk) {
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
    top_chunk->size = new_chunk->size - chunk_size;
    top_chunk->NON_MAIN_ARENA = 0;
    top_chunk->IS_MMAPPED = 1;
    top_chunk->PREV_INUSE = 1;
    print_chunk(top_chunk);

    // Fill the new chunk's size
    // Other attributes are already set from previous calls
    new_chunk->size = chunk_size;
    return chunk2mem(new_chunk);
}

// Allocate a chunk from an existing chunk in the free list
static chunk* allocate_from_free_list(chunk* free_chunk, size_t new_chunk_size)
{
    // Check if this new allocation takes the whole free chunk, if so just return it
    // We also require 32 bytes for the free chunk so we can't be leftover with less than that
    if (new_chunk_size >= free_chunk->size - 32) {
        pop_free_chunk(free_chunk);
        chunk* next_chunk = nextchunk(free_chunk);
        if (next_chunk < top_chunk) {
            next_chunk->PREV_INUSE = 1;
        }
        return chunk2mem(free_chunk);
    }

    printf("Tried to allocate from free chunk with %p, %lx\n", free_chunk, new_chunk_size);

    // Otherwise we have some leftover free memory to manage
    size_t leftover_free_chunk_size = free_chunk->size - new_chunk_size;
    chunk* leftover_free_chunk = (chunk*)((uint64_t)free_chunk + new_chunk_size);

    // Update the leftover free chunk's attributes 
    leftover_free_chunk->prev_size = new_chunk_size;
    leftover_free_chunk->size = leftover_free_chunk_size;
    leftover_free_chunk->NON_MAIN_ARENA = 0;
    leftover_free_chunk->IS_MMAPPED = 1;
    leftover_free_chunk->PREV_INUSE = 1;

    // Put it in the free list
    free_list_append(leftover_free_chunk);

    // Remove the old free chunk from the free list
    pop_free_chunk(free_chunk);

    // Update the old free_chunk and return it
    free_chunk->size = new_chunk_size;
    return chunk2mem(free_chunk);
}

static bool chunk_in_free_list(chunk* c)
{
    chunk* curr_chunk = free_list;
    while (curr_chunk) {
        if (curr_chunk == c) {
            return true;
        }
        curr_chunk = (chunk*)curr_chunk->fd;
    }
    return false;
}

// Coalesce neighboring free heap chunks to reduce fragmentation
static void backward_coalesce()
{
    chunk* curr_chunk = free_list;
    while (curr_chunk) {
        // If the previous chunk in memory is also free, it can be coalesced
        if (!curr_chunk->PREV_INUSE) {
            // Extend the previous chunk to coalesce the current chunk
            chunk* prev_chunk = prevchunk(curr_chunk);
            prev_chunk->size += curr_chunk->size;
            pop_free_chunk(curr_chunk);

            // Update the next chunk's prev_size
            chunk* next_chunk = nextchunk(prev_chunk);
            next_chunk->prev_size = prev_chunk->size;

            // Debug :P
            assert(chunk_in_free_list(prev_chunk));
            assert(!chunk_in_free_list(curr_chunk));
        }
        curr_chunk = (chunk*)curr_chunk->fd;
    }

    // Finally, try to coalesce into the top chunk
    if(!top_chunk->PREV_INUSE) {
        chunk* new_top_chunk = prevchunk(top_chunk);
        pop_free_chunk(new_top_chunk);

        // Set the new top chunks attributes
        new_top_chunk->size += top_chunk->size;
        top_chunk = new_top_chunk;
    }
}

void* hmalloc(size_t size)
{
    if (!top_chunk) {
        init_allocator();
    }

    if (free_list) {
        backward_coalesce();
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
    chunk* next_chunk = nextchunk(free_chunk);
    next_chunk->PREV_INUSE = 0;
    
    free_list_append(free_chunk);
}
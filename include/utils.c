#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "chunk.h"

extern chunk* heap_start;
extern chunk* top_chunk;
extern chunk* free_list;

bool in_free_list(chunk* c)
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

// Print a chunk in a standard, nicely formatted way for debugging
void print_chunk(chunk* c)
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

    if (in_free_list(c)) {
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

// Print the heap state by iterating through chunks in memory
// Also do some error checking to help debugging!
void print_heap()
{
    chunk* curr_chunk = heap_start;
    chunk* last_chunk = NULL;
    puts("[+]\t--- HEAP STATE: ---");
    while (curr_chunk <= top_chunk) {
        // Print the current chunk
        print_chunk(curr_chunk);

        // Check that chunk metadata is self-consistent with neighboring chunks
        if(last_chunk) {
            assert(curr_chunk->prev_size == last_chunk->size);
        }
        
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
        last_chunk = curr_chunk;
        curr_chunk = nextchunk(curr_chunk);
    }
    puts("");
}
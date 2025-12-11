#include <stdio.h>
#include <sys/mman.h>
#include "hpoc.h"

void *malloc(size_t size) {
    return mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void free(void *ptr) {
    munmap(ptr, 0x1000);
}
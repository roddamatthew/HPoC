#include "coal.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main()
{
    uint64_t ptr1 = (uint64_t)hmalloc(0x10);
    uint64_t ptr2 = (uint64_t)hmalloc(0x20);
    uint64_t ptr3 = (uint64_t)hmalloc(0x30);

    printf("ptr1: %p\n", (void *)ptr1);
    printf("ptr2: %p\n", (void *)ptr2);
    printf("ptr3: %p\n", (void *)ptr3);

    print_heap();

    assert(ptr1 + 32 == ptr2);
    assert(ptr2 + 48 == ptr3);

    hfree((void*)ptr2);
    hfree((void*)ptr1);
    print_heap();
    
    uint64_t ptr4 = (uint64_t)hmalloc(0x30);
    print_heap();
    assert(ptr1 == ptr4);

    hfree((void*)ptr3);
    hfree((void*)ptr4);

    uint64_t ptr5 = (uint64_t)hmalloc(0x50);
    print_heap();
    assert(ptr1 == ptr5);

    hfree((void*)ptr5);

    // TODO: colaesce into the top chunk!
    uint64_t ptr6 = (uint64_t)hmalloc(0x200);
    print_heap();
    assert(ptr1 == ptr6);
}
#include "free-list.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main()
{
    uint64_t ptr1 = (uint64_t)hmalloc(10);
    print_free_list();
    
    uint64_t ptr2 = (uint64_t)hmalloc(20);
    uint64_t ptr3 = (uint64_t)hmalloc(50);

    printf("ptr1: %p\n", (void *)ptr1);
    printf("ptr2: %p\n", (void *)ptr2);
    printf("ptr3: %p\n", (void *)ptr3);

    assert(ptr1 + 32 == ptr2);
    assert(ptr2 + 48 == ptr3);

    hfree((void *)ptr3);
    uint64_t ptr4 = (uint64_t)hmalloc(32);
    print_free_list();
    uint64_t ptr5 = (uint64_t)hmalloc(16);
    print_free_list();

    assert(ptr3 == ptr4);
    assert(ptr3 + 48 == ptr5);

    hfree((void *)ptr1);
    hfree((void *)ptr2);
    hfree((void *)ptr4);
    hfree((void *)ptr5);
    print_free_list();
}
#include <stdlib.h>

void* hmalloc(size_t size);
void hfree(void* ptr);
void print_free_list();
void print_heap();
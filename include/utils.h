#pragma once
#include <stdbool.h>
#include "chunk.h"

void print_chunk(chunk*);
void print_heap();
void print_free_list();
bool in_free_list(chunk* c);
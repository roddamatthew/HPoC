#include <stdio.h>
#include "hpoc.h"

int main() {
    int *arr = (int *)malloc(sizeof(int) * 32);

    for (int i = 0; i < 32; i++)
        arr[i] = i;
    
    for (int i = 0; i < 32; i++)
        printf("arr[%d]: %d\n", i, arr[i]);
    free(arr);
}
#define _GNU_SOURCE
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void allocate_memory() {
    char *arr[1000];

    for (int i = 0; i < 1000; ++i) {
        arr[i] = malloc(1024);  // allocate 1KB
        if (arr[i] != NULL) {
            memset(arr[i], 0, 1024);
        }
    }

    for (int i = 0; i < 1000; i += 2) {
        free(arr[i]);  // free half of them
    }
}

int main() {
    // Set M_TOP_PAD to 0 to reduce top padding (less fragmentation)
    if (!mallopt(M_TOP_PAD, 0)) {
        printf("mallopt failed to set M_TOP_PAD\n");
    } else {
        printf("mallopt successfully set M_TOP_PAD\n");
    }

    allocate_memory();

    // Print malloc info to check fragmentation
    malloc_stats();

    return 0;
}

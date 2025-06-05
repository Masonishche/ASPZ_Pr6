#include <stdio.h>
#include <stdlib.h>
#include <malloc_np.h>  // FreeBSD-specific header for mallctl
#include <time.h>

#define NUM_BLOCKS 100000
#define MAX_SIZE 512

void run_test(unsigned narenas) {
    // Set number of arenas
    if (mallctl("arenas.narenas", NULL, NULL, &narenas, sizeof(narenas))) {
        perror("mallctl set arenas.narenas");
        exit(1);
    }

    void* blocks[NUM_BLOCKS] = {0};
    size_t sizes[NUM_BLOCKS];
    srand(time(NULL));

    // Allocate memory blocks
    for (int i = 0; i < NUM_BLOCKS; i++) {
        sizes[i] = rand() % MAX_SIZE + 1;
        blocks[i] = malloc(sizes[i]);
        if (!blocks[i]) {
            perror("malloc");
            exit(1);
        }
    }

    // Free every other block to create fragmentation
    for (int i = 0; i < NUM_BLOCKS; i += 2) {
        free(blocks[i]);
        blocks[i] = NULL;
    }

    // Measure fragmentation metrics
    size_t allocated, active, resident;
    size_t len = sizeof(size_t);
    
    // Update statistics
    unsigned epoch = 1;
    mallctl("epoch", NULL, NULL, &epoch, sizeof(epoch));

    mallctl("stats.allocated", &allocated, &len, NULL, 0);
    mallctl("stats.active", &active, &len, NULL, 0);
    mallctl("stats.resident", &resident, &len, NULL, 0);

    printf("\nConfiguration: arenas.narenas = %u\n", narenas);
    printf("Allocated: %zu bytes\n", allocated);
    printf("Active:    %zu bytes\n", active);
    printf("Resident:  %zu bytes\n", resident);
    printf("Fragmentation (active - allocated): %zu bytes\n", active - allocated);
    printf("Fragmentation percentage: %.2f%%\n", 
           (double)(active - allocated) * 100.0 / active);

    // Free remaining memory
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (blocks[i]) free(blocks[i]);
    }
}

int main() {
    printf("Memory Fragmentation Test on FreeBSD\n");
    printf("====================================\n");
    printf("Testing different arena configurations...\n");

    // Test different arena configurations
    unsigned arenas_config[] = {1, 2, 4, 8};
    size_t num_configs = sizeof(arenas_config)/sizeof(arenas_config[0]);
    
    for (int i = 0; i < num_configs; i++) {
        printf("\nTest %d/%d starting...", i+1, num_configs);
        fflush(stdout);
        run_test(arenas_config[i]);
        printf("Test %d/%d completed", i+1, num_configs);
    }
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}

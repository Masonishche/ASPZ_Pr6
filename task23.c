#include <stdio.h>
#include <stdlib.h>
#include <malloc_np.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_BLOCKS 100000
#define MAX_SIZE 512

void run_test(unsigned num_arenas) {
   
    unsigned *arenas = malloc(num_arenas * sizeof(unsigned));
    if (!arenas) {
        perror("malloc arenas array");
        exit(1);
    }

    for (int i = 0; i < num_arenas; i++) {
        size_t arena_index_size = sizeof(unsigned);
        int ret = mallctl("arenas.create", &arenas[i], &arena_index_size, NULL, 0);
        if (ret != 0) {
            fprintf(stderr, "Failed to create arena %d: %s\n", i, strerror(ret));
            exit(1);
        }
    }

    void* blocks[NUM_BLOCKS] = {0};
    size_t sizes[NUM_BLOCKS];
    srand(time(NULL) ^ getpid()); 

    for (int i = 0; i < NUM_BLOCKS; i++) {
        sizes[i] = rand() % MAX_SIZE + 1;
        
        unsigned arena_index = arenas[rand() % num_arenas];
        
        if (mallctl("thread.arena", NULL, NULL, &arena_index, sizeof(arena_index))) {
            fprintf(stderr, "Failed to set arena: %s\n", strerror(ret));
            exit(1);
        }
        
        blocks[i] = malloc(sizes[i]);
        if (!blocks[i]) {
            perror("malloc");
            exit(1);
        }
    }

    for (int i = 0; i < NUM_BLOCKS; i += 2) {
        free(blocks[i]);
        blocks[i] = NULL;
    }

    size_t allocated, active, resident;
    size_t len = sizeof(size_t);
    
    unsigned epoch = 1;
    mallctl("epoch", NULL, NULL, &epoch, sizeof(epoch));

    mallctl("stats.allocated", &allocated, &len, NULL, 0);
    mallctl("stats.active", &active, &len, NULL, 0);
    mallctl("stats.resident", &resident, &len, NULL, 0);

    printf("\nConfiguration: arenas = %u\n", num_arenas);
    printf("Allocated: %zu bytes\n", allocated);
    printf("Active:    %zu bytes\n", active);
    printf("Resident:  %zu bytes\n", resident);
    printf("Fragmentation (active - allocated): %zu bytes\n", active - allocated);
    printf("Fragmentation percentage: %.2f%%\n", 
           (double)(active - allocated) * 100.0 / active);

    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (blocks[i]) free(blocks[i]);
    }
    
    free(arenas);
}

int main() {
    printf("Memory Fragmentation Test on FreeBSD\n");
    printf("====================================\n");
    printf("Testing different arena configurations...\n");

    unsigned arenas_config[] = {1, 2, 4, 8};
    size_t num_configs = sizeof(arenas_config)/sizeof(arenas_config[0]);
    
    for (int i = 0; i < num_configs; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
           
            printf("\nTest %d/%d starting (PID %d)...\n", i+1, num_configs, getpid());
            run_test(arenas_config[i]);
            printf("Test %d/%d completed (PID %d)\n", i+1, num_configs, getpid());
            exit(0);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                fprintf(stderr, "Test %d failed with status %d\n", i+1, WEXITSTATUS(status));
            }
        } else {
            perror("fork");
            exit(1);
        }
    }
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}

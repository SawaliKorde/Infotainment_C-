//Shared Memory.c

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define SHM_NAME "/random_shm"
#define SHM_SIZE 4096

struct data_entry {
    unsigned long timestamp;
    int value;
};

int main() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, SHM_SIZE);
    struct data_entry *shm_region = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (shm_region == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    printf("Server running. Waiting for data...\n");

    int last_index = 0;
    while (1) {
        sleep(1);
        if (shm_region[last_index].timestamp != 0) {
            printf("Timestamp: %lu | Value: %d\n", shm_region[last_index].timestamp, shm_region[last_index].value);
            last_index = (last_index + 1) % (SHM_SIZE / sizeof(struct data_entry));
        }
    }

    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>

#define SHM_SIZE 4096
#define DEVICE_PATH "/dev/random_dev"

struct data_entry {
    unsigned long timestamp;
    int value;
};

volatile sig_atomic_t stop = 0;

void handle_signal(int sig) {
    stop = 1;
}

int main() {
    int fd;
    struct data_entry *shm_region;

    // Register signal handler for Ctrl+C
    signal(SIGINT, handle_signal);

    // Open the device file
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    // Map the shared memory
    shm_region = (struct data_entry *)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm_region == MAP_FAILED) {
        perror("Failed to mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Successfully mapped shared memory. Reading data...\n");

    int index = 0;
    while (!stop) {
        struct data_entry entry = shm_region[index];
        printf("Timestamp: %lu | Value: %d\n", entry.timestamp, entry.value);
        index = (index + 1) % (SHM_SIZE / sizeof(struct data_entry));
        sleep(1);
    }

    printf("\nExiting reader program...\n");

    // Cleanup
    munmap(shm_region, SHM_SIZE);
    close(fd);

    return EXIT_SUCCESS;
}

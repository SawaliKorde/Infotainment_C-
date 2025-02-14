#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#define PHYSICAL_ADDRESS  0x10000000  // Change this to a valid memory address
#define MAP_SIZE          4096        // Mapping 4KB of memory (1 page)

int main() {
    int fd;
    void *mapped_mem;
    volatile uint32_t *mem_ptr;

    // Open /dev/mem with read/write access
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("Failed to open /dev/mem");
        return 1;
    }

    // Map physical memory to user space
    mapped_mem = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, PHYSICAL_ADDRESS);
    if (mapped_mem == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return 1;
    }

    mem_ptr = (volatile uint32_t *)mapped_mem;

    // Read from memory
    uint32_t read_value = *mem_ptr;
    printf("Value read from address 0x%X: 0x%X\n", PHYSICAL_ADDRESS, read_value);

    // Write to memory (Modify carefully!)
    *mem_ptr = 0xDEADBEEF;  // Example: Writing a test value
    printf("Value written to address 0x%X: 0xDEADBEEF\n", PHYSICAL_ADDRESS);

    // Unmap memory and close file
    if (munmap(mapped_mem, MAP_SIZE) == -1) {
        perror("munmap failed");
    }
    close(fd);

    return 0;
}

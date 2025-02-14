#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

const char* shm_name = "/my_shared_memory";
const int shm_size = 4096;  // Total shared memory size

int main() {
    // Create shared memory object
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        std::cerr << "Shared memory creation failed\n";
        return 1;
    }

    // Set size of the shared memory object
    ftruncate(shm_fd, shm_size);

    // Map shared memory object in address space
    void* ptr = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        std::cerr << "Mapping shared memory failed\n";
        return 1;
    }

    // Initialize the shared memory (first 4 bytes track client count)
    int* client_count = static_cast<int*>(ptr);
    *client_count = 0;  // Set initial count to 0

    std::cout << "Server: Shared memory initialized. Waiting for clients.\n";

    // Keep server running indefinitely
    while (true) {
        sleep(1);
    }

    // Clean up (not reached in this example)
    munmap(ptr, shm_size);
    shm_unlink(shm_name);

    return 0;
}

#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

const char* shm_name = "/my_shared_memory";
const int shm_size = 4096;  // Total shared memory size
const int metadata_size = 8; // 4 bytes for source_id, 4 bytes for target_id

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./client <mode> [target_id] [message]\n";
        std::cerr << "Modes: register, send, listen\n";
        return 1;
    }

    std::string mode = argv[1];  // Mode: register, send, or listen

    // Open shared memory object
    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1) {
        std::cerr << "Shared memory open failed\n";
        return 1;
    }

    // Map shared memory object in address space
    void* ptr = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        std::cerr << "Mapping shared memory failed\n";
        return 1;
    }

    // Get the client count pointer
    int* client_count = static_cast<int*>(ptr);

    if (mode == "register") {
        // Register the client
        int client_id = __sync_fetch_and_add(client_count, 1) + 1;  // Increment client count atomically
        std::cout << "Client registered with ID: " << client_id << "\n";
    } else if (mode == "send") {
        if (argc < 4) {
            std::cerr << "Usage for send: ./client send <target_id> <message>\n";
            return 1;
        }

        int target_id = std::stoi(argv[2]);
        std::string message = argv[3];

        // Find the next free position in the message queue
        char* message_ptr = static_cast<char*>(ptr) + sizeof(int);  // Start after client_count
        while (*message_ptr != '\0') {  // Find the next free slot
            message_ptr += metadata_size + strlen(message_ptr + metadata_size) + 1;
        }

        // Write the message
        int* meta_ptr = reinterpret_cast<int*>(message_ptr);
        meta_ptr[0] = *client_count;  // Source ID (current client ID)
        meta_ptr[1] = target_id;      // Target ID
        strncpy(message_ptr + metadata_size, message.c_str(), message.size() + 1);

        std::cout << "Message sent to Client " << target_id << ": " << message << "\n";
    } else if (mode == "listen") {
        if (argc < 3) {
            std::cerr << "Usage for listen: ./client listen <client_id>\n";
            return 1;
        }

        int client_id = std::stoi(argv[2]);

        // Listen for messages
        char* message_ptr = static_cast<char*>(ptr) + sizeof(int);  // Start after client_count
        while (true) {
            int* meta_ptr = reinterpret_cast<int*>(message_ptr);
            if (meta_ptr[1] == client_id) {  // Check if the message is for this client
                std::cout << "Received message from Client " << meta_ptr[0] << ": "
                          << (message_ptr + metadata_size) << "\n";
                break;
            }
            message_ptr += metadata_size + strlen(message_ptr + metadata_size) + 1;
            if (message_ptr >= static_cast<char*>(ptr) + shm_size) {
                message_ptr = static_cast<char*>(ptr) + sizeof(int);  // Wrap around
                sleep(1);  // Avoid busy waiting
            }
        }
    }

    // Clean up
    munmap(ptr, shm_size);

    return 0;
}

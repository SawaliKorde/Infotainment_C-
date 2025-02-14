//This is server which only "READS" from "/dev/mem and adds the data read from /dev/mem to the SM
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdbool.h>

#define PORT 8080
#define MAX_CLIENTS_PER_THREAD 5
#define MAX_THREADS 10

const char* shm_name = "/my_shared_memory";
const int shm_size = 4096; 
sem_t mem; 
void *ptr1; 
pthread_t client_handler_threads[MAX_THREADS]; 

pthread_mutex_t connection_mutex;

// Structure for sensor data
struct sensorData {
    int bus_id;
    int scale; // range assumed 1-500
    int dev_id;
    float raw_data;
};

struct ThreadData {
    int client_sockets[MAX_CLIENTS_PER_THREAD];
    int count; // Number of clients currently handled by this thread
};

struct ThreadData thread_data[MAX_THREADS]; // Array to hold thread data


double getRandomTemp() { return rand() % 500; }
double getRandomPressure() { return rand() % 100; }
double getRandomVoltage() { return rand() % 250; }

// Thread function to generate data and write to shared memory
void* generateData(void* arg) {
	int mem_fd = open("/dev/mem/", O_RDONLY);
	if(mem_fd < 0){
	    perror("Failed to open /dev/shm");
	    return NULL;
	    }
} 

// Thread function to handle client requests
void* handleClientRequests(void* arg) {
    struct ThreadData* thread_data = (struct ThreadData*)arg;

    for (int i = 0; i < thread_data->count; i++) {
        int client_socket = thread_data->client_sockets[i];
        char buffer[1024];
        int bytes_read;

        while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            printf("Received from client: %s\n", buffer);

            int dev_id, bus_id;
            sscanf(buffer, "dev_id:%d bus_id:%d", &dev_id, &bus_id);

            // Search shared memory for matching data
            sem_wait(&mem);
            struct sensorData* shared_data = (struct sensorData*)ptr1;
            char response[1024];
            snprintf(response, sizeof(response), "No data found for dev_id:%d bus_id:%d\n", dev_id, bus_id);

            for (int j = 0; j < shm_size / sizeof(struct sensorData); ++j) {
                if (shared_data[j].dev_id == dev_id && shared_data[j].bus_id == bus_id) {
                    snprintf(response, sizeof(response), "bus_id:%d scale:%d raw_data:%.2f dev_id:%d\n",
                             shared_data[j].bus_id, shared_data[j].scale, shared_data[j].raw_data, shared_data[j].dev_id);
                    break;
                }
            }
            sem_post(&mem);

            // Send response to client
            send(client_socket, response, strlen(response), 0);
        }

        close(client_socket);
    }

    return NULL;
}

// Thread function to manage client connections
void* manageConnections(void* arg) {
    int server_fd = *(int*)arg;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    int thread_index = 0; // Index for the current thread

    while (true) {
        // Accept a new client connection
        int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("New Socket - Client connected %d\n", new_socket);

        pthread_mutex_lock(&connection_mutex);

        // Check if the current thread has space for more clients
        if (thread_data[thread_index].count < MAX_CLIENTS_PER_THREAD) {
            thread_data[thread_index].client_sockets[thread_data[thread_index].count] = new_socket;
            thread_data[thread_index].count++;

            // Create thread if it's the first client for this thread
            if (thread_data[thread_index].count == 1) {
                if (pthread_create(&client_handler_threads[thread_index], NULL, handleClientRequests, &thread_data[thread_index]) != 0) {
                    perror("Thread creation failed");
                    close(new_socket);
                    thread_data[thread_index].count--;
                } else {
                    printf("New thread created for client connection: %d\n", new_socket);
                }
            }
        } else {
            // Move to the next thread
            thread_index = (thread_index + 1) % MAX_THREADS;

            if (thread_data[thread_index].count == 0) {
                printf("Thread %d is now active.\n", thread_index);
            }
        }

        pthread_mutex_unlock(&connection_mutex);
    }
}


int main() {
    pthread_mutex_init(&connection_mutex, NULL);

    // Initialize thread data
    for (int i = 0; i < MAX_THREADS; i++) {
        thread_data[i].count = 0;
    }

    // Socket connection
    int server_fd;
    struct sockaddr_in address;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket Creation Failed");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind Failed");
        return -1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen Failed");
        return -1;
    }

    // Shared memory creation
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Shared Memory creation failed\n");
        return 1;
    }

    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("Failed to set shared memory size\n");
        return 1;
    }

    ptr1 = mmap(0, shm_size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr1 == MAP_FAILED) {
        perror("Mapping shared memory failed\n");
        return 1;
    }

    // Initialize semaphore
    sem_init(&mem, 0, 1);

    printf("Server Started \n");

    // Create thread to generate data
    pthread_t gen_mem_thread;
    pthread_create(&gen_mem_thread, NULL, generateData, NULL);

    // Create thread to manage client connections
    pthread_t connection_manager_thread;
    pthread_create(&connection_manager_thread, NULL, manageConnections, &server_fd);

    // Wait for threads to finish (this will never happen in this example)
    pthread_join(gen_mem_thread, NULL);
    pthread_join(connection_manager_thread, NULL);

    // Clean up
    pthread_mutex_destroy(&connection_mutex);
    sem_destroy(&mem);
    munmap(ptr1, shm_size);
    shm_unlink(shm_name);
    close(server_fd);

    return 0;
}

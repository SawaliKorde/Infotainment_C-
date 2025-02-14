#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <pthread.h>

#define PORT 8080
#define SHM_KEY 1234
#define SHM_SIZE 1024

// Client data with double-linked list pointers
struct ClientData {
    int id;
    char name[100];
    char email[100];
    struct ClientData* prev;
    struct ClientData* next;
};

struct ClientData* head = NULL;

// Function to create a new client data node
struct ClientData* createClientData(struct ClientData data) {
    struct ClientData* newNode = (struct ClientData*)malloc(sizeof(struct ClientData));
    if (!newNode) {
        printf("Memory allocation error!\n");
        exit(1);
    }
    newNode->id = data.id;
    strcpy(newNode->name, data.name);
    strcpy(newNode->email, data.email);
    newNode->prev = newNode->next = NULL;
    return newNode;
}

// Function to add client data
void add_data_dlink(struct ClientData data) {
    struct ClientData* newNode = createClientData(data);
    
    // If list is empty
    if (head == NULL) {
        head = newNode;
        printf("First client added successfully\n");
        return;
    }
    
    // Add node at the end
    struct ClientData* current = head;
    while (current->next != NULL) {
        // Check for duplicate ID
        if (current->id == data.id) {
            printf("Error: Client with ID %d already exists\n", data.id);
            free(newNode);
            return;
        }
        current = current->next;
    }
    
    // Check last node for duplicate ID
    if (current->id == data.id) {
        printf("Error: Client with ID %d already exists\n", data.id);
        free(newNode);
        return;
    }
    
    // Add new node
    current->next = newNode;
    newNode->prev = current;
    printf("Client added successfully\n");
}

// Function to delete client data
void delete_data(char* name) {
    struct ClientData* current = head;
    
    if (head == NULL) {
        printf("List is empty, nothing to delete\n");
        return;
    }
    
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            if (current == head) {
                head = current->next;
                if (head != NULL) {
                    head->prev = NULL;
                }
            } else if (current->next == NULL) {
                current->prev->next = NULL;
            } else {
                current->prev->next = current->next;
                current->next->prev = current->prev;
            }
            free(current);
            printf("Client with name %s deleted successfully\n", name);
            return;
        }
        current = current->next;
    }
    printf("Client with name %s not found\n", name);
}

// Function to search client data
void search_data(int socket, char* name) {
    struct ClientData* current = head;
    struct ClientData found_data;
    char msg[1024];
    
    if (head == NULL) {
        strcpy(msg, "List is empty");
        send(socket, msg, strlen(msg), 0);
        return;
    }
    
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            found_data = *current;
            send(socket, &found_data, sizeof(struct ClientData), 0);
            printf("\n*----------------------------------------------*\n");
            printf("Found - ID: %d  Name: %s  Email: %s\n", 
                   current->id, current->name, current->email);
            printf("*----------------------------------------------*\n");
            return;
        }
        current = current->next;
    }
    
    strcpy(msg, "Client not found");
    send(socket, msg, strlen(msg), 0);
    printf("\n*----------------------------------------------*\n");
    printf("Client with name %s not found\n", name);
    printf("*----------------------------------------------*\n");
}

// Function to show all clients
void show() {
    struct ClientData* current = head;
    printf("\n*----------------------------------------------*\n");
    printf("Client List:\n");
    
    if (head == NULL) {
        printf("List is empty\n");
        printf("*----------------------------------------------*\n");
        return;
    }
    
    while (current != NULL) {
        printf("ID: %d  Name: %s  Email: %s\n", 
               current->id, current->name, current->email);
        current = current->next;
    }
    printf("*----------------------------------------------*\n");
}

// Function to handle client connections
void* handle_client(void* client_socket) {
    int socket = *(int*)client_socket;
    free(client_socket);
    struct ClientData data;
    char buffer[1024] = {0};
    char msg[1024];
    
    while(1) {
        int valread = read(socket, buffer, sizeof(buffer));
        if (valread <= 0) {
            break;
        }
        buffer[valread] = '\0';
        
        if (strcmp(buffer, "add") == 0) {
            read(socket, &data, sizeof(data));
            add_data_dlink(data);
            strcpy(msg, "Data added successfully.....");
            send(socket, msg, strlen(msg), 0);
            printf("\n*----------------------------------------------*\n");
            printf("ID: %d  Name: %s  Email: %s\n", data.id, data.name, data.email);
            printf("*----------------------------------------------*\n");
        }
        else if (strcmp(buffer, "delete") == 0) {
            read(socket, &data.name, sizeof(data.name));
            delete_data(data.name);
            strcpy(msg, "Data deleted successfully...");
            send(socket, msg, strlen(msg), 0);
            printf("\n*----------------------------------------------*\n");
            printf("Deleted Name: %s\n", data.name);
            printf("*----------------------------------------------*\n");
        }
        else if (strcmp(buffer, "show") == 0) {
            show();
        }
        else if (strcmp(buffer, "search") == 0) {
            read(socket, &data.name, sizeof(data.name));
            search_data(socket, data.name);
        }
        else if (strcmp(buffer, "exit") == 0) {
            close(socket);
            break;
        }
    }
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation error");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return -1;
    }

    // Listen for connections
    if (listen(server_fd, 2) < 0) {
        perror("Listen failed");
        return -1;
    }

    printf("Server is listening on port %d\n", PORT);

    // Create shared memory segment
    int shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        return 1;
    }

    void* ptr = shmat(shm_id, NULL, 0);
    if (ptr == (void*)-1) {
        perror("shmat failed");
        return 1;
    }

    memset(ptr, 0, SHM_SIZE);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }
        printf("Connection Established\n");
        
        pthread_t client_thread;
        int* client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = new_socket;
        
        pthread_create(&client_thread, NULL, handle_client, (void*)client_sock_ptr);
    }

    // Clean up shared memory
    shmdt(ptr);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}
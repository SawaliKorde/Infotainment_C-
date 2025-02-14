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

// Client data
struct ClientData {
    int id;
    char name[100];
    char email[100];
};

typedef struct Node {
    struct ClientData data;
    struct Node* prev;
    struct Node* next;
} DNode;

DNode* head = NULL;

// Function to create a new node
DNode* createDNode(struct ClientData data) {
    DNode* newNode = (DNode*)malloc(sizeof(DNode));
    if (!newNode) {
        printf("Memory allocation error!\n");
        exit(1);
    }
    newNode->data = data;
    newNode->prev = newNode->next = NULL;
    return newNode;
}

// Function to add client data
void add_data_dlink(struct ClientData data) {
    DNode* newNode = createDNode(data);
    
    // If list is empty
    if (head == NULL) {
        head = newNode;
        printf("First client added successfully\n");
        return;
    }
    
    // Add node at the end
    DNode* current = head;
    while (current->next != NULL) {
        // Check for duplicate ID
        if (current->data.id == data.id) {
            printf("Error: Client with ID %d already exists\n", data.id);
            free(newNode);
            return;
        }
        current = current->next;
    }
    
    // Check last node for duplicate ID
    if (current->data.id == data.id) {
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
    DNode* current = head;
    
    if (head == NULL) {
        printf("List is empty, nothing to delete\n");
        return;
    }
    
    while (current != NULL) {
        if (strcmp(current->data.name, name) == 0) {
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
    DNode* current = head;
    struct ClientData found_data;
    char msg[1024];
    
    if (head == NULL) {
        strcpy(msg, "List is empty");
        send(socket, msg, strlen(msg), 0);
        return;
    }
    
    while (current != NULL) {
        if (strcmp(current->data.name, name) == 0) {
            found_data = current->data;
            send(socket, &found_data, sizeof(struct ClientData), 0);
            printf("\n*----------------------------------------------*\n");
            printf("Found - ID: %d  Name: %s  Email: %s\n", 
                   current->data.id, current->data.name, current->data.email);
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
    DNode* current = head;
    printf("\n*----------------------------------------------*\n");
    printf("Client List:\n");
    
    if (head == NULL) {
        printf("List is empty\n");
        printf("*----------------------------------------------*\n");
        return;
    }
    
    while (current != NULL) {
        printf("ID: %d  Name: %s  Email: %s\n", 
               current->data.id, current->data.name, current->data.email);
        current = current->next;
    }
    printf("*----------------------------------------------*\n");
}

// Function to handle client connections
void* handle_client(void* client_socket) {
    int socket = *(int*)client_socket;
    free(client_socket);
    struct ClientData data;
    int choice;
    char msg[1024];
    
    while(1) {
        // Read the choice as an integer
        int valread = read(socket, &choice, sizeof(choice));
        if (valread <= 0) {
            break;
        }
        
        switch(choice) {
            case 1: // Add
                read(socket, &data, sizeof(data));
                add_data_dlink(data);
                strcpy(msg, "Data added successfully.....");
                send(socket, msg, strlen(msg), 0);
                printf("\n*----------------------------------------------*\n");
                printf("ID: %d  Name: %s  Email: %s\n", data.id, data.name, data.email);
                printf("*----------------------------------------------*\n");
                break;
            
            case 2: // Delete
                read(socket, &data.name, sizeof(data.name));
                delete_data(data.name);
                strcpy(msg, "Data deleted successfully...");
                send(socket, msg, strlen(msg), 0);
                printf("\n*----------------------------------------------*\n");
                printf("Deleted Name: %s\n", data.name);
                printf("*----------------------------------------------*\n");
                break;
            
            case 3: // Show
                show();
                break;
            
            case 4: // Search
                read(socket, &data.name, sizeof(data.name));
                search_data(socket, data.name);
                break;
            
            case 5: // Exit
                close(socket);
                return NULL;
            
            default:
                strcpy(msg, "Invalid choice");
                send(socket, msg, strlen(msg), 0);
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

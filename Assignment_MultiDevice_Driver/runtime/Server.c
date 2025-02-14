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
#define CONFIG_FILE "config.txt"
#define MAX_LINE 256
#define MAX_DEVICES 10

#define CONFIG_FILE "/home/rps/Documents/MultiBusDeviceLoader/runtime/config.txt"
#define DRIVER_PATH "/home/rps/Documents/MultiBusDeviceLoader/driver/"
typedef struct Bus {
  int busId;
  string busName;
 struct Device *devices;
}Bus;

typedef struct Device {
    int devId;
    char *devName;         // Pointer to device name
    char *devPhysicalDev;  // Pointer to physical device name
    double value;
    struct Device *next;
} Device;

struct Device devices[MAX_DEVICES];
int device_count = 0;
bool paused = false;
Device *deviceList;

// Function to execute system commands
int execute_command(const char* cmd) {
    printf("Executing: %s\n", cmd);
    return system(cmd);
}

void reload_device(const char* device_name) {
    char command[256];
    
    // First, unload the module if it exists
    snprintf(command, sizeof(command), "sudo rmmod %s 2>/dev/null", device_name);
    execute_command(command);
    
    // Compile the module
    snprintf(command, sizeof(command), "make -C . M=%s.c", device_name);
    execute_command(command);
    
    // Load the new module
    snprintf(command, sizeof(command), "sudo insmod %s.ko", device_name);
    execute_command(command);
}


// Function to parse config file and update devices
void parse_config() {
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (!fp) {
        perror("Error opening config file");
        return;
    }

    char line[MAX_LINE];
    char current_bus[20] = "";
    int new_device_count = 0;
    struct Device new_devices[MAX_DEVICES];

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;  // Remove newline
        
        if (strncmp(line, "BUS ", 4) == 0) {
            strcpy(current_bus, line + 4);
            continue;
        }
        
        if (strlen(line) > 0 && line[0] != '/') {
            char name[50], type[50];
            if (sscanf(line, "%s %s", name, type) == 2) {
                strcpy(new_devices[new_device_count].name, name);
                strcpy(new_devices[new_device_count].type, type);
                strcpy(new_devices[new_device_count].bus, current_bus);
                new_devices[new_device_count].active = true;
                new_device_count++;
            }
        } else if (strlen(line) > 0) {
            strcpy(new_devices[new_device_count-1].path, line);
        }
    }
    fclose(fp);

    // Compare with existing devices and reload if new
    for (int i = 0; i < new_device_count; i++) {
        bool found = false;
        for (int j = 0; j < device_count; j++) {
            if (strcmp(new_devices[i].name, devices[j].name) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            printf("New device found: %s\n", new_devices[i].name);
            reload_device(new_devices[i].type);
        }
    }

    // Update devices array
    memcpy(devices, new_devices, sizeof(new_devices));
    device_count = new_device_count;
}

// Function to handle client commands
void handle_command(const char* command, int client_socket) {
    char response[1024];
    
    if (strcmp(command, "reload") == 0) {
        parse_config();
        strcpy(response, "Configuration reloaded and new devices initialized");
    }
    else if (strcmp(command, "stop") == 0) {
        for (int i = 0; i < device_count; i++) {
            char cmd[256];
            snprintf(cmd, sizeof(cmd), "sudo rmmod %s", devices[i].type);
            execute_command(cmd);
        }
        strcpy(response, "All devices stopped");
    }
    else if (strcmp(command, "pause") == 0) {
        paused = true;
        strcpy(response, "Data reading paused");
    }
    else if (strcmp(command, "resume") == 0) {
        paused = false;
        strcpy(response, "Data reading resumed");
    }
    else {
        strcpy(response, "Unknown command");
    }
    
    send(client_socket, response, strlen(response), 0);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server started on port %d\n", PORT);
    
    // Initial config parse
    parse_config();
    
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }
        
        printf("New client connected\n");
        
        while (1) {
            int valread = read(new_socket, buffer, 1024);
            if (valread <= 0) break;
            
            buffer[valread] = '\0';
            printf("Received command: %s\n", buffer);
            
            handle_command(buffer, new_socket);
            memset(buffer, 0, sizeof(buffer));
        }
        
        close(new_socket);
        printf("Client disconnected\n");
    }
    
    return 0;
}


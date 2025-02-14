#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>

#define PORT 8080
#define MAX_DEVICES 10
#define MAX_LINE 256

#define CONFIG_FILE "/home/rps/Documents/MultiBusDeviceLoader/runtime/config.txt"
#define DRIVER_PATH "/home/rps/Documents/MultiBusDeviceLoader/driver/"

typedef struct Device {
    int devId;
    char *devName;
    char *devPhysicalDev;
    double value;
    struct Device *next;
} Device;

// Global variables
Device *device_list = NULL;
bool paused = false;
static int next_device_id = 1;

// Function to create new device
Device* create_device(const char* name, const char* physical_dev) {
    Device* new_device = (Device*)malloc(sizeof(Device));
    if (!new_device) {
        perror("Failed to allocate memory for device");
        return NULL;
    }
    
    new_device->devId = next_device_id++;
    new_device->devName = strdup(name);
    new_device->devPhysicalDev = strdup(physical_dev);
    new_device->value = 0.0;
    new_device->next = NULL;
    return new_device;
}

// Function to free device memory
void free_device(Device* dev) {
    if (dev) {
        free(dev->devName);
        free(dev->devPhysicalDev);
        free(dev);
    }
}

// Function to execute system commands
int execute_command(const char* cmd) {
    printf("Executing: %s\n", cmd);
    return system(cmd);
}

// Function to reload specific device
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

// Function to clean up existing device list
void cleanup_devices() {
    Device *current = device_list;
    while (current != NULL) {
        Device *next = current->next;
        free_device(current);
        current = next;
    }
    device_list = NULL;
}

// Function to find device in list
Device* find_device(const char* name) {
    Device *current = device_list;
    while (current != NULL) {
        if (strcmp(current->devName, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Function to parse config file and update devices
void parse_config() {
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (!fp) {
        perror("Error opening config file");
        return;
    }

    Device *new_list = NULL;
    Device *last_device = NULL;
    char line[MAX_LINE];
    char current_bus[20] = "";

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;  // Remove newline
        
        if (strncmp(line, "BUS ", 4) == 0) {
            strcpy(current_bus, line + 4);
            continue;
        }
        
        if (strlen(line) > 0 && line[0] != '/') {
            char name[50], type[50];
            if (sscanf(line, "%s %s", name, type) == 2) {
                Device *existing = find_device(name);
                if (!existing) {
                    Device *new_device = create_device(name, "");
                    if (new_list == NULL) {
                        new_list = new_device;
                    } else {
                        last_device->next = new_device;
                    }
                    last_device = new_device;
                    printf("New device found: %s\n", name);
                    reload_device(type);
                }
            }
        } else if (strlen(line) > 0 && last_device != NULL) {
            free(last_device->devPhysicalDev);
            last_device->devPhysicalDev = strdup(line);
        }
    }
    fclose(fp);

    // Update global device list
    cleanup_devices();
    device_list = new_list;
}

// Function to stop all devices
void stop_all_devices() {
    Device *current = device_list;
    while (current != NULL) {
        char command[256];
        snprintf(command, sizeof(command), "sudo rmmod %s", current->devName);
        execute_command(command);
        current = current->next;
    }
}

// Function to handle client commands
void handle_command(const char* command, int client_socket) {
    char response[1024];
    
    if (strcmp(command, "Reload") == 0) {
        parse_config();
        strcpy(response, "Configuration reloaded and new devices initialized");
    }
    else if (strcmp(command, "Stop") == 0) {
        stop_all_devices();
        strcpy(response, "All devices stopped");
    }
    else if (strcmp(command, "Pause") == 0) {
        paused = true;
        strcpy(response, "Data reading paused");
    }
    else if (strcmp(command, "Resume") == 0) {
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
    
    // Cleanup before exit
    cleanup_devices();
    return 0;
}

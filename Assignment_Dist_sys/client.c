#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define DEVICE_SHM_SIZE 4096
#define MAX_ALERT_LENGTH 256
#define DEVICE_TEMP_PATH "/dev/temp_dev"
#define DEVICE_FLOW_PATH "/dev/flowm_dev"
#define DEVICE_PRESSURE_PATH "/dev/pressure_dev"
#define DEVICE_VOLTAGE_PATH "/dev/volt_dev"

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile sig_atomic_t stop = 0;

struct data_entry {
    unsigned long timestamp;
    int value;
};

struct monitor_params {
    const char *device_path;
    const char *device_type;
    int low_threshold;
    int high_threshold;
    const char *low_msg;
    const char *high_msg;
};

void handle_signal(int sig) {
    stop = 1;
}

void get_timestamp(char *buffer, size_t size) {
    time_t now;
    struct tm *tm_info;
    time(&now);
    tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

void write_to_kernel(const char *device_path, const char *alert_message) {
    int fd = open(device_path, O_RDWR);
    if (fd >= 0) {
        write(fd, alert_message, strlen(alert_message));
        close(fd);
    }
}

void print_alert(const char *alert_message) {
    pthread_mutex_lock(&print_mutex);
    printf("\033[1;31m%s\033[0m", alert_message);  // Red color for alerts
    sleep(2);  // Pause for 2 seconds
    pthread_mutex_unlock(&print_mutex);
}

void write_alert_to_file(const char *alert_message) {
    pthread_mutex_lock(&log_mutex);
    FILE *log = fopen("logalerts.txt", "a");
    if (log) {
        char timestamp[26];
        get_timestamp(timestamp, sizeof(timestamp));
        fprintf(log, "[%s] %s", timestamp, alert_message);
        fclose(log);
    }
    pthread_mutex_unlock(&log_mutex);
}

void *monitor_device(void *arg) {
    const struct monitor_params *params = (struct monitor_params *)arg;
    int device_fd = open(params->device_path, O_RDWR);
    if (device_fd < 0) {
        perror("Failed to open device");
        return NULL;
    }
    
    struct data_entry *device_shm = mmap(NULL, DEVICE_SHM_SIZE, PROT_READ | PROT_WRITE, 
                                       MAP_SHARED, device_fd, 0);
    if (device_shm == MAP_FAILED) {
        close(device_fd);
        return NULL;
    }

    printf("Started monitoring %s...\n", params->device_type);
    
    int index = 0;
    while (!stop) {
        struct data_entry entry = device_shm[index];
        const char *status = "NORMAL";
        const char *msg = NULL;

        if (entry.value < params->low_threshold) {
            status = "LOW";
            msg = params->low_msg;
        } else if (entry.value > params->high_threshold) {
            status = "HIGH";
            msg = params->high_msg;
        }

        pthread_mutex_lock(&print_mutex);
        printf("\n%s: %d\n", params->device_type, entry.value);
        printf("Timestamp: %lu\n", entry.timestamp);
        if (strcmp(status, "NORMAL") == 0) {
            printf("Status: \033[1;32m%s\033[0m\n", status);  // Green for normal
        } else {
            printf("Status: \033[1;33m%s\033[0m\n", status);  // Yellow for warnings
        }
        pthread_mutex_unlock(&print_mutex);

        if (msg) {
            char alert_message[MAX_ALERT_LENGTH];
            snprintf(alert_message, sizeof(alert_message), 
                    "%s ALERT - Value: %d - %s\n", 
                    params->device_type, entry.value, msg);
            write_alert_to_file(alert_message);
            write_to_kernel(params->device_path, alert_message);
            print_alert(alert_message);
        }

        index = (index + 1) % (DEVICE_SHM_SIZE / sizeof(struct data_entry));
        sleep(1);
    }

    munmap(device_shm, DEVICE_SHM_SIZE);
    close(device_fd);
    printf("Stopped monitoring %s\n", params->device_type);
    return NULL;
}

int main() {
    pthread_t threads[4];
    const struct monitor_params device_params[] = {
        {
            .device_path = DEVICE_TEMP_PATH,
            .device_type = "Temperature",
            .low_threshold = 30,
            .high_threshold = 60,
            .low_msg = "Ramp up to normal range",
            .high_msg = "Cut down flow by 1/2"
        },
        {
            .device_path = DEVICE_FLOW_PATH,
            .device_type = "Flowmeter",
            .low_threshold = 45,
            .high_threshold = 80,
            .low_msg = "Ramp up to normal range",
            .high_msg = "Cut down flow by 1/2"
        },
        {
            .device_path = DEVICE_PRESSURE_PATH,
            .device_type = "Pressure",
            .low_threshold = 100,
            .high_threshold = 200,
            .low_msg = "Open the outlet valve",
            .high_msg = "Shutdown the flow meter"
        },
        {
            .device_path = DEVICE_VOLTAGE_PATH,
            .device_type = "Voltage",
            .low_threshold = 45,
            .high_threshold = 105,
            .low_msg = "Wait and Shutting down!!",
            .high_msg = "MCB Trip!!"
        }
    };

    signal(SIGINT, handle_signal);
    printf("Starting device monitoring...\n");

    for (int i = 0; i < 4; i++) {
        if (pthread_create(&threads[i], NULL, monitor_device, (void*)&device_params[i]) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&log_mutex);
    pthread_mutex_destroy(&print_mutex);
    printf("Monitoring stopped. Exiting...\n");
    return EXIT_SUCCESS;
}

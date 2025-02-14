#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

#define DEVICE_SHM_SIZE 4096
#define ALERT_SHM_SIZE 4096
#define DEVICE_PATH "/dev/pressure_dev"
#define ALERT_SHM_NAME "/all_alerts"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t logmutex = PTHREAD_MUTEX_INITIALIZER;

struct data_entry {
    unsigned long timestamp;
    int value;
};

volatile sig_atomic_t stop = 0;

void handle_signal(int sig) {
    stop = 1;
}

int main() {
    int device_fd, alert_shm_fd;
    struct data_entry *device_shm;
    char *alert_shm;
    size_t current_pos = 0;
    const char *status = "NORMAL";

    signal(SIGINT, handle_signal);

    device_fd = open(DEVICE_PATH, O_RDWR);
    if (device_fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }
    device_shm = mmap(NULL, DEVICE_SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, device_fd, 0);
    if (device_shm == MAP_FAILED) {
        perror("Failed to mmap device memory");
        close(device_fd);
        return EXIT_FAILURE;
    }

    alert_shm_fd = shm_open(ALERT_SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (alert_shm_fd < 0) {
        perror("Failed to create alert shared memory");
        munmap(device_shm, DEVICE_SHM_SIZE);
        close(device_fd);
        return EXIT_FAILURE;
    }

    ftruncate(alert_shm_fd, ALERT_SHM_SIZE);

    alert_shm = mmap(NULL, ALERT_SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, alert_shm_fd, 0);
    if (alert_shm == MAP_FAILED) {
        perror("Failed to mmap alert memory");
        close(alert_shm_fd);
        munmap(device_shm, DEVICE_SHM_SIZE);
        close(device_fd);
        return EXIT_FAILURE;
    }

    printf("Monitoring pressure...\n");

    int index = 0;
        while (!stop) {
        pthread_mutex_lock(&mutex);
        struct data_entry entry = device_shm[index];

        const char *msg = NULL;
        char alert_message[256]; //buffer to store alrt message
        if (entry.value < 100) {
            status = "LOW";
            msg = "Pressure: Rampup to normal range";
        } else if (entry.value > 200) {
            status = "HIGH";
            msg = "FLOWMETER_ALERT: Shutdown the flowmeter";
        } else {
            status = "NORMAL";
        }
        
        printf("\nPressure: %d pb\n", entry.value);
        printf("Timestamp: %lu\n", entry.timestamp);
        printf("Status: %s\n", status);

        if (msg) {
            //copies the msg into the alert buffr
            snprintf(alert_message, sizeof(alert_message), "%s\n", msg);

            FILE *log = fopen("logpressure.txt", "a");
            if (log) {
                fwrite(alert_message, 1, strlen(alert_message), log);
                fclose(log);
            } else {
                perror("Failed to open log file");
            }
        }

        pthread_mutex_unlock(&mutex);
        index = (index + 1) % (DEVICE_SHM_SIZE / sizeof(struct data_entry));
        sleep(1);
    }


    // Clean up
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&logmutex);

    munmap(alert_shm, ALERT_SHM_SIZE);
    close(alert_shm_fd);
    munmap(device_shm, DEVICE_SHM_SIZE);
    close(device_fd);
    shm_unlink(ALERT_SHM_NAME);

    printf("Exiting...\n");
    return EXIT_SUCCESS;
}


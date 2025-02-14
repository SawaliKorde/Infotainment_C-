#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/simple_device"

int main()
{
    int fd;
    char write_buffer[] = "Hello, device!";
    char read_buffer[1024];

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return EXIT_FAILURE;
    }

    if (write(fd, write_buffer, sizeof(write_buffer)) < 0) {
        perror("Failed to write to the device");
        close(fd);
        return EXIT_FAILURE;
    }

    if (read(fd, read_buffer, sizeof(read_buffer)) < 0) {
        perror("Failed to read from the device");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Read from device: %s\n", read_buffer);

    close(fd);
    return EXIT_SUCCESS;
}

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE "/dev/char_device"

int main() {
    int fd;
    char write_buf[100];
    char read_buf[100];

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    // User input
    printf("Enter text to write to device: ");
    fgets(write_buf, sizeof(write_buf), stdin);
    write_buf[strcspn(write_buf, "\n")] = 0;  // Remove newline

    // Write to device
    write(fd, write_buf, strlen(write_buf));

    // Read from device
    lseek(fd, 0, SEEK_SET);
    read(fd, read_buf, sizeof(read_buf));

    printf("Read from device: %s\n", read_buf);

    close(fd);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/shm_writer_device"

int main()
{
    int fd;
   
    char write_buffer[] = "Hello, this is developer checking!!";
    char read_buffer[1024] = {0};

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return EXIT_FAILURE;
    }
    
    ssize_t bytes_written =  write(fd, write_buffer, sizeof(write_buffer));
    if (bytes_written < 0) {
        perror("Failed to write to the device");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Bytes written: %ld\n", bytes_written);
     ssize_t bytes_read = read(fd, read_buffer, sizeof(read_buffer));
    if (bytes_read < 0) {
        perror("Failed to read from the device");
        close(fd);
        return EXIT_FAILURE;
    }
    read_buffer[bytes_read]='\0';
    
    printf("Read from device read_buffer: %s \n", read_buffer);

    close(fd);
    
    return EXIT_SUCCESS;
}
